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
#include "../common/data_verification.h"

#include "bot.h"
#include "../common/events/player_event_logs.h"
#include "worldserver.h"

extern WorldServer worldserver;

extern QueryServ* QServ;

static uint64 ScaleAAXPBasedOnCurrentAATotal(int earnedAA, uint64 add_aaxp)
{
	float baseModifier = RuleR(AA, ModernAAScalingStartPercent);
	int aaMinimum = RuleI(AA, ModernAAScalingAAMinimum);
	int aaLimit = RuleI(AA, ModernAAScalingAALimit);

	// Are we within the scaling window?
	if (earnedAA >= aaLimit || earnedAA < aaMinimum)
	{
		LogDebug("Not within AA scaling window");

		// At or past the limit.  We're done.
		return add_aaxp;
	}

	// We're not at the limit yet.  How close are we?
	int remainingAA = aaLimit - earnedAA;

	// We might not always be X - 0
	int scaleRange = aaLimit - aaMinimum;

	// Normalize and get the effectiveness based on the range and the character's
	// current spent AA.
	float normalizedScale = (float)remainingAA / scaleRange;

	// Scale.
	uint64 totalWithExpMod = add_aaxp * (baseModifier / 100) * normalizedScale;

	// Are we so close to the scale limit that we're earning more XP without scaling?  This
	// will happen when we get very close to the limit.  In this case, just grant the unscaled
	// amount.
	if (totalWithExpMod < add_aaxp)
	{
		return add_aaxp;
	}

	Log(Logs::Detail,
		Logs::None,
		"Total before the modifier %d :: NewTotal %d :: ScaleRange: %d, SpentAA: %d, RemainingAA: %d, normalizedScale: %0.3f",
		add_aaxp, totalWithExpMod, scaleRange, earnedAA, remainingAA, normalizedScale);

	return totalWithExpMod;
}

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

uint64 Client::CalcEXP(uint8 consider_level, bool ignore_modifiers) {
	uint64 in_add_exp = EXP_FORMULA;

	if (XPRate != 0) {
		in_add_exp = static_cast<uint64>(in_add_exp * (static_cast<float>(XPRate) / 100.0f));
	}

	if (!ignore_modifiers) {
		auto total_modifier = 1.0f;
		auto zone_modifier  = 1.0f;

		if (RuleR(Character, ExpMultiplier) >= 0) {
			total_modifier *= RuleR(Character, ExpMultiplier);
		}

		if (zone->newzone_data.zone_exp_multiplier >= 0) {
			zone_modifier *= zone->newzone_data.zone_exp_multiplier;
		}

		if (RuleB(Character, UseRaceClassExpBonuses)) {
			if (
				GetClass() == Class::Warrior ||
				GetClass() == Class::Rogue ||
				GetBaseRace() == HALFLING
			) {
				total_modifier *= 1.05;
			}
		}

		if (zone->IsHotzone()) {
			total_modifier += RuleR(Zone, HotZoneBonus);
		}

		in_add_exp = uint64(float(in_add_exp) * total_modifier * zone_modifier);
	}

	if (RuleB(Character,UseXPConScaling)) {
		if (consider_level != 0xFF) {
			switch (consider_level) {
				case ConsiderColor::Gray:
					in_add_exp = 0;
					return 0;
				case ConsiderColor::Green:
					in_add_exp = in_add_exp * RuleI(Character, GreenModifier) / 100;
					break;
				case ConsiderColor::LightBlue:
					in_add_exp = in_add_exp * RuleI(Character, LightBlueModifier) / 100;
					break;
				case ConsiderColor::DarkBlue:
					in_add_exp = in_add_exp * RuleI(Character, BlueModifier) / 100;
					break;
				case ConsiderColor::White:
					in_add_exp = in_add_exp * RuleI(Character, WhiteModifier) / 100;
					break;
				case ConsiderColor::Yellow:
					in_add_exp = in_add_exp * RuleI(Character, YellowModifier) / 100;
					break;
				case ConsiderColor::Red:
					in_add_exp = in_add_exp * RuleI(Character, RedModifier) / 100;
					break;
			}
		}
	}

	if (!ignore_modifiers) {
		if (RuleB(Zone, LevelBasedEXPMods)) {
			if (zone->level_exp_mod[GetLevel()].ExpMod) {
				in_add_exp *= zone->level_exp_mod[GetLevel()].ExpMod;
			}
		}

		if (RuleR(Character, FinalExpMultiplier) >= 0) {
			in_add_exp *= RuleR(Character, FinalExpMultiplier);
		}

		if (RuleB(Character, EnableCharacterEXPMods)) {
			in_add_exp *= zone->GetEXPModifier(this);
		}
	}

	return in_add_exp;
}

uint64 Client::GetExperienceForKill(Mob *against)
{
#ifdef LUA_EQEMU
	uint64 lua_ret = 0;
	bool ignoreDefault = false;
	lua_ret = LuaParser::Instance()->GetExperienceForKill(this, against, ignoreDefault);

	if (ignoreDefault) {
		return lua_ret;
	}
#endif

	if (against && against->IsNPC()) {
		uint32 level = (uint32)against->GetLevel();
		uint64 ret = EXP_FORMULA;

		auto mod = against->GetKillExpMod();
		if(mod >= 0) {
			ret *= mod;
			ret /= 100;
		}

		return ret;
	}

	return 0;
}

float static GetConLevelModifierPercent(uint8 conlevel)
{
	switch (conlevel)
	{
	case ConsiderColor::Green:
		return (float)RuleI(Character, GreenModifier) / 100;
		break;
	case ConsiderColor::LightBlue:
		return (float)RuleI(Character, LightBlueModifier) / 100;
		break;
	case ConsiderColor::DarkBlue:
		return (float)RuleI(Character, BlueModifier) / 100;
		break;
	case ConsiderColor::White:
		return (float)RuleI(Character, WhiteModifier) / 100;
		break;
	case ConsiderColor::Yellow:
		return (float)RuleI(Character, YellowModifier) / 100;
		break;
	case ConsiderColor::Red:
		return (float)RuleI(Character, RedModifier) / 100;
		break;
	default:
		return 0;
	}
}

