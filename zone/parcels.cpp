/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)
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

#include "../common/global_define.h"
#include "../common/eqemu_logsys.h"
#include "../common/rulesys.h"
#include "../common/strings.h"
#include "../common/misc_functions.h"
#include "../common/events/player_event_logs.h"
#include "../common/repositories/trader_repository.h"

#include "client.h"
#include "entity.h"
#include "mob.h"

#include "quest_parser_collection.h"
#include "string_ids.h"
#include "worldserver.h"

void Client::SendBulkParcels(uint32 merchant_id)
{
	auto item = database.GetItem(5352);
	if (item) {
		auto charges = item->MaxCharges;
		auto inst = database.CreateItem(item, charges);
		if (inst) {
			auto item_price = static_cast<uint32>(item->Price * RuleR(Merchant, SellCostMod) * item->SellRate); 
			auto item_charges = charges ? charges : 1;

			inst->SetCharges(item_charges); 
			inst->SetMerchantCount(charges); 
			inst->SetMerchantSlot(0); 
			inst->SetPrice(item_price);

			SendParcelPacket(inst);
			safe_delete(inst);
		}
	}
}

void Client::SendParcelPacket(const EQ::ItemInstance* inst)
{
	struct Parcel_Struct
	{
		/*000*/	ItemPacketType	PacketType;
		/*004*/	char			SerializedItem[1];
		/*944*/	uint32			sent_time;
		/*948*/ uint32			player_name_length;
		/**/	char			player_name[1];
		/**/	uint32			note_length;
		/**/	char			note[1];
		/*xx*/
	};

	if (!inst) {
		return;
	}

	// Serialize item into |-delimited string (Titanium- uses '|' delimiter .. newer clients use pure data serialization)
	std::string packet = inst->Serialize(125);

	std::string player_name = "SentfromRola";
	std::string note = "This is a very long note for testing purposes only.";

	auto p_size = 16 + 4 + packet.length() + player_name.length() + note.length();
	auto out = new EQApplicationPacket(OP_ItemPacket, p_size);
	auto buffer = new char[p_size];
	char* bufptr = buffer;
	memset(buffer, 0, p_size);

	VARSTRUCT_ENCODE_TYPE(uint32, bufptr, ItemPacketParcel);
	memcpy(bufptr, packet.data(), packet.length());	bufptr += packet.length();
	VARSTRUCT_ENCODE_TYPE(uint32, bufptr, time(nullptr));
	VARSTRUCT_ENCODE_TYPE(uint32, bufptr, player_name.length());
	memcpy(bufptr, player_name.data(), player_name.length()); 
	bufptr += player_name.length();
	VARSTRUCT_ENCODE_TYPE(uint32, bufptr, note.length());
	memcpy(bufptr, note.data(), note.length());	bufptr += note.length();
	VARSTRUCT_ENCODE_TYPE(uint32, bufptr, packet.length()); 

	memcpy(out->pBuffer, buffer, p_size);
	safe_delete_array(buffer);
	FastQueuePacket(&out);
}
