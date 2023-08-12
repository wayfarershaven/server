/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "../common/eqemu_logsys.h"
#include "../common/global_define.h"
#include "wguild_mgr.h"
#include "../common/servertalk.h"
#include "clientlist.h"
#include "zonelist.h"
#include "zoneserver.h"


extern ClientList client_list;
extern ZSList zoneserver_list;



WorldGuildManager guild_mgr;



void WorldGuildManager::SendGuildRefresh(uint32 guild_id, bool name, bool motd, bool rank, bool relation) {
	LogGuilds("Broadcasting guild refresh for [{}], changes: name=[{}], motd=[{}], rank=d, relation=[{}]", guild_id, name, motd, rank, relation);
	auto pack = new ServerPacket(ServerOP_RefreshGuild, sizeof(ServerGuildRefresh_Struct));
	ServerGuildRefresh_Struct *s = (ServerGuildRefresh_Struct *) pack->pBuffer;
	s->guild_id = guild_id;
	s->name_change = name;
	s->motd_change = motd;
	s->rank_change = rank;
	s->relation_change = relation;
	zoneserver_list.SendPacket(pack);
	safe_delete(pack);
}

void WorldGuildManager::SendCharRefresh(uint32 old_guild_id, uint32 guild_id, uint32 charid) {
	LogGuilds("Broadcasting char refresh for [{}] from guild [{}] to world", charid, guild_id);
	auto pack = new ServerPacket(ServerOP_GuildCharRefresh, sizeof(ServerGuildCharRefresh_Struct));
	ServerGuildCharRefresh_Struct *s = (ServerGuildCharRefresh_Struct *) pack->pBuffer;
	s->guild_id = guild_id;
	s->old_guild_id = old_guild_id;
	s->char_id = charid;
	zoneserver_list.SendPacket(pack);
	safe_delete(pack);
}

void WorldGuildManager::SendGuildDelete(uint32 guild_id) {
	LogGuilds("Broadcasting guild delete for guild [{}] to world", guild_id);
	auto pack = new ServerPacket(ServerOP_DeleteGuild, sizeof(ServerGuildID_Struct));
	ServerGuildID_Struct *s = (ServerGuildID_Struct *) pack->pBuffer;
	s->guild_id = guild_id;
	zoneserver_list.SendPacket(pack);
	safe_delete(pack);
}