void Client::CalculateNormalizedAAExp(uint64 &add_aaxp, uint8 conlevel, bool resexp)
{
	// Functionally this is the same as having the case in the switch, but this is
	// cleaner to read.
	if (ConsiderColor::Gray == conlevel || resexp)
	{
		add_aaxp = 0;
		return;
	}

	// For this, we ignore the provided value of add_aaxp because it doesn't
	// apply.  XP per AA is normalized such that there are X white con kills
	// per AA.

	uint32 whiteConKillsPerAA = RuleI(AA, NormalizedAANumberOfWhiteConPerAA);
	uint32 xpPerAA = RuleI(AA, ExpPerPoint);

	float colorModifier = GetConLevelModifierPercent(conlevel);
	float percentToAAXp = (float)m_epp.perAA / 100;

	// Normalize the amount of AA XP we earned for this kill.
	add_aaxp = percentToAAXp * (xpPerAA / (whiteConKillsPerAA / colorModifier));
}

void Client::CalculateStandardAAExp(uint64 &add_aaxp, uint8 conlevel, bool resexp)
{
    if (!resexp)
    {
        //if XP scaling is based on the con of a monster, do that now.
        if (RuleB(Character, UseXPConScaling))
        {
            if (conlevel != 0xFF && !resexp)
            {
                add_aaxp *= GetConLevelModifierPercent(conlevel);
            }
        }
    }	//end !resexp

    float aatotalmod = 1.0;
    if (zone->newzone_data.zone_exp_multiplier >= 0) {
        aatotalmod *= zone->newzone_data.zone_exp_multiplier;
    }

    // Shouldn't race not affect AA XP?
    if (RuleB(Character, UseRaceClassExpBonuses))
    {
        if (GetBaseRace() == HALFLING) {
            aatotalmod *= 1.05;
        }

        if (GetClass() == Class::Rogue || GetClass() == Class::Warrior) {
            aatotalmod *= 1.05;
        }
    }

    // why wasn't this here? Where should it be?
    if (zone->IsHotzone())
    {
        aatotalmod += RuleR(Zone, HotZoneBonus);
    }

    if (RuleB(Zone, LevelBasedEXPMods)) {
        if (zone->level_exp_mod[GetLevel()].ExpMod) {
            add_aaxp *= zone->level_exp_mod[GetLevel()].AAExpMod;
        }
    }

    if (RuleR(Character, FinalExpMultiplier) >= 0) {
        add_aaxp *= RuleR(Character, FinalExpMultiplier);
    }

    // PATCH: Add safety and logging for AAEXP modifier
    if (RuleB(Character, EnableCharacterEXPMods)) {
        float aa_mod = zone->GetAAEXPModifier(this);
        if (aa_mod <= 0.0f) {
            LogError("[AAEXP] zone->GetAAEXPModifier returned [{}] for [{}] (character_id: {}), defaulting to 1.0!", aa_mod, GetCleanName(), CharacterID());
            aa_mod = 1.0f;
        } else {
            LogError("[AAEXP] zone->GetAAEXPModifier returned [{}] for [{}] (character_id: {})", aa_mod, GetCleanName(), CharacterID());
        }
        add_aaxp *= aa_mod;
    }

    add_aaxp = (uint64)(RuleR(Character, AAExpMultiplier) * add_aaxp * aatotalmod);
}

void Client::CalculateLeadershipExp(uint64 &add_exp, uint8 conlevel)
{
	if (IsLeadershipEXPOn() && (conlevel == ConsiderColor::DarkBlue || conlevel == ConsiderColor::White || conlevel == ConsiderColor::Yellow || conlevel == ConsiderColor::Red))
	{
		add_exp = static_cast<uint64>(static_cast<float>(add_exp) * 0.8f);

		if (GetGroup())
		{
			if (m_pp.group_leadership_points < MaxBankedGroupLeadershipPoints(GetLevel())
				&& RuleI(Character, KillsPerGroupLeadershipAA) > 0)
			{
				uint64 exp = GROUP_EXP_PER_POINT / RuleI(Character, KillsPerGroupLeadershipAA);
				Client *mentoree = GetGroup()->GetMentoree();
				if (GetGroup()->GetMentorPercent() && mentoree &&
					mentoree->GetGroupPoints() < MaxBankedGroupLeadershipPoints(mentoree->GetLevel()))
				{
					uint64 mentor_exp = exp * (GetGroup()->GetMentorPercent() / 100.0f);
					exp -= mentor_exp;
					mentoree->AddLeadershipEXP(mentor_exp, 0); // ends up rounded down
					mentoree->MessageString(Chat::LeaderShip, GAIN_GROUP_LEADERSHIP_EXP);
				}
				if (exp > 0)
				{
					// possible if you mentor 100% to the other client
					AddLeadershipEXP(exp, 0); // ends up rounded up if mentored, no idea how live actually does it
					MessageString(Chat::LeaderShip, GAIN_GROUP_LEADERSHIP_EXP);
				}
			}
			else
			{
				MessageString(Chat::LeaderShip, MAX_GROUP_LEADERSHIP_POINTS);
			}
		}
		else
		{
			Raid *raid = GetRaid();
			// Raid leaders CAN NOT gain group AA XP, other group leaders can though!
			if (raid->IsLeader(this))
			{
				if (m_pp.raid_leadership_points < MaxBankedRaidLeadershipPoints(GetLevel())
					&& RuleI(Character, KillsPerRaidLeadershipAA) > 0)
				{
					AddLeadershipEXP(0, RAID_EXP_PER_POINT / RuleI(Character, KillsPerRaidLeadershipAA));
					MessageString(Chat::LeaderShip, GAIN_RAID_LEADERSHIP_EXP);
				}
				else
				{
					MessageString(Chat::LeaderShip, MAX_RAID_LEADERSHIP_POINTS);
				}
			}
			else
			{
				if (m_pp.group_leadership_points < MaxBankedGroupLeadershipPoints(GetLevel())
					&& RuleI(Character, KillsPerGroupLeadershipAA) > 0)
				{
					uint32 group_id = raid->GetGroup(this);
					uint64 exp = GROUP_EXP_PER_POINT / RuleI(Character, KillsPerGroupLeadershipAA);
					Client *mentoree = raid->GetMentoree(group_id);
					if (raid->GetMentorPercent(group_id) && mentoree &&
						mentoree->GetGroupPoints() < MaxBankedGroupLeadershipPoints(mentoree->GetLevel()))
					{
						uint64 mentor_exp = exp * (raid->GetMentorPercent(group_id) / 100.0f);
						exp -= mentor_exp;
						mentoree->AddLeadershipEXP(mentor_exp, 0);
						mentoree->MessageString(Chat::LeaderShip, GAIN_GROUP_LEADERSHIP_EXP);
					}
					if (exp > 0)
					{
						AddLeadershipEXP(exp, 0);
						MessageString(Chat::LeaderShip, GAIN_GROUP_LEADERSHIP_EXP);
					}
				}
				else
				{
					MessageString(Chat::LeaderShip, MAX_GROUP_LEADERSHIP_POINTS);
				}
			}
		}
	}
}

