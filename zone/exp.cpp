/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2003 EQEMu Development Team (http://eqemulator.net)

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
#include "../common/features.h"
#include "../common/rulesys.h"
#include "../common/strings.h"

#include "client.h"
#include "data_bucket.h"
#include "groups.h"
#include "mob.h"
#include "raids.h"

#include "queryserv.h"
#include "quest_parser_collection.h"
#include "lua_parser.h"
#include "string_ids.h"

#include "bot.h"
#include "../common/events/player_event_logs.h"
#include "worldserver.h"

extern WorldServer worldserver;

extern QueryServ* QServ;

static uint32 MaxBankedGroupLeadershipPoints(int Level)
{
	if(Level < 35)
		return 4;

	if(Level < 51)
		return 6;

	return 8;
}

static uint32 MaxBankedRaidLeadershipPoints(int Level)
{
	if(Level < 45)
		return 6;

	if(Level < 55)
		return 8;

	return 10;
}

void Client::CalculateLeadershipExp(uint64 &add_exp, uint8 conlevel) {
	if (IsLeadershipEXPOn() && (conlevel == CON_BLUE || conlevel == CON_WHITE || conlevel == CON_YELLOW || conlevel == CON_RED)) {
		add_exp = static_cast<uint64>(static_cast<float>(add_exp) * 0.8f);

		if (GetGroup()) {
			if (m_pp.group_leadership_points < MaxBankedGroupLeadershipPoints(GetLevel())
				&& RuleI(Character, KillsPerGroupLeadershipAA) > 0) {
				uint64 exp = GROUP_EXP_PER_POINT / RuleI(Character, KillsPerGroupLeadershipAA);
				Client *mentoree = GetGroup()->GetMentoree();
				if (GetGroup()->GetMentorPercent() && mentoree &&
					mentoree->GetGroupPoints() < MaxBankedGroupLeadershipPoints(mentoree->GetLevel())) {
					uint64 mentor_exp = exp * (GetGroup()->GetMentorPercent() / 100.0f);
					exp -= mentor_exp;
					mentoree->AddLeadershipEXP(mentor_exp, 0); // ends up rounded down
					mentoree->MessageString(Chat::LeaderShip, GAIN_GROUP_LEADERSHIP_EXP);
				}
				if (exp > 0) {
					// possible if you mentor 100% to the other client
					AddLeadershipEXP(exp, 0); // ends up rounded up if mentored, no idea how live actually does it
					MessageString(Chat::LeaderShip, GAIN_GROUP_LEADERSHIP_EXP);
				}
			} else {
				MessageString(Chat::LeaderShip, MAX_GROUP_LEADERSHIP_POINTS);
			}
		} else {
			Raid *raid = GetRaid();
			// Raid leaders CAN NOT gain group AA XP, other group leaders can though!
			if (raid->IsLeader(this)) {
				if (m_pp.raid_leadership_points < MaxBankedRaidLeadershipPoints(GetLevel())
					&& RuleI(Character, KillsPerRaidLeadershipAA) > 0) {
					AddLeadershipEXP(0, RAID_EXP_PER_POINT / RuleI(Character, KillsPerRaidLeadershipAA));
					MessageString(Chat::LeaderShip, GAIN_RAID_LEADERSHIP_EXP);
				} else {
					MessageString(Chat::LeaderShip, MAX_RAID_LEADERSHIP_POINTS);
				}
			} else {
				if (m_pp.group_leadership_points < MaxBankedGroupLeadershipPoints(GetLevel())
					&& RuleI(Character, KillsPerGroupLeadershipAA) > 0) {
					uint32 group_id = raid->GetGroup(this);
					uint64 exp = GROUP_EXP_PER_POINT / RuleI(Character, KillsPerGroupLeadershipAA);
					Client *mentoree = raid->GetMentoree(group_id);
					if (raid->GetMentorPercent(group_id) && mentoree &&
						mentoree->GetGroupPoints() < MaxBankedGroupLeadershipPoints(mentoree->GetLevel())) {
						uint64 mentor_exp = exp * (raid->GetMentorPercent(group_id) / 100.0f);
						exp -= mentor_exp;
						mentoree->AddLeadershipEXP(mentor_exp, 0);
						mentoree->MessageString(Chat::LeaderShip, GAIN_GROUP_LEADERSHIP_EXP);
					}
					if (exp > 0) {
						AddLeadershipEXP(exp, 0);
						MessageString(Chat::LeaderShip, GAIN_GROUP_LEADERSHIP_EXP);
					}
				} else {
					MessageString(Chat::LeaderShip, MAX_GROUP_LEADERSHIP_POINTS);
				}
			}
		}
	}
}

uint64 Mob::GetBaseEXP() {
	float exp = EXP_FORMULA;
	Log(Logs::General, Logs::Group, "base exp: %4.25f", exp);

	float zemmod = 75.0;
	if (zone->newzone_data.zone_exp_multiplier >= 0) {
		zemmod = zone->newzone_data.zone_exp_multiplier * 100;
	}
	Log(Logs::General, Logs::Group, "zem: %4.25f", zemmod);
	float totalmod = 1.0;
	if (zone->IsHotzone()) {
		totalmod += RuleR(Zone, HotZoneBonus);
	}

	// AK had a permanent 20% XP increase.
	totalmod += 0.20;
	Log(Logs::General, Logs::Group, "totalmod: %4.25f", totalmod);

	uint64 basexp = exp * zemmod * totalmod;
	Log(Logs::General, Logs::Group, "baseexp: %d", basexp);
	uint64 logged_xp = basexp;

	if (player_damage == 0) {
		basexp *= 0.25f;
		Log(Logs::General, Logs::Group, "%s was not damaged by a player. Full XP was: %0.2f Earned XP is: %0.2f", GetName(), logged_xp, basexp);
	} else if (dire_pet_damage > 0) {
		float percentage = static_cast<float>(dire_pet_damage) / total_damage;
		percentage *= 100.0f;
		if (percentage > 50 && percentage <= 74) {
			basexp *= 0.75f;
		} else if (percentage >= 75) {
			basexp *= 0.50f;
		}
		Log(Logs::General, Logs::Group,
			"%s was %0.2f percent damaged by a dire charmed pet (%d/%d). Full XP was: %0.2f Earned XP is: %0.2f",
			GetName(), percentage, dire_pet_damage, total_damage, logged_xp, basexp);
	}
	Log(Logs::General, Logs::Group, "baseexp final: %d", basexp);
	return basexp;
}

void Client::AddEXP(uint64 in_add_exp, uint8 conlevel, bool resexp, uint32 mob_level) {

	if (!IsEXPEnabled()) {
		return;
	}
	
	float mob_level_exp_mod = 0.0;
	if (mob_level && mob_level > 0) {
		int divsor = RuleI(Character, ExpByLevelDivsor);
		if (divsor < 1) {
			divsor = 1; //cant divide by zero
		}
		float multiplyer = RuleR(Character, ExpByLevelMultiplyer);
		mob_level_exp_mod = mob_level * multiplyer / divsor;
	}

	EVENT_ITEM_ScriptStopReturn();

	if (conlevel == CON_GRAY) {
		return;
	}

	uint64 add_exp = in_add_exp;

	if (!resexp && (XPRate != 0)) {
		add_exp = static_cast<uint64>(in_add_exp * (static_cast<float>(XPRate) / 100.0f));
	}

	// Calculate any changes to leadership experience.
	if(!resexp) {
		CalculateLeadershipExp(add_exp, conlevel);
	}
	
	if (m_epp.perAA < 0 || m_epp.perAA > 100) {
		m_epp.perAA = 0;    // stop exploit with sanity check
	}

	float totalexpmod = 1.0;
	float totalaamod = 1.0;
	uint64 add_aaxp = 0;

	totalexpmod += mob_level_exp_mod;
	totalaamod += mob_level_exp_mod;

	// Figure out Con Based Bonus
	if (RuleB(Character, ExpConBasedBonus)) {
		switch (conlevel) {
			case CON_RED:
				totalexpmod  = totalexpmod * RuleR(Character, ExpMultiplierRed);
				totalaamod  = totalaamod * RuleR(Character, AAExpMultiplierRed);
				break;
			case CON_YELLOW:
				totalexpmod  = totalexpmod * RuleR(Character, ExpMultiplierYellow);
				totalaamod  = totalaamod * RuleR(Character, AAExpMultiplierYellow);
				break;
			case CON_WHITE_TITANIUM:
			case CON_WHITE:
				totalexpmod  = totalexpmod * RuleR(Character, ExpMultiplierWhite);
				totalaamod  = totalaamod * RuleR(Character, AAExpMultiplierWhite);
				break;
			case CON_BLUE:
				totalexpmod  = totalexpmod * RuleR(Character, ExpMultiplierDarkBlue);
				totalaamod  = totalaamod * RuleR(Character, AAExpMultiplierDarkBlue);
				break;
			case CON_LIGHTBLUE:
				totalexpmod  = totalexpmod * RuleR(Character, ExpMultiplierLightBlue);
				totalaamod  = totalaamod * RuleR(Character, AAExpMultiplierLightBlue);
				break;
			case CON_GREEN:
				totalexpmod  = totalexpmod * RuleR(Character, ExpMultiplierGreen);
				totalaamod  = totalaamod * RuleR(Character, AAExpMultiplierGreen);
				break;
		}
	}

	if (GetInstanceID() != 0) {
		add_exp = add_exp * RuleR(Character, ExpInstanceMultiplier);
	}

	//figure out how much of this goes to AAs
	add_aaxp = add_exp * m_epp.perAA / 100;

	//take that amount away from regular exp
	add_exp -= add_aaxp;

	// General EXP Modifier (For tuning)
	if (RuleR(Character, ExpMultiplier) >= 0) {
		totalexpmod  = totalexpmod * RuleR(Character, ExpMultiplier);
	}

	// General AA Exp Modifier (For tuning)
	if (RuleR(Character, AAExpMultiplier) >= 0) {
		totalaamod  = totalaamod * RuleR(Character, AAExpMultiplier);
	}

	// Bonus XP Routine
	if (RuleB(Character, BonusExpEnabled)) {
		totalexpmod = totalexpmod * RuleR(Character, BonusExpPercent);
		totalaamod  = totalaamod * RuleR(Character, BonusExpPercent);
	}
	
	add_exp = uint64(float(add_exp) * totalexpmod);
	add_aaxp = uint64(float(add_aaxp) * totalaamod);

	// ZEMS
	if (RuleB(Zone, LevelBasedEXPMods)) {
		if (zone->level_exp_mod[GetLevel()].ExpMod) {
			add_exp *= zone->level_exp_mod[GetLevel()].ExpMod;
			add_aaxp *= zone->level_exp_mod[GetLevel()].AAExpMod;
		}
	}


	// Calculate Caps
	uint64 requiredxp = GetEXPForLevel(GetLevel() + 1) - GetEXPForLevel(GetLevel());
	float xp_cap = (float) requiredxp * 0.13f; //13% of total XP is our cap
	uint64 aaxp_cap = RuleI(Character, MaxAAExpPerKill);

	// Enforce Reg XP Cap
	if (add_exp > xp_cap) {
		add_exp = xp_cap;
	}

	// Enforce AA XP Cap
	if (add_aaxp > aaxp_cap) {
		add_aaxp = aaxp_cap;
	}

	uint64 exp = GetEXP() + add_exp;
	uint64 aaexp = add_aaxp;
	uint64 had_aaexp = GetAAXP();
	aaexp += had_aaexp;
	if (aaexp < had_aaexp) {
		aaexp = had_aaexp;    //watch for wrap
	}

	uint64 neededxp = GetEXPForLevel(GetLevel() + 1) - (GetEXP() + add_exp);
	if (admin >= 100 && GetGM()) {
		Message(Chat::Yellow, fmt::format("[GM] You have gained {} ({}) AXP and {} ({}) EXP. {} more EXP is needed for Level {}",
				add_aaxp, GetAAXP() + add_aaxp, add_exp, GetEXP() + add_exp, neededxp, GetLevel() + 1).c_str());
	}

	//Message(Chat::Yellow, "[DEBUG] XP awarded: %i (%i) Required XP is: %i Cap: %0.2f ", add_exp, GetEXP() + add_exp, requiredxp, xp_cap);
	//Message(Chat::Yellow, "[DEBUG] AA XP awarded: %i (%i) Required AA XP is: %i Cap: %i ", add_aaxp, had_aaexp + add_exp, RuleI(AA, ExpPerPoint), aaxp_cap);


	// Check for Unused AA Cap.  If at or above cap, set AAs to cap, set aaexp to 0 and set aa percentage to 0.
	// Doing this here means potentially one kill wasted worth of experience, but easiest to put it here than to rewrite this function.
	int aa_cap = RuleI(AA, UnusedAAPointCap);

	if (aa_cap >= 0 && aaexp > 0) {
		if (m_pp.aapoints == aa_cap) {
			MessageString(Chat::Red, AA_CAP);
			aaexp = 0;
			m_epp.perAA = 0;
		} else if (m_pp.aapoints > aa_cap) {
			MessageString(Chat::Red, OVER_AA_CAP, fmt::format_int(aa_cap).c_str(), fmt::format_int(aa_cap).c_str());
			m_pp.aapoints = aa_cap;
			aaexp = 0;
			m_epp.perAA = 0;
		}
	}

	// AA Sanity Checking for players who set aa exp and deleveled below allowed aa level.
	if (GetLevel() <= 50 && m_epp.perAA > 0) {
		Message(Chat::Yellow, "You are below the level allowed to gain AA Experience. AA Experience set to 0%");
		aaexp = 0;
		m_epp.perAA = 0;
	}

	SetEXP(exp, aaexp, resexp);
}

void Client::AddEXPPercent(uint8 percent, uint8 level) {

	if (percent < 0) {
		percent = 1;
	}

	if (percent > 100) {
		percent = 100;
	}

	uint64 requiredxp = GetEXPForLevel(level + 1) - GetEXPForLevel(level);
	float tmpxp = requiredxp * (percent / 100.0);
	uint64 newxp = (uint64) tmpxp;
	AddQuestEXP(newxp);
}

void Client::AddQuestEXP(uint64 in_add_exp) {
	// Quest handle method. This divides up AA XP, but does not apply bonuses/modifiers. The quest writer will do that.

	this->EVENT_ITEM_ScriptStopReturn();

	uint64 add_exp = in_add_exp;

	if (m_epp.perAA<0 || m_epp.perAA>100) {
		m_epp.perAA = 0;    // stop exploit with sanity check
	}

	uint64 add_aaxp;

	//figure out how much of this goes to AAs
	add_aaxp = add_exp * m_epp.perAA / 100;
	//take that amount away from regular exp
	add_exp -= add_aaxp;

	uint64 exp = GetEXP() + add_exp;
	uint64 aaexp = add_aaxp;

	uint64 had_aaexp = GetAAXP();
	aaexp += had_aaexp;
	if(aaexp < had_aaexp) {
		aaexp = had_aaexp;    //watch for wrap
	}

	// Check for Unused AA Cap.  If at or above cap, set AAs to cap, set aaexp to 0 and set aa percentage to 0.
	// Doing this here means potentially one kill wasted worth of experience, but easiest to put it here than to rewrite this function.

	if (m_pp.aapoints >= RuleI(Character, UnusedAAPointCap)) {
		if (aaexp > 0) {
			Message(15, "You have reached the Unused AA Point Cap (%d).  Please spend some AA Points before continuing.  Setting AA percentage to 0.", RuleI(Character, UnusedAAPointCap));
			aaexp = 0;
			m_epp.perAA = 0;
		}

		if (m_pp.aapoints > RuleI(Character, UnusedAAPointCap)) {
			Message(15, "You have exceeded the Unused AA Point Cap (%d).  Unused AA Points reduced to %d.", RuleI(Character, UnusedAAPointCap), RuleI(Character, UnusedAAPointCap));
			m_pp.aapoints = RuleI(Character, UnusedAAPointCap);
		}
	}

	SetEXP(exp, aaexp, false);
}

void Client::SetEXP(uint64 set_exp, uint64 set_aaxp, bool isrezzexp) {
	LogDebug("Attempting to Set Exp for [{}] (XP: [{}], AAXP: [{}], Rez: [{}])", this->GetCleanName(), set_exp, set_aaxp, isrezzexp ? "true" : "false");
	//max_AAXP = GetEXPForLevel(52) - GetEXPForLevel(51);	//GetEXPForLevel() doesn't depend on class/race, just level, so it shouldn't change between Clients
	max_AAXP = max_AAXP = GetEXPForLevel(0, true);	//this may be redundant since we're doing this in Client::FinishConnState2()

	if (max_AAXP == 0 || GetEXPForLevel(GetLevel()) == 0xFFFFFFFF) {
		Message(Chat::Red, "Error in Client::SetEXP. EXP not set.");
		return; // Must be invalid class/race
	}

	if ((set_exp + set_aaxp) > (m_pp.exp+m_pp.expAA)) {

		uint64 exp_gained = set_exp - m_pp.exp;
		uint64 aaxp_gained = set_aaxp - m_pp.expAA;
		float exp_percent = (float)((float)exp_gained / (float)(GetEXPForLevel(GetLevel() + 1) - GetEXPForLevel(GetLevel())))*(float)100; //EXP needed for level
		float aaxp_percent = (float)((float)aaxp_gained / (float)(RuleI(AA, ExpPerPoint)))*(float)100; //AAEXP needed for level
		std::string exp_amount_message = "";
		if (RuleI(Character, ShowExpValues) >= 1) {
			if (exp_gained > 0 && aaxp_gained > 0) exp_amount_message = StringFormat("%u, %u AA", exp_gained, aaxp_gained);
			else if (exp_gained > 0) exp_amount_message = StringFormat("%u", exp_gained);
			else exp_amount_message = StringFormat("%u AA", aaxp_gained);
		}

		std::string exp_percent_message = "";
		if (RuleI(Character, ShowExpValues) >= 2) {
			if (exp_gained > 0 && aaxp_gained > 0) exp_percent_message = StringFormat("(%.3f%%, %.3f%%AA)", exp_percent, aaxp_percent);
			else if (exp_gained > 0) exp_percent_message = StringFormat("(%.3f%%)", exp_percent);
			else exp_percent_message = StringFormat("(%.3f%%AA)", aaxp_percent);
		}

		if (isrezzexp) {
			if (RuleI(Character, ShowExpValues) > 0) {
				Message(Chat::Experience, "You regain %s experience from resurrection. %s", exp_amount_message.c_str(), exp_percent_message.c_str());
			} else {
				MessageString(Chat::Experience, REZ_REGAIN);
			}
		} else {
			if(this->IsGrouped()) {
				if (RuleI(Character, ShowExpValues) > 0) {
					Message(Chat::Experience, "You have gained %s party experience! %s", exp_amount_message.c_str(), exp_percent_message.c_str());
				} else if (RuleB(Character, BonusExpEnabled)) {
					MessageString(Chat::Experience, GAIN_GROUPXP_BONUS);
				} else {
					MessageString(Chat::Experience, GAIN_GROUPXP);
				}
			} else if (IsRaidGrouped()) {
				if (RuleI(Character, ShowExpValues) > 0) {
					Message(Chat::Experience, "You have gained %s raid experience! %s", exp_amount_message.c_str(), exp_percent_message.c_str());
				} else if (RuleB(Character, BonusExpEnabled)) {
					MessageString(Chat::Experience, GAIN_RAIDXP_BONUS);
				} else {
					MessageString(Chat::Experience, GAIN_RAIDEXP);
				}
			} else {
				if (RuleI(Character, ShowExpValues) > 0) {
					Message(Chat::Experience, "You have gained %s experience! %s", exp_amount_message.c_str(), exp_percent_message.c_str());
				} else if (RuleB(Character, BonusExpEnabled)) {
					MessageString(Chat::Experience, GAIN_XP_BONUS);
				} else {
					MessageString(Chat::Experience, GAIN_XP);
				}
			}
		}
	} else if((set_exp + set_aaxp) < (m_pp.exp+m_pp.expAA)) { //only loss message if you lose exp, no message if you gained/lost nothing.
		uint64 exp_lost = m_pp.exp - set_exp;
		float exp_percent = (float)((float)exp_lost / (float)(GetEXPForLevel(GetLevel() + 1) - GetEXPForLevel(GetLevel())))*(float)100;

		if (RuleI(Character, ShowExpValues) == 1 && exp_lost > 0) {
			Message(Chat::Yellow, "You have lost %i experience.", exp_lost);
		} else if (RuleI(Character, ShowExpValues) == 2 && exp_lost > 0) {
			Message(Chat::Yellow, "You have lost %i experience. (%.3f%%)", exp_lost, exp_percent);
		} else {
			Message(Chat::Yellow, "You have lost experience.");
		}
	}

	//check_level represents the level we should be when we have
	//this amount of exp (once these loops complete)
	uint16 check_level = GetLevel();
	//see if we gained any levels
	bool level_increase = true;
	int8 level_count = 0;

	if ((signed)(set_exp - m_pp.exp) > 0) { // XP INCREASE
		while (set_exp >= GetEXPForLevel(check_level+1)) {
			check_level++;
			if (check_level > 127) {	//hard level cap
				check_level = 127;
				break;
			}
			level_count++;
		}
	} else { // XP DECREASE
		while (set_exp < GetEXPForLevel(check_level)) {
			check_level--;
			if (check_level < 1) {	// hard level floor
				check_level = 1;
				break;
			}
			level_count--;
		}
	}

	//see if we gained any AAs
	if (set_aaxp >= max_AAXP) {
		/*
			Note: AA exp is stored differently than normal exp.
			Exp points are only stored in m_pp.expAA until you
			gain a full AA point, once you gain it, a point is
			added to m_pp.aapoints and the amount needed to gain
			that point is subtracted from m_pp.expAA
			then, once they spend an AA point, it is subtracted from
			m_pp.aapoints. In theory it then goes into m_pp.aapoints_spent,
			but im not sure if we have that in the right spot.
		*/
		//record how many points we have
		uint32 last_unspentAA = m_pp.aapoints;

		//figure out how many AA points we get from the exp were setting
		m_pp.aapoints = set_aaxp / max_AAXP;
		LogDebug("Calculating additional AA Points from AAXP for [{}]: [{}] / [{}] = [{}] points", this->GetCleanName(), set_aaxp, max_AAXP, (float)set_aaxp / (float)max_AAXP);

		//get remainder exp points, set in PP below
		set_aaxp = set_aaxp - (max_AAXP * m_pp.aapoints);

		//add in how many points we had
		m_pp.aapoints += last_unspentAA;
		//set_aaxp = m_pp.expAA % max_AAXP;

		//figure out how many points were actually gained
		uint32 gained = (m_pp.aapoints - last_unspentAA);

		//Message(Chat::Yellow, "You have gained %d skill points!!", m_pp.aapoints - last_unspentAA);
		char val1[20] = { 0 };
		char val2[20] = { 0 };
		if (gained == 1 && m_pp.aapoints == 1) {
			MessageString(Chat::Experience, GAIN_SINGLE_AA_SINGLE_AA, ConvertArray(m_pp.aapoints, val1)); //You have gained an ability point!  You now have %1 ability point.
		} else if (gained == 1 && m_pp.aapoints > 1) {
			MessageString(Chat::Experience, GAIN_SINGLE_AA_MULTI_AA, ConvertArray(m_pp.aapoints, val1)); //You have gained an ability point!  You now have %1 ability points.
		} else {
			MessageString(Chat::Experience, GAIN_MULTI_AA_MULTI_AA, ConvertArray(gained, val1), ConvertArray(m_pp.aapoints, val2)); //You have gained %1 ability point(s)!  You now have %2 ability point(s).You now have %1 ability point%2.
		}
		
		if (RuleB(AA, SoundForAAEarned)) {
			SendSound();
		}

		RecordPlayerEventLog(PlayerEvent::AA_GAIN, PlayerEvent::AAGainedEvent{gained});

		/* QS: PlayerLogAARate */
		if (RuleB(QueryServ, PlayerLogAARate)) {
			QServ->QSAARate(this->CharacterID(), m_pp.aapoints, last_unspentAA);
		}
	}

	uint8 maxlevel = RuleI(Character, MaxExpLevel) + 1;

	if(maxlevel <= 1) {
		maxlevel = RuleI(Character, MaxLevel) + 1;
	}

	if(check_level > maxlevel) {
		check_level = maxlevel;

		if(RuleB(Character, KeepLevelOverMax)) {
			set_exp = GetEXPForLevel(GetLevel()+1);
		} else {
			set_exp = GetEXPForLevel(maxlevel);
		}
	}

	if(RuleB(Character, PerCharacterQglobalMaxLevel)) {
		uint32 MaxLevel = GetCharMaxLevelFromQGlobal();
		if(MaxLevel) {
			if(GetLevel() >= MaxLevel) {
				uint64 expneeded = GetEXPForLevel(MaxLevel);
				if(set_exp > expneeded) {
					set_exp = expneeded;
				}
			}
		}
	}

	if(RuleB(Character, PerCharacterBucketMaxLevel)) {
		uint32 MaxLevel = GetCharMaxLevelFromBucket();
		if(MaxLevel) {
			if(GetLevel() >= MaxLevel) {
				uint64 expneeded = GetEXPForLevel(MaxLevel);
				if(set_exp > expneeded) {
					set_exp = expneeded;
				}
			}
		}
	}

	if ((GetLevel() != check_level) && !(check_level >= maxlevel)) {
		char val1[20]={0};
		if (level_increase) {
			if (level_count == 1) {
				MessageString(Chat::Experience, GAIN_LEVEL, ConvertArray(check_level, val1));
			} else {
				Message(Chat::Yellow, "Welcome to level %i!", check_level);
			}

			if (check_level == RuleI(Character, DeathItemLossLevel) &&
				m_ClientVersionBit & EQ::versions::maskUFAndEarlier) {
				MessageString(Chat::Yellow, CORPSE_ITEM_LOST);
				}

			if (check_level == RuleI(Character, DeathExpLossLevel)) {
				MessageString(Chat::Yellow, CORPSE_EXP_LOST);
			}
		} else {
			MessageString(Chat::Experience, LOSE_LEVEL, ConvertArray(check_level, val1));
		}

		uint8 myoldlevel = GetLevel();

		SetLevel(check_level);

		if (RuleB(Bots, Enabled) && RuleB(Bots, BotLevelsWithOwner)) {
			// hack way of doing this..but, least invasive... (same criteria as gain level for sendlvlapp)
			Bot::LevelBotWithClient(this, GetLevel(), (myoldlevel == check_level - 1));
		}
	}

	//If were at max level then stop gaining experience if we make it to the cap
	if(GetLevel() == maxlevel - 1){
		uint64 expneeded = GetEXPForLevel(maxlevel);
		if(set_exp > expneeded) {
			set_exp = expneeded;
		}
	}

	//set the client's EXP and AAEXP
	m_pp.exp = set_exp;
	m_pp.expAA = set_aaxp;

	if (GetLevel() < 51) {
		m_epp.perAA = 0;	// turn off aa exp if they drop below 51
	} else
		SendAlternateAdvancementStats();	//otherwise, send them an AA update

	//send the expdata in any case so the xp bar isnt stuck after leveling
	uint64 tmpxp1 = GetEXPForLevel(GetLevel()+1);
	uint64 tmpxp2 = GetEXPForLevel(GetLevel());
	// Quag: crash bug fix... Divide by zero when tmpxp1 and 2 equalled each other, most likely the error case from GetEXPForLevel() (invalid class, etc)
	if (tmpxp1 != tmpxp2 && tmpxp1 != 0xFFFFFFFF && tmpxp2 != 0xFFFFFFFF) {
		auto outapp = new EQApplicationPacket(OP_ExpUpdate, sizeof(ExpUpdate_Struct));
		ExpUpdate_Struct* eu = (ExpUpdate_Struct*)outapp->pBuffer;
		float tmpxp = (float) ( (float) set_exp-tmpxp2 ) / ( (float) tmpxp1-tmpxp2 );
		eu->exp = (uint64)(330.0f * tmpxp);
		FastQueuePacket(&outapp);
	}

	/* 
	if (admin >= AccountStatus::GMAdmin && GetGM()) {
		char val1[20]={0};
		char val2[20]={0};
		char val3[20]={0};
		MessageString(Chat::Experience, GM_GAINXP, ConvertArray(set_aaxp,val1),ConvertArray(set_exp,val2),ConvertArray(GetEXPForLevel(GetLevel()+1),val3));	//[GM] You have gained %1 AXP and %2 EXP (%3).
		//Message(Chat::Yellow, "[GM] You have gained %d AXP and %d EXP (%d)", set_aaxp, set_exp, GetEXPForLevel(GetLevel()+1));
		//Message(Chat::Yellow, "[GM] You have gained %d AXP and %d EXP (%d)", set_aaxp, set_exp, GetEXPForLevel(GetLevel()+1));
		//Message(Chat::Yellow, "[GM] You now have %d / %d EXP and %d / %d AA exp.", set_exp, GetEXPForLevel(GetLevel()+1), set_aaxp, max_AAXP);
	}
	*/
}

void Client::SetLevel(uint8 set_level, bool command)
{
	if (GetEXPForLevel(set_level) == 0xFFFFFFFF) {
		LogError("GetEXPForLevel([{}]) = 0xFFFFFFFF", set_level);
		return;
	}

	auto outapp = new EQApplicationPacket(OP_LevelUpdate, sizeof(LevelUpdate_Struct));
	auto* lu = (LevelUpdate_Struct *) outapp->pBuffer;
	lu->level = set_level;
	if(m_pp.level2 != 0) {
		lu->level_old = m_pp.level2;
	} else {
		lu->level_old = level;
	}

	level = set_level;

	if(IsRaidGrouped()) {
		Raid *r = GetRaid();
		if(r){
			r->UpdateLevel(GetName(), set_level);
		}
	}

	if(set_level > m_pp.level2) {
		if(m_pp.level2 == 0) {
			m_pp.points += 5;
		} else {
			m_pp.points += (5 * (set_level - m_pp.level2));
		}

		m_pp.level2 = set_level;
	}

	if (set_level > m_pp.level) {
		int levels_gained = (set_level - m_pp.level);
		const auto export_string = fmt::format("{}", levels_gained);
		parse->EventPlayer(EVENT_LEVEL_UP, this, export_string, 0);
		if (player_event_logs.IsEventEnabled(PlayerEvent::LEVEL_GAIN)) {
			auto e = PlayerEvent::LevelGainedEvent{
				.from_level = m_pp.level,
				.to_level = set_level,
				.levels_gained = levels_gained
			};

			RecordPlayerEventLog(PlayerEvent::LEVEL_GAIN, e);
		}

		if (RuleB(QueryServ, PlayerLogLevels)) {
			const auto event_desc = fmt::format(
				"Leveled UP :: to Level:{} from Level:{} in zoneid:{} instid:{}",
				set_level,
				m_pp.level,
				GetZoneID(),
				GetInstanceID()
			);
			QServ->PlayerLogEvent(Player_Log_Levels, CharacterID(), event_desc);
		}
	} else if (set_level < m_pp.level) {
		int levels_lost = (m_pp.level - set_level);
		const auto export_string = fmt::format("{}", levels_lost);
		parse->EventPlayer(EVENT_LEVEL_DOWN, this, export_string, 0);
		if (player_event_logs.IsEventEnabled(PlayerEvent::LEVEL_LOSS)) {
			auto e = PlayerEvent::LevelLostEvent{
				.from_level = m_pp.level,
				.to_level = set_level,
				.levels_lost = levels_lost
			};

			RecordPlayerEventLog(PlayerEvent::LEVEL_LOSS, e);
		}

		if (RuleB(QueryServ, PlayerLogLevels)) {
			const auto event_desc = fmt::format(
				"Leveled DOWN :: to Level:{} from Level:{} in zoneid:{} instid:{}",
				set_level,
				m_pp.level,
				GetZoneID(),
				GetInstanceID()
			);
			QServ->PlayerLogEvent(Player_Log_Levels, CharacterID(), event_desc);
		}
	}

	m_pp.level = set_level;
	if (command) {
		m_pp.exp = GetEXPForLevel(set_level);
		Message(Chat::Yellow, fmt::format("Welcome to level {}!", set_level).c_str());
		lu->exp = 0;
	} else {
		const auto temporary_xp = (
			static_cast<float>(m_pp.exp - GetEXPForLevel(GetLevel())) /
			static_cast<float>(GetEXPForLevel(GetLevel() + 1) - GetEXPForLevel(GetLevel()))
		);
		lu->exp = static_cast<uint64>(330.0f * temporary_xp);	
	}
	QueuePacket(outapp);
	safe_delete(outapp);
	SendAppearancePacket(AT_WhoLevel, set_level); // who level change
	entity_list.UpdateConLevels(this); // update npc con levels for client

	LogInfo("Setting Level for [{}] to [{}]", GetName(), set_level);

	CalcBonuses();

	if (!RuleB(Character, HealOnLevel)) {
		const auto max_hp = CalcMaxHP();
		if (GetHP() > max_hp) {
			SetHP(max_hp);
		}
	} else {
		SetHP(CalcMaxHP()); // Why not, lets give them a free heal
	}

	if(!RuleB(Character, ManaOnLevel)) {
		int mp = CalcMaxMana();
		if(GetMana() > mp) {
			SetMana(mp);
		}
	} else {
		SetMana(CalcMaxMana()); // Why not, lets give them a free heal
	}
	DoTributeUpdate();
	SendHPUpdate();
	UpdateWho();
	SendManaUpdate();
	UpdateMercLevel();
	Save();
}

// Note: The client calculates exp separately, we cant change this function
// Add: You can set the values you want now, client will be always sync :) - Merkur
uint64 Client::GetEXPForLevel(uint16 check_level, bool aa)
{
// Warning: Changing anything in this method WILL cause levels to change in-game the first time a player
	// gains or loses XP.

	if(aa) {
		if(m_epp.perAA > 99) {
			return (RuleI(AA, ExpPerPoint));
		} else {
			check_level = 52;
		}
	}

	check_level -= 1;
	float base = (check_level)*(check_level)*(check_level);

	// Classes: In the XP formula AK used, they WERE calculated in. This was due to Sony not being able to change their XP
	// formula drastically (see above comment.) Instead, they gave the penalized classes a bonus on gain. We've decided to go
	// the easy route, and simply not use a class mod at all.

	float playermod = 10;
	uint8 race = GetBaseRace();
	if(race == HALFLING) {
		playermod *= 95.0;
	} else if(race == DARK_ELF || race == DWARF || race == ERUDITE || race == GNOME ||
			race == HALF_ELF || race == HIGH_ELF || race == HUMAN || race == WOOD_ELF ||
			race == VAHSHIR || race == FROGLOK || race == FROGLOK2 || race == DRAKKIN) {
		playermod *= 100.0;
	} else if(race == BARBARIAN) {
		playermod *= 105.0;
	} else if(race == OGRE) {
		playermod *= 115.0;
	} else if(race == IKSAR || race == TROLL) {
		playermod *= 120.0;
	}

	float mod;
	if (check_level <= 29) {
		mod = 1.0;
	} else if (check_level <= 34) {
		mod = 1.1;
	} else if (check_level <= 39) {
		mod = 1.2;
	} else if (check_level <= 44) {
		mod = 1.3;
	} else if (check_level <= 50) {
		mod = 1.4;
	} else if (check_level == 51) {
		mod = 1.5;
	} else if (check_level == 52) {
		mod = 1.6;
	} else if (check_level == 53) {
		mod = 1.7;
	} else if (check_level == 54) {
		mod = 1.9;
	} else if (check_level == 55) {
		mod = 2.1;
	} else if (check_level == 56) {
		mod = 2.3;
	} else if (check_level == 57) {
		mod = 2.5;
	} else if (check_level == 58) {
		mod = 2.7;
	} else if (check_level == 59) {
		mod = 3.0;
	} else if (check_level == 60) {
		mod = 3.1;
	} else if (check_level == 61) {
		mod = 3.2;
	} else if (check_level == 62) {
		mod = 3.3;
	} else if (check_level == 63) {
		mod = 3.4;
	} else if (check_level == 64) {
		mod = 3.5;
	} else if (check_level == 65) {
		mod = 3.6;
	} else if (check_level == 66) {
        mod = RuleR(Character, ExpLevel66Mod);
	} else if (check_level == 67) {
        mod = RuleR(Character, ExpLevel67Mod);
	} else if (check_level == 68) {
        mod = RuleR(Character, ExpLevel68Mod);
	} else if (check_level == 69) {
        mod = RuleR(Character, ExpLevel69Mod);
    } else if (check_level == 70) {
        mod = RuleR(Character, ExpLevel70Mod);
	} else {
        mod = 6.2;
	}

	uint64 finalxp = uint64(base * playermod * mod);
	if(aa) {
		uint64 aaxp;
		aaxp = finalxp - GetEXPForLevel(51);
		return aaxp;
	}
	finalxp = mod_client_xp_for_level(finalxp, check_level);
	return finalxp;
}

void Client::AddLevelBasedExp(uint8 exp_percentage, uint8 max_level)
{
	uint64	award;
	uint64	xp_for_level;

	if (exp_percentage > 100) {
		exp_percentage = 100;
	}

	if (!max_level || GetLevel() < max_level) {
		max_level = GetLevel();
	}

	xp_for_level = GetEXPForLevel(max_level + 1) - GetEXPForLevel(max_level);
	award = xp_for_level * exp_percentage / 100;

	if (RuleB(Zone, LevelBasedEXPMods)) {
		if (zone->level_exp_mod[GetLevel()].ExpMod) {
			award *= zone->level_exp_mod[GetLevel()].ExpMod;
		}
	}

	uint64 newexp = GetEXP() + award;
	SetEXP(newexp, GetAAXP());
}

void Group::SplitExp(uint64 exp, Mob* other) {
	if (other->CastToNPC()->MerchantType != 0) { // Ensure NPC isn't a merchant
		return;
	}

	if (other->GetOwner() && other->GetOwner()->IsClient()) { // Ensure owner isn't pc
		return;
	}

	unsigned int i;
	int8 membercount = 0;
	int8 close_membercount = 0;
	uint8 maxlevel = 1;
	uint16 weighted_levels = 0;
	uint8 minlevel = 70;

	// This loop grabs the max player level for the group level check further down, the min level to subtract the 6th group member, adds up all
	// the player levels for the weighted split, and does a preliminary count of the group members for the group bonus calc.
	for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (members[i] != nullptr && members[i]->IsClient()) {
			Client *cmember = members[i]->CastToClient();
			if (cmember->GetZoneID() == zone->GetZoneID()) {
				if (cmember->GetLevel() > maxlevel) {
					maxlevel = cmember->GetLevel();
				}

				if (cmember->GetLevel() < minlevel) {
					minlevel = cmember->GetLevel();
				}

				if (cmember->GetLevelCon(other->GetLevel()) != CON_GRAY) {
					++membercount;
					if (cmember->IsInRange(other)) {
						weighted_levels += cmember->GetLevel();
						++close_membercount;
					}
				}
			}
		}
	}

	// If the NPC is green to the whole group or they are all out of the kill zone (wipe?) this will return.
	if (membercount <= 0 || close_membercount <= 0) {
		return;
	}

	// We don't need this variable set if we don't have a 6th member to subtract from the split.
	if (close_membercount < 6) {
		minlevel = 0;
	}

	bool isgray = false;

	int conlevel = Mob::GetLevelCon(maxlevel, other->GetLevel());
	if (conlevel == CON_GRAY) {
		isgray = true;
	}

	//Give XP out to lower toons from NPCs that are green to the highest player.
	//No Exp for gray cons.
	if (isgray) {
		return;
	}

	// This loop uses the max player level we now have to determine who is in level range to gain XP. Anybody
	// outside the range is subtracted from the player count and has their level removed from the weighted split.
	for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (members[i] != nullptr && members[i]->IsClient()) { // If Group Member is Client
			Client *cmember = members[i]->CastToClient();
			if (!cmember->IsInLevelRange(maxlevel) &&
				cmember->CastToClient()->GetZoneID() == zone->GetZoneID() &&
				cmember->GetLevelCon(other->GetLevel()) != CON_GRAY) {
				if (membercount != 0 && close_membercount != 0) {
					Log(Logs::Detail, Logs::Group, "%s is not within level range, removing from XP gain.",
						cmember->GetName());
					--membercount;

					if (cmember->CastToClient()->IsInRange(other)) {
						if (weighted_levels >= cmember->GetLevel()) {
							weighted_levels -= cmember->GetLevel();
						}

						--close_membercount;
						minlevel = 0;
					}
				} else {
					return;
				}
			}
		}
	}

	// Another sanity check.
	if (membercount <= 0 || close_membercount <= 0) {
		return;
	}

	float groupmod = 1.0;

	switch(membercount) {
		case 2:
			groupmod += 0.20;
			break;
		case 3:
			groupmod += 0.40;
			break;
		case 4:
			groupmod += 1.20;
			break;
		case 5:
		case 6:
			groupmod += 1.60;
			break;
	}

	Log(Logs::Detail, Logs::Group, "Group mod is %d", groupmod);

	float groupexp = (static_cast<float>(exp) * groupmod) * RuleR(Character, GroupExpMultiplier);

	// This loop figures out the split, and sends XP for each player that qualifies. (NPC is not green to the player, player is in the
	// zone where the kill occurred, is in range of the corpse, and is in level range with the rest of the group.)
	for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (members[i] != nullptr && members[i]->IsClient()) { // If Group Member is Client
			Client *cmember = members[i]->CastToClient();

			if (cmember->CastToClient()->GetZoneID() == zone->GetZoneID() &&
				cmember->GetLevelCon(other->GetLevel()) != CON_GRAY &&
				cmember->IsInRange(other)) {
				if (cmember->IsInLevelRange(maxlevel)) {
					float split_percent = static_cast<float>(cmember->GetLevel()) / weighted_levels;
					float splitgroupxp = groupexp * split_percent;
					if (splitgroupxp < 1) {
						splitgroupxp = 1;
					}

					if (conlevel == CON_GRAY) {
						conlevel = Mob::GetLevelCon(cmember->GetLevel(), other->GetLevel());
					}

					cmember->AddEXP(static_cast<uint64>(splitgroupxp), conlevel, false, other->GetLevel());
					Log(Logs::Detail, Logs::Group, "%s splits %0.2f with the rest of the group. Their share: %0.2f",
						cmember->GetName(), groupexp, splitgroupxp);
					//if(cmember->Admin() > 80)
					//	cmember->Message(Chat::Yellow, "Group XP awarded is: %0.2f Total XP is: %0.2f for count: %i total count: %i in_exp is: %i", splitgroupxp, groupexp, close_membercount, membercount, exp);

				} else {
					Log(Logs::Detail, Logs::Group, "%s is too low in level to gain XP from this group.",
						cmember->GetName());
				}
			} else {
				Log(Logs::Detail, Logs::Group,
					"%s is not in the kill zone, is out of range, or %s is green to them. They won't recieve group XP.",
					cmember->GetName(), other->GetCleanName());
			}
		}
	}
}