void WorldGuildManager::ProcessZonePacket(ServerPacket *pack) {
	switch(pack->opcode) {

	case ServerOP_RefreshGuild: {
		if(pack->size != sizeof(ServerGuildRefresh_Struct)) {
			LogGuilds("Received ServerOP_RefreshGuild of incorrect size [{}], expected [{}]", pack->size, sizeof(ServerGuildRefresh_Struct));
			return;
		}
		ServerGuildRefresh_Struct *s = (ServerGuildRefresh_Struct *) pack->pBuffer;
		LogGuilds("Received and broadcasting guild refresh for [{}], changes: name=[{}], motd=[{}], rank=d, relation=[{}]", s->guild_id, s->name_change, s->motd_change, s->rank_change, s->relation_change);

		//broadcast this packet to all zones.
		zoneserver_list.SendPacket(pack);

		//preform a local refresh.
		if(!RefreshGuild(s->guild_id)) {
			LogGuilds("Unable to preform local refresh on guild [{}]", s->guild_id);
			//can we do anything?
		}

		break;
	}

	case ServerOP_GuildCharRefresh:
	{
		ServerGuildCharRefresh_Struct *s = (ServerGuildCharRefresh_Struct *) pack->pBuffer;
		LogGuilds("Received and broadcasting guild member refresh for char [{}] to all zones with members of guild [{}]", s->char_id, s->guild_id);

		RefreshGuild(s->guild_id);
		//preform the local update
		client_list.UpdateClientGuild(s->char_id, s->guild_id);

		//broadcast this update to any zone with a member in this guild.
		//because im sick of this not working, sending it to all zones, just spends a bit more bandwidth.
		zoneserver_list.SendPacket(pack);

		break;
	}

	case ServerOP_DeleteGuild: 
	{
		if(pack->size != sizeof(ServerGuildID_Struct)) {
			LogGuilds("Received ServerOP_DeleteGuild of incorrect size [{}], expected [{}]", pack->size, sizeof(ServerGuildID_Struct));
			return;
		}
		ServerGuildID_Struct *s = (ServerGuildID_Struct *) pack->pBuffer;
		LogGuilds("Received and broadcasting guild delete for guild [{}]", s->guild_id);

		//broadcast this packet to all zones.
		zoneserver_list.SendPacket(pack);

		LoadGuilds();

		break;
	}

	case ServerOP_GuildMemberUpdate: {
		if(pack->size != sizeof(ServerGuildMemberUpdate_Struct))
		{
			LogGuilds("Received ServerOP_GuildMemberUpdate of incorrect size [{}], expected [{}]", pack->size, sizeof(ServerGuildMemberUpdate_Struct));
			return;
		}
		ServerGuildID_Struct* s = (ServerGuildID_Struct*)pack->pBuffer;
		RefreshGuild(s->guild_id);

		zoneserver_list.SendPacket(pack);

		break;
	}
	case ServerOP_GuildPermissionUpdate: 
	{
		if (pack->size != sizeof(ServerGuildPermissionUpdate_Struct))
		{
			LogGuilds("Received ServerOP_GuildPermissionUpdate of incorrect size [{}], expected [{}]", pack->size, sizeof(ServerGuildPermissionUpdate_Struct));
			return;
		}

		ServerGuildPermissionUpdate_Struct* sg = (ServerGuildPermissionUpdate_Struct*)pack->pBuffer;

		auto guild = GetGuildByGuildID(sg->GuildID);
		if (!guild) {
			guild_mgr.LoadGuilds();
			guild = GetGuildByGuildID(sg->GuildID);
		}
		if (guild) {
			if (sg->FunctionValue) {
				guild->functions[sg->FunctionID].perm_value |= (1UL << (8 - sg->Rank));
			}
			else {
				guild->functions[sg->FunctionID].perm_value &= ~(1UL << (8 - sg->Rank));
			}

			LogGuilds("World Received ServerOP_GuildPermissionUpdate for guild [{}] function id {} with value of {}",
				sg->GuildID,
				sg->FunctionID,
				sg->FunctionValue
			);

			for (auto const& z : zoneserver_list.getZoneServerList()) {
				auto r = z.get();
				if (r->GetZoneID() > 0) {
					r->SendPacket(pack);
				}
			}
		}
		else {
			LogError("World Received ServerOP_GuildPermissionUpdate for guild [{}] function id {} with value of {} but guild could not be found.",
				sg->GuildID,
				sg->FunctionID,
				sg->FunctionValue
			);
		}

		break;
	}
	case ServerOP_GuildRankNameChange:
	{
		if (pack->size != sizeof(ServerGuildRankNameChange))
		{
			LogGuilds("Received ServerOP_ServerGuildRankNameChange of incorrect size [{}], expected [{}]", pack->size, sizeof(ServerGuildPermissionUpdate_Struct));
			return;
		}

		ServerGuildRankNameChange* rnc = (ServerGuildRankNameChange*)pack->pBuffer;

		auto guild = GetGuildByGuildID(rnc->guild_id);
		if (!guild) {
			guild_mgr.LoadGuilds();
			guild = GetGuildByGuildID(rnc->guild_id);
		}
		if (guild) {
			guild->rank_names[rnc->rank] = rnc->rank_name;
			LogGuilds("World Received ServerOP_GuildRankNameChange from zone for guild [{}] rank id {} with new name of {}",
				rnc->guild_id,
				rnc->rank,
				rnc->rank_name.c_str()
			);
			for (auto const& z : zoneserver_list.getZoneServerList()) {
				auto r = z.get();
				if (r->GetZoneID() > 0) {
					r->SendPacket(pack);
				}
			}
		}
		else {
			LogError("World Received ServerOP_GuildRankNameChange from zone for guild [{}] rank id {} with new name of {} but could not find guild.",
				rnc->guild_id,
				rnc->rank,
				rnc->rank_name.c_str()
			);
		}

		break;
	}
	default:
		LogGuilds("Unknown packet {:#04x} received from zone??", pack->opcode);
		break;
	}
}