void Client::CalculateExp(uint64 in_add_exp, uint64 &add_exp, uint64 &add_aaxp, uint8 conlevel, bool resexp)
{
	add_exp = in_add_exp;

	if (!resexp && (XPRate != 0))
	{
		add_exp = static_cast<uint64>(in_add_exp * (static_cast<float>(XPRate) / 100.0f));
	}

	// Make sure it was initialized.
	add_aaxp = 0;

	if (!resexp)
	{
		//figure out how much of this goes to AAs
		add_aaxp = add_exp * m_epp.perAA / 100;

		//take that amount away from regular exp
		add_exp -= add_aaxp;

		float totalmod = 1.0;
		float zemmod = 1.0;

		//get modifiers
		if (RuleR(Character, ExpMultiplier) >= 0) {
			totalmod *= RuleR(Character, ExpMultiplier);
		}

		//add the zone exp modifier.
		if (zone->newzone_data.zone_exp_multiplier >= 0) {
			zemmod *= zone->newzone_data.zone_exp_multiplier;
		}

		if (RuleB(Character, UseRaceClassExpBonuses))
		{
			if (GetBaseRace() == HALFLING) {
				totalmod *= 1.05;
			}

			if (GetClass() == Class::Rogue || GetClass() == Class::Warrior) {
				totalmod *= 1.05;
			}
		}

		//add hotzone modifier if one has been set.
		if (zone->IsHotzone())
		{
			totalmod += RuleR(Zone, HotZoneBonus);
		}

		add_exp = uint64(float(add_exp) * totalmod * zemmod);

		//if XP scaling is based on the con of a monster, do that now.
		if (RuleB(Character, UseXPConScaling))
		{
			if (conlevel != 0xFF && !resexp)
			{
				add_exp = add_exp * GetConLevelModifierPercent(conlevel);
			}
		}

		// Calculate any changes to leadership experience.
		CalculateLeadershipExp(add_exp, conlevel);
	}	//end !resexp

	if (RuleB(Zone, LevelBasedEXPMods)) {
		if (zone->level_exp_mod[GetLevel()].ExpMod) {
			add_exp *= zone->level_exp_mod[GetLevel()].ExpMod;
		}
	}

	if (RuleR(Character, FinalExpMultiplier) >= 0) {
		add_exp *= RuleR(Character, FinalExpMultiplier);
	}

	if (RuleB(Character, EnableCharacterEXPMods)) {
		add_exp *= zone->GetEXPModifier(this);
	}

	//Enforce Percent XP Cap per kill, if rule is enabled
	int kill_percent_xp_cap = RuleI(Character, ExperiencePercentCapPerKill);
	if (kill_percent_xp_cap >= 0) {
		auto experience_for_level = (GetEXPForLevel(GetLevel() + 1) - GetEXPForLevel(GetLevel()));
		float exp_percent = (float)((float)add_exp / (float)(GetEXPForLevel(GetLevel() + 1) - GetEXPForLevel(GetLevel()))) * (float)100; //EXP needed for level
		if (exp_percent > kill_percent_xp_cap) {
			add_exp = GetEXP() + static_cast<uint64>(std::floor(experience_for_level * (kill_percent_xp_cap / 100.0f)));
			return;
		}
	}

	add_exp = GetEXP() + add_exp;
}