void Raid::SplitExp(uint64 exp, Mob* other) {
	if( other->CastToNPC()->MerchantType != 0 ) { // Ensure NPC isn't a merchant
		return;
	}

	if(other->GetOwner() && other->GetOwner()->IsClient()) { // Ensure owner isn't pc
		return;
	}

	uint64 groupexp = exp;
	uint8 membercount = 0;
	uint8 maxlevel = 1;

	for (int i = 0; i < MAX_RAID_MEMBERS; i++) {
		if (members[i].member != nullptr) {
			if(members[i].member->GetLevel() > maxlevel) {
				maxlevel = members[i].member->GetLevel();
			}
			membercount++;
		}
	}

	groupexp = (uint64)((float)groupexp * (1.0f-(RuleR(Character, RaidExpMultiplier))));

	int conlevel = Mob::GetLevelCon(maxlevel, other->GetLevel());
	if(conlevel == CON_GRAY) {
		return;	//no exp for grays...
	}

	if (membercount == 0) {
		return;
	}

	for (unsigned int x = 0; x < MAX_RAID_MEMBERS; x++) {
		if (members[x].member != nullptr) { // If Group Member is Client
			Client *cmember = members[x].member;
			// add exp + exp cap
			int16 diff = cmember->GetLevel() - maxlevel;
			int16 maxdiff = -(cmember->GetLevel()*15/10 - cmember->GetLevel());
			if(maxdiff > -5) {
				maxdiff = -5;
			}

			if (diff >= (maxdiff)) { /*Instead of person who killed the mob, the person who has the highest level in the group*/
				uint64 tmp = (cmember->GetLevel()+3) * (cmember->GetLevel()+3) * 75 * 35 / 10;
				uint64 tmp2 = (groupexp / membercount) + 1;
				cmember->AddEXP( tmp < tmp2 ? tmp : tmp2, conlevel, false, other->GetLevel());
			}
		}
	}
}