void Client::AddEXP(ExpSource exp_source, uint64 in_add_exp, uint8 conlevel, bool resexp, NPC* npc) {
    LogError("[AddEXP] Called for [{}]: exp_source [{}], in_add_exp [{}], conlevel [{}], resexp [{}], npc [{}]",
        GetCleanName(), static_cast<int>(exp_source), in_add_exp, conlevel, resexp, npc ? npc->GetCleanName() : "nullptr");

    if (!IsEXPEnabled()) {
        LogError("[AddEXP] EXP is not enabled for [{}]. Aborting AddEXP.", GetCleanName());
        return;
    }

    uint64 exp = 0;
    uint64 aaexp = 0;

    if (m_epp.perAA < 0 || m_epp.perAA > 100) {
        LogError("[AddEXP] Invalid perAA value [{}] for [{}], resetting to 0.", m_epp.perAA, GetCleanName());
        m_epp.perAA = 0;
    }

    // Calculate XP/AA
    CalculateExp(in_add_exp, exp, aaexp, conlevel, resexp);
    LogError("[AddEXP] After CalculateExp: exp [{}], aaexp [{}]", exp, aaexp);

    // AA XP Calculation (old/new style)
    if (!RuleB(AA, NormalizedAAEnabled)) {
        CalculateStandardAAExp(aaexp, conlevel, resexp);
        LogError("[AddEXP] After CalculateStandardAAExp: aaexp [{}]", aaexp);
    } else {
        CalculateNormalizedAAExp(aaexp, conlevel, resexp);
        LogError("[AddEXP] After CalculateNormalizedAAExp: aaexp [{}]", aaexp);
    }

    if (RuleB(AA, ModernAAScalingEnabled) && aaexp > 0) {
        uint64 prev_aaexp = aaexp;
        aaexp = ScaleAAXPBasedOnCurrentAATotal(GetSpentAA() + GetAAPoints(), aaexp);
        LogError("[AddEXP] After ScaleAAXPBasedOnCurrentAATotal: was [{}], now [{}]", prev_aaexp, aaexp);
    }

    // Cap AA XP per kill
    if (RuleI(AA, MaxAAEXPPerKill) >= 0 && aaexp > RuleI(AA, MaxAAEXPPerKill)) {
        LogError("[AddEXP] AA XP [{}] exceeded MaxAAEXPPerKill [{}], capping.", aaexp, RuleI(AA, MaxAAEXPPerKill));
        aaexp = RuleI(AA, MaxAAEXPPerKill);
    }

    uint32 had_aaexp = GetAAXP();
    aaexp += had_aaexp;

    if (aaexp < had_aaexp) {
        LogError("[AddEXP] AAEXP wrap detected for [{}] ({} < {}), resetting.", GetCleanName(), aaexp, had_aaexp);
        aaexp = had_aaexp;
    }

    int aa_cap = RuleI(AA, UnusedAAPointCap);
    if (aa_cap >= 0 && aaexp > 0) {
        if (m_pp.aapoints == aa_cap) {
            LogError("[AddEXP] Unused AA cap [{}] reached for [{}], zeroing aaexp and perAA.", aa_cap, GetCleanName());
            MessageString(Chat::Red, AA_CAP);
            aaexp = 0;
            m_epp.perAA = 0;
        } else if (m_pp.aapoints > aa_cap) {
            LogError("[AddEXP] Over Unused AA cap [{}] for [{}] ({} > {}), setting aapoints=cap and zeroing aaexp/perAA.",
                aa_cap, GetCleanName(), m_pp.aapoints, aa_cap);
            MessageString(Chat::Red, OVER_AA_CAP, fmt::format_int(aa_cap).c_str(), fmt::format_int(aa_cap).c_str());
            m_pp.aapoints = aa_cap;
            aaexp = 0;
            m_epp.perAA = 0;
        }
    }

    if (GetLevel() <= 50 && m_epp.perAA > 0) {
        LogError("[AddEXP] Player [{}] < level 51 and perAA [{}] > 0. Zeroing aaexp and perAA.", GetCleanName(), m_epp.perAA);
        Message(Chat::Yellow, "You are below the level allowed to gain AA Experience. AA Experience set to 0%%");
        aaexp = 0;
        m_epp.perAA = 0;
    }

    // Before SetEXP, log what will be granted
    LogError("[AddEXP] About to call SetEXP: exp [{}], aaexp [{}] (current EXP [{}], current AA [{}]) for [{}]",
        exp, aaexp, GetEXP(), GetAAXP(), GetCleanName());

    // SUGGESTION: If player is at cap for their level, and exp==cap, only AA will be gained
    // You could log this condition for clarity (OPTIONAL):
    auto max_level = GetClientMaxLevel() ? (GetClientMaxLevel() + 1) : (RuleI(Character, MaxExpLevel) + 1);
    if (GetLevel() == max_level - 1 && GetEXP() == GetEXPForLevel(max_level)) {
        LogError("[AddEXP] [{}] is at max level [{}] and max EXP cap [{}]. Only AA XP may increase.", GetCleanName(), GetLevel(), GetEXPForLevel(max_level));
    }

    SetEXP(exp_source, exp, aaexp, resexp, npc);
}