void Client::SetLeadershipEXP(uint64 group_exp, uint64 raid_exp) {
	while(group_exp >= GROUP_EXP_PER_POINT) {
		group_exp -= GROUP_EXP_PER_POINT;
		m_pp.group_leadership_points++;
		MessageString(Chat::LeaderShip, GAIN_GROUP_LEADERSHIP_POINT);
	}
	while(raid_exp >= RAID_EXP_PER_POINT) {
		raid_exp -= RAID_EXP_PER_POINT;
		m_pp.raid_leadership_points++;
		MessageString(Chat::LeaderShip, GAIN_RAID_LEADERSHIP_POINT);
	}

	m_pp.group_leadership_exp = group_exp;
	m_pp.raid_leadership_exp = raid_exp;

	SendLeadershipEXPUpdate();
}

void Client::AddLeadershipEXP(uint64 group_exp, uint64 raid_exp) {
	SetLeadershipEXP(GetGroupEXP() + group_exp, GetRaidEXP() + raid_exp);
}

void Client::SendLeadershipEXPUpdate() {
	auto outapp = new EQApplicationPacket(OP_LeadershipExpUpdate, sizeof(LeadershipExpUpdate_Struct));
	LeadershipExpUpdate_Struct* eu = (LeadershipExpUpdate_Struct *) outapp->pBuffer;

	eu->group_leadership_exp = m_pp.group_leadership_exp;
	eu->group_leadership_points = m_pp.group_leadership_points;
	eu->raid_leadership_exp = m_pp.raid_leadership_exp;
	eu->raid_leadership_points = m_pp.raid_leadership_points;

	FastQueuePacket(&outapp);
}