void Client::SetEXP(ExpSource exp_source, uint64 set_exp, uint64 set_aaxp, bool isrezzexp, NPC* npc) {
    LogError("[SetEXP] Called for [{}]: exp_source [{}], set_exp [{}], set_aaxp [{}], isrezzexp [{}], npc [{}]",
        GetCleanName(), static_cast<int>(exp_source), set_exp, set_aaxp, isrezzexp, npc ? npc->GetCleanName() : "nullptr");

    uint64 current_exp = GetEXP();
    uint64 current_aa_exp = GetAAXP();
    uint64 total_current_exp = current_exp + current_aa_exp;
    uint64 total_add_exp = set_exp + set_aaxp;

#ifdef LUA_EQEMU
    uint64 lua_ret = 0;
    bool ignore_default = false;
    LogError("[SetEXP] LUA_EQEMU: Calling LuaParser::SetEXP...");
    lua_ret = LuaParser::Instance()->SetEXP(this, exp_source, current_exp, set_exp, isrezzexp, ignore_default);
    LogError("[SetEXP] LUA_EQEMU: LuaParser::SetEXP returned [{}], ignore_default [{}]", lua_ret, ignore_default);
    if (ignore_default) {
        set_exp = lua_ret;
        LogError("[SetEXP] LUA_EQEMU: set_exp overridden by Lua to [{}]", set_exp);
    }

    lua_ret = 0;
    ignore_default = false;
    LogError("[SetEXP] LUA_EQEMU: Calling LuaParser::SetAAEXP...");
    lua_ret = LuaParser::Instance()->SetAAEXP(this, exp_source, current_aa_exp, set_aaxp, isrezzexp, ignore_default);
    LogError("[SetEXP] LUA_EQEMU: LuaParser::SetAAEXP returned [{}], ignore_default [{}]", lua_ret, ignore_default);
    if (ignore_default) {
        set_aaxp = lua_ret;
        LogError("[SetEXP] LUA_EQEMU: set_aaxp overridden by Lua to [{}]", set_aaxp);
    }
    total_add_exp = set_exp + set_aaxp;
#endif

    LogError("[SetEXP] After Lua override: set_exp [{}], set_aaxp [{}]", set_exp, set_aaxp);

    auto max_AAXP = GetRequiredAAExperience();
    if (max_AAXP == 0 || GetEXPForLevel(GetLevel()) == 0xFFFFFFFF) {
        LogError("[SetEXP] INVALID max_AAXP [{}] or GetEXPForLevel({}) returned 0xFFFFFFFF -- aborting EXP grant!", max_AAXP, GetLevel());
        Message(Chat::Red, "Error in Client::SetEXP. EXP not set.");
        return;
    }

    // Determine group member count
    uint32 membercount = 0;
    if (GetGroup()) {
        for (uint32 i = 0; i < MAX_GROUP_MEMBERS; i++) {
            if (GetGroup()->members[i] != nullptr) {
                membercount++;
            }
        }
    }
    LogError("[SetEXP] Group member count: [{}]", membercount);

    // Calculate max level, with bucket override
    uint8 max_level = RuleI(Character, MaxExpLevel) + 1;
    if (max_level <= 1) {
        max_level = RuleI(Character, MaxLevel) + 1;
        LogError("[SetEXP] Fallback max_level: [{}]", max_level);
    }
    auto client_max_level = GetClientMaxLevel();
    if (client_max_level) {
        max_level = client_max_level + 1;
        LogError("[SetEXP] Client data bucket max_level: [{}]", max_level);
    }

    // Logging: current vs intended EXP/AA XP
    LogError("[SetEXP] Player [{}] is level [{}], EXP [{}], AAEXP [{}]", GetCleanName(), GetLevel(), current_exp, current_aa_exp);

    // --- BEGIN: LEVEL CAP/CLAMP LOGIC ---
    uint16 check_level = GetLevel() + 1;
    bool level_increase = true;
    int8 level_count = 0;

    // Up-level loop
    while (set_exp >= GetEXPForLevel(check_level)) {
        LogError("[SetEXP] set_exp [{}] >= EXPForLevel({}) [{}] -- incrementing level (possible level up)",
            set_exp, check_level, GetEXPForLevel(check_level));
        check_level++;
        if (check_level > 127) {
            LogError("[SetEXP] Level up hard cap (127) reached, breaking loop");
            check_level = 127;
            break;
        }
        level_count++;
        if (GetMercenaryID()) {
            LogError("[SetEXP] Updating merc level after up-level");
            UpdateMercLevel();
        }
    }

    // Down-level loop
    while (set_exp < GetEXPForLevel(check_level - 1)) {
        LogError("[SetEXP] set_exp [{}] < EXPForLevel({}) [{}] -- decrementing level (possible delevel)",
            set_exp, check_level - 1, GetEXPForLevel(check_level - 1));
        check_level--;
        if (check_level < 2) {
            LogError("[SetEXP] Delevel hard minimum (2) reached, breaking loop");
            check_level = 2;
            break;
        }
        level_increase = false;
        if (GetMercenaryID()) {
            LogError("[SetEXP] Updating merc level after down-level");
            UpdateMercLevel();
        }
    }
    check_level--;

    // --- SUGGESTION: Clamp EXP at the max allowed for this client! ---
    // This is CRUCIAL for correct capped behavior (and suggested for your problem)
    if (check_level > max_level) {
        LogError("[SetEXP] check_level [{}] > max_level [{}] -- capping to max", check_level, max_level);
        check_level = max_level;
        if (RuleB(Character, KeepLevelOverMax)) {
            set_exp = GetEXPForLevel(GetLevel() + 1);
            LogError("[SetEXP] KeepLevelOverMax: set_exp set to EXPForLevel({}) [{}]", GetLevel() + 1, set_exp);
        } else {
            set_exp = GetEXPForLevel(max_level);
            LogError("[SetEXP] Capping set_exp to EXPForLevel({}) [{}]", max_level, set_exp);
        }
    }

    // --- Extra clamp: If at max level, never allow set_exp to go past cap
    if (GetLevel() == max_level - 1) {
        uint32 expneeded = GetEXPForLevel(max_level);
        if (set_exp > expneeded) {
            LogError("[SetEXP] At level cap [{}], clamping set_exp [{}] to exp cap [{}]", GetLevel(), set_exp, expneeded);
            set_exp = expneeded;
        }
    }
    // --- END: LEVEL CAP/CLAMP LOGIC ---

    // --- BEGIN: EXP GAIN/LOSS LOGIC ---
    if (total_add_exp > total_current_exp) {
		uint64 exp_gained = set_exp > current_exp ? set_exp - current_exp : 0;
		uint64 aa_exp_gained = set_aaxp > current_aa_exp ? set_aaxp - current_aa_exp : 0;

		// Build amount message if needed
		std::string exp_amount_message;
		if (RuleI(Character, ShowExpValues) >= 1) {
			if (exp_gained > 0 && aa_exp_gained > 0) {
				exp_amount_message = fmt::format("({}) ({})", exp_gained, aa_exp_gained);
			} else if (exp_gained > 0) {
				exp_amount_message = fmt::format("({})", exp_gained);
			} else if (aa_exp_gained > 0) {
				exp_amount_message = fmt::format("({}) AA", aa_exp_gained);
			}
		}

		// Build percent message if needed
		std::string exp_percent_message;
		if (RuleI(Character, ShowExpValues) >= 2) {
			float exp_percent = 0.0f;
			float aa_exp_percent = 0.0f;
			if (exp_gained > 0) {
				exp_percent = 100.0f * (float)(exp_gained) / (float)(GetEXPForLevel(GetLevel() + 1) - GetEXPForLevel(GetLevel()));
			}
			if (aa_exp_gained > 0) {
				aa_exp_percent = 100.0f * (float)(aa_exp_gained) / (float)RuleI(AA, ExpPerPoint);
			}

			if (exp_gained > 0 && aa_exp_gained > 0) {
				exp_percent_message = StringFormat("(%.3f%%, %.3f%%AA)", exp_percent, aa_exp_percent);
			} else if (exp_gained > 0) {
				exp_percent_message = StringFormat("(%.3f%%)", exp_percent);
			} else if (aa_exp_gained > 0) {
				exp_percent_message = StringFormat("(%.3f%%AA)", aa_exp_percent);
			}
		}

		if (isrezzexp) {
			// Resurrection experience gain
			if (RuleI(Character, ShowExpValues) > 0) {
				Message(Chat::Experience, "You regain %s experience from resurrection. %s", exp_amount_message.c_str(), exp_percent_message.c_str());
			} else {
				MessageString(Chat::Experience, REZ_REGAIN);
			}
		} else {
			// Non-resurrection XP gain
			if (membercount > 1) {
				// Group
				if (RuleI(Character, ShowExpValues) > 0) {
					Message(Chat::Experience, "You have gained %s party experience! %s", exp_amount_message.c_str(), exp_percent_message.c_str());
				} else if (zone->IsHotzone()) {
					Message(Chat::Experience, "You gain party experience (with a bonus)!");
				} else {
					MessageString(Chat::Experience, GAIN_GROUPXP);
				}
			} else if (IsRaidGrouped()) {
				// Raid
				if (RuleI(Character, ShowExpValues) > 0) {
					Message(Chat::Experience, "You have gained %s raid experience! %s", exp_amount_message.c_str(), exp_percent_message.c_str());
				} else if (zone->IsHotzone()) {
					Message(Chat::Experience, "You gained raid experience (with a bonus)!");
				} else {
					MessageString(Chat::Experience, GAIN_RAIDEXP);
				}
			} else {
				// Solo
				if (RuleI(Character, ShowExpValues) > 0) {
					Message(Chat::Experience, "You have gained %s experience! %s", exp_amount_message.c_str(), exp_percent_message.c_str());
				} else if (zone->IsHotzone()) {
					Message(Chat::Experience, "You gain experience (with a bonus)!");
				} else {
					MessageString(Chat::Experience, GAIN_XP);
				}
			}
		}
		ProcessEvolvingItem(exp_gained, npc);
	} else if (total_add_exp < total_current_exp) {
		// Only message if experience is lost
		uint64 exp_lost = current_exp > set_exp ? current_exp - set_exp : 0;
		float exp_percent = 0.0f;
		if (exp_lost > 0) {
			exp_percent = 100.0f * (float)(exp_lost) / (float)(GetEXPForLevel(GetLevel() + 1) - GetEXPForLevel(GetLevel()));
		}

		if (RuleI(Character, ShowExpValues) == 1 && exp_lost > 0) {
			Message(Chat::Yellow, "You have lost %llu experience.", exp_lost);
		} else if (RuleI(Character, ShowExpValues) == 2 && exp_lost > 0) {
			Message(Chat::Yellow, "You have lost %llu experience. (%.3f%%)", exp_lost, exp_percent);
		} else if (exp_lost > 0) {
			Message(Chat::Yellow, "You have lost experience.");
		}
	}
    // --- END: EXP GAIN/LOSS LOGIC ---

	// --- BEGIN: AA LOGIC ---
	if (set_aaxp >= max_AAXP) {
		LogError("[SetEXP] set_aaxp [{}] >= max_AAXP [{}] -- calculating AA point gain", set_aaxp, max_AAXP);
		uint32 last_unspentAA = m_pp.aapoints;

		// How many new AA points from this grant
		m_pp.aapoints = set_aaxp / max_AAXP;
		set_aaxp = set_aaxp - (max_AAXP * m_pp.aapoints);
		m_pp.aapoints += last_unspentAA;

		uint32 gained = m_pp.aapoints - last_unspentAA;
		LogError("[SetEXP] AA points awarded: [{}], total now [{}], leftover aaxp [{}]", gained, m_pp.aapoints, set_aaxp);

		// Messaging for AA points gained
		char val1[20] = {0};
		char val2[20] = {0};

		if (gained == 1 && m_pp.aapoints == 1) {
			// You have gained an ability point!  You now have %1 ability point.
			MessageString(Chat::Experience, GAIN_SINGLE_AA_SINGLE_AA, ConvertArray(m_pp.aapoints, val1));
		} else if (gained == 1 && m_pp.aapoints > 1) {
			// You have gained an ability point!  You now have %1 ability points.
			MessageString(Chat::Experience, GAIN_SINGLE_AA_MULTI_AA, ConvertArray(m_pp.aapoints, val1));
		} else if (gained > 1) {
			// You have gained %1 ability point(s)!  You now have %2 ability point(s).
			MessageString(Chat::Experience, GAIN_MULTI_AA_MULTI_AA, ConvertArray(gained, val1), ConvertArray(m_pp.aapoints, val2));
		}

		if (RuleB(AA, SoundForAAEarned)) {
			SendSound();
		}

		if (parse->PlayerHasQuestSub(EVENT_AA_GAIN)) {
			parse->EventPlayer(EVENT_AA_GAIN, this, std::to_string(gained), 0);
		}

		RecordPlayerEventLog(PlayerEvent::AA_GAIN, PlayerEvent::AAGainedEvent{gained});

		/* QS: PlayerLogAARate */
		if (RuleB(QueryServ, PlayerLogAARate)) {
			int add_points = gained;
			std::string query = StringFormat(
				"INSERT INTO `qs_player_aa_rate_hourly` (char_id, aa_count, hour_time) VALUES (%i, %i, UNIX_TIMESTAMP() - MOD(UNIX_TIMESTAMP(), 3600)) ON DUPLICATE KEY UPDATE `aa_count` = `aa_count` + %i",
				CharacterID(), add_points, add_points
			);
			QServ->SendQuery(query.c_str());
		}
	}
	// --- END: AA LOGIC ---

    // --- BEGIN: LEVEL UP/DOWN LOGIC ---
    if ((GetLevel() != check_level) && !(check_level >= max_level)) {
        LogError("[SetEXP] Level change: [{}] -> [{}]", GetLevel(), check_level);
        uint8 myoldlevel = GetLevel();
        SetLevel(check_level);
        LogError("[SetEXP] SetLevel called with check_level [{}]", check_level);
        if (RuleB(Bots, Enabled) && RuleB(Bots, BotLevelsWithOwner)) {
            LogError("[SetEXP] LevelBotWithClient called for new level [{}]", GetLevel());
            Bot::LevelBotWithClient(this, GetLevel(), (myoldlevel == check_level - 1));
        }
    }
    // --- END: LEVEL UP/DOWN LOGIC ---

    // --- Final clamp, set EXP/AA, and AA toggle ---
    m_pp.exp = set_exp;
    m_pp.expAA = set_aaxp;
    LogError("[SetEXP] m_pp.exp set to [{}], m_pp.expAA set to [{}]", m_pp.exp, m_pp.expAA);

    if (GetLevel() < 51) {
        LogError("[SetEXP] Player <51, disabling AA EXP.");
        m_epp.perAA = 0;
    } else {
        LogError("[SetEXP] Player 51+, updating AA stats.");
        SendAlternateAdvancementStats();
    }

    // --- EXP bar update logic ---
    uint32 tmpxp1 = GetEXPForLevel(GetLevel() + 1);
    uint32 tmpxp2 = GetEXPForLevel(GetLevel());
    if (tmpxp1 != tmpxp2 && tmpxp1 != 0xFFFFFFFF && tmpxp2 != 0xFFFFFFFF) {
        auto outapp = new EQApplicationPacket(OP_ExpUpdate, sizeof(ExpUpdate_Struct));
        ExpUpdate_Struct* eu = (ExpUpdate_Struct*)outapp->pBuffer;
        float tmpxp = (float)((float)set_exp - tmpxp2) / ((float)tmpxp1 - tmpxp2);
        eu->exp = (uint32)(330.0f * tmpxp);
        LogError("[SetEXP] Sending exp update packet (bar: [{}] / 330)", eu->exp);
        FastQueuePacket(&outapp);
    } else {
        LogError("[SetEXP] Skipped exp bar update (divide by zero or invalid exp values)");
    }

    if (admin >= AccountStatus::GMAdmin && GetGM()) {
        char val1[20] = {0};
        char val2[20] = {0};
        char val3[20] = {0};
        LogError("[SetEXP] GM EXP info: set_aaxp [{}], set_exp [{}], EXPForLevel(next) [{}]",
            set_aaxp, set_exp, GetEXPForLevel(GetLevel() + 1));
        MessageString(Chat::Experience, GM_GAINXP, ConvertArray(set_aaxp, val1), ConvertArray(set_exp, val2), ConvertArray(GetEXPForLevel(GetLevel() + 1), val3));
    }

    LogError("[SetEXP] Complete for [{}]: final EXP [{}], AAEXP [{}], level [{}]", GetCleanName(), m_pp.exp, m_pp.expAA, GetLevel());
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

	if (m_pp.level2 != 0) {
		lu->level_old = m_pp.level2;
	} else {
		lu->level_old = level;
	}

	level = set_level;

	if (IsRaidGrouped()) {
		Raid *r = GetRaid();
		if (r) {
			r->UpdateLevel(GetName(), set_level);
		}
	}

	if (set_level > m_pp.level2) {
		if (m_pp.level2 == 0) {
			m_pp.points += 5;
		} else {
			m_pp.points += (5 * (set_level - m_pp.level2));
		}

		m_pp.level2 = set_level;
	}

	if (set_level > m_pp.level) {
		int levels_gained = (set_level - m_pp.level);

		if (parse->PlayerHasQuestSub(EVENT_LEVEL_UP)) {
			parse->EventPlayer(EVENT_LEVEL_UP, this, std::to_string(levels_gained), 0);
		}

		if (player_event_logs.IsEventEnabled(PlayerEvent::LEVEL_GAIN)) {
			auto e = PlayerEvent::LevelGainedEvent{
				.from_level = m_pp.level,
				.to_level = set_level,
				.levels_gained = levels_gained
			};

			RecordPlayerEventLog(PlayerEvent::LEVEL_GAIN, e);
		}
	} else if (set_level < m_pp.level) {
		int levels_lost = (m_pp.level - set_level);

		if (parse->PlayerHasQuestSub(EVENT_LEVEL_DOWN)) {
			parse->EventPlayer(EVENT_LEVEL_DOWN, this, std::to_string(levels_lost), 0);
		}

		if (player_event_logs.IsEventEnabled(PlayerEvent::LEVEL_LOSS)) {
			auto e = PlayerEvent::LevelLostEvent{
				.from_level = m_pp.level,
				.to_level = set_level,
				.levels_lost = levels_lost
			};

			RecordPlayerEventLog(PlayerEvent::LEVEL_LOSS, e);
		}
	}

	m_pp.level = set_level;

	if (command) {
		m_pp.exp = GetEXPForLevel(set_level);
		Message(Chat::Yellow, fmt::format("Welcome to level {}!", set_level).c_str());
		lu->exp = 0;

		AutoGrantAAPoints();
	} else {
		const auto temporary_xp = (
			static_cast<float>(m_pp.exp - GetEXPForLevel(GetLevel())) /
			static_cast<float>(GetEXPForLevel(GetLevel() + 1) - GetEXPForLevel(GetLevel()))
		);
		lu->exp = static_cast<uint32>(330.0f * temporary_xp);
	}

	QueuePacket(outapp);
	safe_delete(outapp);
	SendAppearancePacket(AppearanceType::WhoLevel, set_level); // who level change

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

	if (RuleI(World, PVPMinLevel) > 0 && level >= RuleI(World, PVPMinLevel) && m_pp.pvp == 0) {
		SetPVP(true);
	}

	if (IsInAGuild()) {
		guild_mgr.SendToWorldMemberLevelUpdate(GuildID(), GetLevel(), std::string(GetCleanName()));
		DoGuildTributeUpdate();
	}

	DoTributeUpdate();
	SendHPUpdate();
	SetMana(CalcMaxMana());
	UpdateWho();

	UpdateMercLevel();

	Save();
}

// Note: The client calculates exp separately, we cant change this function
// Add: You can set the values you want now, client will be always sync :) - Merkur
uint32 Client::GetEXPForLevel(uint16 check_level)
{
#ifdef LUA_EQEMU
	uint32 lua_ret = 0;
	bool ignoreDefault = false;
	lua_ret = LuaParser::Instance()->GetEXPForLevel(this, check_level, ignoreDefault);

	if (ignoreDefault) {
		return lua_ret;
	}
#endif

	uint16 check_levelm1 = check_level-1;
	float mod;
	if (check_level < 31)
		mod = 1.0;
	else if (check_level < 36)
		mod = 1.1;
	else if (check_level < 41)
		mod = 1.2;
	else if (check_level < 46)
		mod = 1.3;
	else if (check_level < 52)
		mod = 1.4;
	else if (check_level < 53)
		mod = 1.5;
	else if (check_level < 54)
		mod = 1.6;
	else if (check_level < 55)
		mod = 1.7;
	else if (check_level < 56)
		mod = 1.9;
	else if (check_level < 57)
		mod = 2.1;
	else if (check_level < 58)
		mod = 2.3;
	else if (check_level < 59)
		mod = 2.5;
	else if (check_level < 60)
		mod = 2.7;
	else if (check_level < 61)
		mod = 3.0;
	else
		mod = 3.1;

	float base = (check_levelm1)*(check_levelm1)*(check_levelm1);

	mod *= 1000;

	uint32 finalxp = uint32(base * mod);

	if(RuleB(Character,UseOldRaceExpPenalties))
	{
		float racemod = 1.0;
		if(GetBaseRace() == TROLL || GetBaseRace() == IKSAR) {
			racemod = 1.2;
		} else if(GetBaseRace() == OGRE) {
			racemod = 1.15;
		} else if(GetBaseRace() == BARBARIAN) {
			racemod = 1.05;
		} else if(GetBaseRace() == HALFLING) {
			racemod = 0.95;
		}

		finalxp = uint64(finalxp * racemod);
	}

	if(RuleB(Character,UseOldClassExpPenalties))
	{
		float classmod = 1.0;
		if(GetClass() == Class::Paladin || GetClass() == Class::ShadowKnight || GetClass() == Class::Ranger || GetClass() == Class::Bard) {
			classmod = 1.4;
		} else if(GetClass() == Class::Monk) {
			classmod = 1.2;
		} else if(GetClass() == Class::Wizard || GetClass() == Class::Enchanter || GetClass() == Class::Magician || GetClass() == Class::Necromancer) {
			classmod = 1.1;
		} else if(GetClass() == Class::Rogue) {
			classmod = 0.91;
		} else if(GetClass() == Class::Warrior) {
			classmod = 0.9;
		}

		finalxp = uint64(finalxp * classmod);
	}

	return finalxp;
}