uint8 Client::GetCharMaxLevelFromQGlobal() {
	auto char_cache = GetQGlobals();

	std::list<QGlobal> global_map;

	if (char_cache) {
		QGlobalCache::Combine(global_map, char_cache->GetBucket(), 0, CharacterID(), zone->GetZoneID());
	}

	for (const auto& global : global_map) {
		if (global.name == "CharMaxLevel") {
			if (Strings::IsNumber(global.value)) {
				return static_cast<uint8>(std::stoul(global.value));
			}
		}
	}

	return 0;
}

uint8 Client::GetCharMaxLevelFromBucket()
{
	auto new_bucket_name = fmt::format(
		"{}-CharMaxLevel",
		GetBucketKey()
	);

	auto bucket_value = DataBucket::GetData(new_bucket_name);
	if (!bucket_value.empty()) {
		if (Strings::IsNumber(bucket_value)) {
			return static_cast<uint8>(std::stoul(bucket_value));
		}
	}

	auto old_bucket_name = fmt::format(
		"{}-CharMaxLevel",
		CharacterID()
	);

	bucket_value = DataBucket::GetData(old_bucket_name);
	if (!bucket_value.empty()) {
		if (Strings::IsNumber(bucket_value)) {
			return static_cast<uint8>(std::stoul(bucket_value));
		}
	}

	return 0;
}