void Client::AddLevelBasedExp(ExpSource exp_source, uint8 exp_percentage, uint8 max_level, bool ignore_mods)
{
	uint64	award;
	uint64	xp_for_level;

	if (exp_percentage > 100)
	{
		exp_percentage = 100;
	}

	if (!max_level || GetLevel() < max_level)
	{
		max_level = GetLevel();
	}

	xp_for_level = GetEXPForLevel(max_level + 1) - GetEXPForLevel(max_level);
	award = xp_for_level * exp_percentage / 100;

	if (RuleB(Zone, LevelBasedEXPMods) && !ignore_mods) {
		if (zone->level_exp_mod[GetLevel()].ExpMod) {
			award *= zone->level_exp_mod[GetLevel()].ExpMod;
		}
	}

	if (RuleR(Character, FinalExpMultiplier) >= 0) {
		award *= RuleR(Character, FinalExpMultiplier);
	}

	uint64 newexp = GetEXP() + award;
	SetEXP(exp_source, newexp, GetAAXP());
}

void Group::SplitExp(ExpSource exp_source, const uint64 exp, Mob* other) {
	if (other->CastToNPC()->MerchantType != 0) {
		return;
	}

	if (other->GetOwner() && other->GetOwner()->IsClient()) {
		return;
	}

	const auto member_count = GroupCount();
	if (!member_count) {
		return;
	}

	auto       group_experience = exp;
	const auto highest_level    = GetHighestLevel();

	auto group_modifier = 1.0f;
	if (RuleB(Character, EnableGroupMemberEXPModifier)) {
		if (EQ::ValueWithin(member_count, 2, 5)) {
			group_modifier = 1 + RuleR(Character, GroupMemberEXPModifier) * (member_count - 1); // 2 = 1.2x, 3 = 1.4x, 4 = 1.6x, 5 = 1.8x
		} else if (member_count == 6) {
			group_modifier = RuleR(Character, FullGroupEXPModifier);
		}
	}

	if (EQ::ValueWithin(member_count, 2, 6)) {
		if (RuleB(Character, EnableGroupEXPModifier)) {
			group_experience += static_cast<uint64>(
				static_cast<float>(exp) *
				group_modifier *
				RuleR(Character, GroupExpMultiplier)
			);
		} else {
			group_experience += static_cast<uint64>(
				static_cast<float>(exp) *
				group_modifier
			);
		}
	}

	const uint8 consider_level = Mob::GetLevelCon(highest_level, other->GetLevel());
	if (consider_level == ConsiderColor::Gray) {
		return;
	}

	for (const auto& m : members) {
		if (m && m->IsClient()) {
			const int32 diff     = m->GetLevel() - highest_level;
			int32       max_diff = -(m->GetLevel() * 15 / 10 - m->GetLevel());

			if (max_diff > -5) {
				max_diff = -5;
			}

			if (diff >= max_diff) {
				const uint64 tmp  = (m->GetLevel() + 3) * (m->GetLevel() + 3) * 75 * 35 / 10;
				const uint64 tmp2 = group_experience / member_count;
				m->CastToClient()->AddEXP(exp_source, tmp < tmp2 ? tmp : tmp2, consider_level, false, other->CastToNPC());
			}
		}
	}
}