uint64 Client::GetRequiredAAExperience() {
#ifdef LUA_EQEMU
	uint64 lua_ret = 0;
	bool ignoreDefault = false;
	lua_ret = LuaParser::Instance()->GetRequiredAAExperience(this, ignoreDefault);

	if (ignoreDefault) {
		return lua_ret;
	}
#endif

	return RuleI(AA, ExpPerPoint);
}

bool Client::IsInRange(Mob* defender)
{
	float exprange = RuleR(Zone, GroupEXPRange);

	float t1, t2, t3;
	t1 = defender->GetX() - GetX();
	t2 = defender->GetY() - GetY();
	//t3 = defender->GetZ() - GetZ();
	if(t1 < 0) {
		abs(t1);
	}

	if(t2 < 0) {
		abs(t2);
	}

	//if(t3 < 0)
	//	abs(t3);
	if(( t1 > exprange) || ( t2 > exprange)) { //	|| ( t3 > 40) ) {
		//_log(CLIENT__EXP, "%s is out of range. distances (%.3f,%.3f,%.3f), range %.3f No XP will be awarded.", defender->GetName(), t1, t2, t3, exprange);
		return false;
	} else {
		return true;
	}
}

bool Client::IsInLevelRange(uint8 maxlevel)
{
	uint8 max_level = GetLevel()*1.5 + 0.5;
	if(max_level < 6) {
		max_level = 6;
	}

	if (max_level >= maxlevel) {
		return true;
	} else {
		return false;
	}
}

void Client::GetExpLoss(Mob* killerMob, uint16 spell, uint64 &exploss)
{
	float loss;
	uint8 level = GetLevel();
	if(level >= 1 && level <= 29) {
		loss = 0.16f;
	} else if(level == 30) {
		loss = 0.08f;
	} else if(level >= 31 && level <= 34) {
		loss = 0.15f;
	} else if(level == 35) {
		loss = 0.075f;
	} else if(level >= 36 && level <= 39) {
		loss = 0.14f;
	} else if(level == 40) {
		loss = 0.07f;
	} else if(level >= 41 && level <= 44) {
		loss = 0.13f;
	} else if(level == 45) {
		loss = 0.065f;
	} else if(level >= 46 && level <= 50) {
		loss = 0.12f;
	} else if(level >= 51) {
		loss = 0.06f;
	}

	if(RuleB(Character, SmoothEXPLoss)) {
		if(loss >= 0.12) {
			loss /= 2;
		}
	}

	uint64 requiredxp = GetEXPForLevel(level + 1) - GetEXPForLevel(level);
	exploss = (uint64)((float)requiredxp * (loss * RuleR(Character, EXPLossMultiplier)));

	if((level < RuleI(Character, DeathExpLossLevel)) || (level > RuleI(Character, DeathExpLossMaxLevel)) || IsBecomeNPC()) {
		exploss = 0;
	} else if(killerMob) {
		if(killerMob->IsClient()) {
			exploss = 0;
		} else if(killerMob->GetOwner() && killerMob->GetOwner()->IsClient()) {
			exploss = 0;
		}
	}

	if(spell != SPELL_UNKNOWN) {
		uint32 buff_count = GetMaxTotalSlots();
		for(uint16 buffIt = 0; buffIt < buff_count; buffIt++) {
			if(buffs[buffIt].spellid == spell && buffs[buffIt].client) {
				exploss = 0;	// no exp loss for pvp dot
				break;
			}
		}
	}
}