void Raid::SplitExp(ExpSource exp_source, const uint64 exp, Mob* other) {
	if (other->CastToNPC()->MerchantType != 0) {
		return;
	}

	if (other->GetOwner() && other->GetOwner()->IsClient()) {
		return;
	}

	const auto member_count = RaidCount();
	if (!member_count) {
		return;
	}

	auto       raid_experience = exp;
	const auto highest_level   = GetHighestLevel();

	if (RuleB(Character, EnableRaidEXPModifier)) {
		raid_experience = static_cast<uint64>(static_cast<float>(raid_experience) *	(1.0f - RuleR(Character, RaidExpMultiplier)));
	}

	raid_experience = static_cast<uint64>(static_cast<float>(raid_experience) * RuleR(Character, FinalRaidExpMultiplier));

	const auto consider_level = Mob::GetLevelCon(highest_level, other->GetLevel());
	if (consider_level == ConsiderColor::Gray) {
		return;
	}

	uint32 member_modifier = 1;
	if (RuleB(Character, EnableRaidMemberEXPModifier)) {
		member_modifier = member_count;
	}

	for (const auto& m : members) {
		if (m.member && !m.is_bot) {
			const int32 diff     = m.member->GetLevel() - highest_level;
			int32       max_diff = -(m.member->GetLevel() * 15 / 10 - m.member->GetLevel());

			if (max_diff > -5) {
				max_diff = -5;
			}

			if (diff >= max_diff) {
				const uint64 tmp  = (m.member->GetLevel() + 3) * (m.member->GetLevel() + 3) * 75 * 35 / 10;
				const uint64 tmp2 = (raid_experience / member_modifier) + 1;
				m.member->AddEXP(exp_source, tmp < tmp2 ? tmp : tmp2, consider_level, false, other->CastToNPC());
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

	const uint32 zone_id = zone ? zone->GetZoneID() : 0;

	if (char_cache) {
		QGlobalCache::Combine(global_map, char_cache->GetBucket(), 0, CharacterID(), zone_id);
	}

	for (const auto& global : global_map) {
		if (global.name == "CharMaxLevel") {
			if (Strings::IsNumber(global.value)) {
				return static_cast<uint8>(Strings::ToUnsignedInt(global.value));
			}
		}
	}

	return 0;
}

uint8 Client::GetCharMaxLevelFromBucket()
{
	DataBucketKey k = GetScopedBucketKeys();
	k.key = "CharMaxLevel";

	auto b = DataBucket::GetData(k);
	if (!b.value.empty()) {
		if (Strings::IsNumber(b.value)) {
			return static_cast<uint8>(Strings::ToUnsignedInt(b.value));
		}
	}

	return 0;
}

uint32 Client::GetRequiredAAExperience() {
#ifdef LUA_EQEMU
	uint32 lua_ret = 0;
	bool ignoreDefault = false;
	lua_ret = LuaParser::Instance()->GetRequiredAAExperience(this, ignoreDefault);

	if (ignoreDefault) {
		return lua_ret;
	}
#endif

	return RuleI(AA, ExpPerPoint);
}
