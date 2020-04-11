/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2018 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include "client.h"
#include "mob.h"
#include "../common/races.h"
#include "../common/say_link.h"

inline std::string GetMobAttributeByString(Mob *mob, const std::string &attribute)
{
    if (attribute == "ac") {
        return std::to_string(mob->GetAC());
    }

    if (attribute == "atk") {
        return std::to_string(mob->GetATK());
    }

    if (attribute == "end") {
        int endurance = 0;
        if (mob->IsClient()) {
            endurance = mob->CastToClient()->GetEndurance();
        }

        return std::to_string(endurance);
    }

    if (attribute == "hp") {
        return std::to_string(mob->GetHP());
    }

    if (attribute == "mana") {
        return std::to_string(mob->GetMana());
    }

    if (attribute == "str") {
        return std::to_string(mob->GetSTR());
    }

    if (attribute == "sta") {
        return std::to_string(mob->GetSTA());
    }

    if (attribute == "dex") {
        return std::to_string(mob->GetDEX());
    }

    if (attribute == "agi") {
        return std::to_string(mob->GetAGI());
    }

    if (attribute == "int") {
        return std::to_string(mob->GetINT());
    }

    if (attribute == "wis") {
        return std::to_string(mob->GetWIS());
    }

    if (attribute == "cha") {
        return std::to_string(mob->GetCHA());
    }

    if (attribute == "mr") {
        return std::to_string(mob->GetMR());
    }

    if (attribute == "cr") {
        return std::to_string(mob->GetCR());
    }

    if (attribute == "fr") {
        return std::to_string(mob->GetFR());
    }

    if (attribute == "pr") {
        return std::to_string(mob->GetPR());
    }

    if (attribute == "dr") {
        return std::to_string(mob->GetDR());
    }

    if (attribute == "cr") {
        return std::to_string(mob->GetCR());
    }

    if (attribute == "pr") {
        return std::to_string(mob->GetPR());
    }

    if (attribute == "cor") {
        return std::to_string(mob->GetCorrup());
    }

    if (attribute == "phy") {
        return std::to_string(mob->GetPhR());
    }

    if (attribute == "name") {
        return mob->GetCleanName();
    }

    if (attribute == "surname") {
        std::string last_name = mob->GetLastName();
        return (last_name.length() > 0 ? mob->GetLastName() : " ");
    }

    if (attribute == "race") {
        return GetRaceIDName(mob->GetRace());
    }

    if (attribute == "class") {
        return GetClassIDName(mob->GetClass(), 0);
    }

    if (attribute == "level") {
        return std::to_string(mob->GetLevel());
    }

    if (attribute == "flymode") {
        return std::to_string(mob->GetFlyMode());
    }

    if (attribute == "maxbuffslots") {
        return std::to_string(mob->GetMaxBuffSlots());
    }

    if (attribute == "curbuffslots") {
        return std::to_string(mob->GetCurrentBuffSlots());
    }

    if (attribute == "tohit") {
        return std::to_string(mob->compute_tohit(EQEmu::skills::SkillHandtoHand));
    }

    if (attribute == "total_to_hit") {
        return std::to_string(mob->GetTotalToHit(EQEmu::skills::SkillHandtoHand, 0));
    }

    if (attribute == "defense") {
        return std::to_string(mob->compute_defense());
    }

    if (attribute == "total_defense") {
        return std::to_string(mob->GetTotalDefense());
    }

    if (attribute == "offense") {
        return std::to_string(mob->offense(EQEmu::skills::SkillHandtoHand));
    }

    if (attribute == "mitigation_ac") {
        return std::to_string(mob->GetMitigationAC());
    }

    if (mob->IsNPC()) {
        NPC *npc = mob->CastToNPC();

        if (attribute == "npcid") {
            return std::to_string(npc->GetNPCTypeID());
        }
        if (attribute == "texture") {
            return std::to_string(npc->GetTexture());
        }
        if (attribute == "bodytype") {
            return std::to_string(npc->GetBodyType());
        }
        if (attribute == "gender") {
            return std::to_string(npc->GetGender());
        }
        if (attribute == "size") {
            return std::to_string((int)npc->GetSize());
        }
        if (attribute == "runspeed") {
            return std::to_string((int)npc->GetRunspeed());
        }
        if (attribute == "walkspeed") {
            return std::to_string((int)npc->GetWalkspeed());
        }
        if (attribute == "spawngroup") {
            return std::to_string(npc->GetSp2());
        }
        if (attribute == "grid") {
            return std::to_string(npc->GetGrid());
        }
        if (attribute == "emote") {
            return std::to_string(npc->GetEmoteID());
        }
        if (attribute == "see_invis") {
            return std::to_string(npc->SeeInvisible());
        }
        if (attribute == "see_invis_undead") {
            return std::to_string(npc->SeeInvisibleUndead());
        }
        if (attribute == "faction") {
            return std::to_string(npc->GetNPCFactionID());
        }
        if (attribute == "loottable") {
            return std::to_string(npc->GetLoottableID());
        }
        if (attribute == "prim_skill") {
            return std::to_string(npc->GetPrimSkill());
        }
        if (attribute == "sec_skill") {
            return std::to_string(npc->GetSecSkill());
        }
        if (attribute == "melee_texture_1") {
            return std::to_string(npc->GetMeleeTexture1());
        }
        if (attribute == "melee_texture_2") {
            return std::to_string(npc->GetMeleeTexture2());
        }
        if (attribute == "aggrorange") {
            return std::to_string((int)npc->GetAggroRange());
        }
        if (attribute == "assistrange") {
            return std::to_string((int)npc->GetAssistRange());
        }
        if (attribute == "findable") {
            return std::to_string(npc->IsFindable());
        }
        if (attribute == "trackable") {
            return std::to_string(npc->IsTrackable());
        }
        if (attribute == "spells_id") {
            return std::to_string(npc->GetNPCSpellsID());
        }
        if (attribute == "roamboxMinX") {
            return std::to_string((int)npc->GetRoamboxMinX());
        }
        if (attribute == "roamboxMaxX") {
            return std::to_string((int)npc->GetRoamboxMaxX());
        }
        if (attribute == "roamboxMinY") {
            return std::to_string((int)npc->GetRoamboxMinY());
        }
        if (attribute == "roamboxMaxY") {
            return std::to_string((int)npc->GetRoamboxMaxY());
        }
        if (attribute == "roamboxMinDelay") {
            return std::to_string((int)npc->GetRoamboxMinDelay());
        }
        if (attribute == "roamboxDelay") {
            return std::to_string((int)npc->GetRoamboxDelay());
        }
        if (attribute == "roamboxDistance") {
            return std::to_string((int)npc->GetRoamboxDistance());
        }
        if (attribute == "proximityMinX") {
            return std::to_string((int)npc->GetProximityMinX());
        }
        if (attribute == "proximityMaxX") {
            return std::to_string((int)npc->GetProximityMaxX());
        }
        if (attribute == "proximityMinY") {
            return std::to_string((int)npc->GetProximityMinY());
        }
        if (attribute == "proximityMaxY") {
            return std::to_string((int)npc->GetProximityMaxY());
        }
        if (attribute == "proximityMinZ") {
            return std::to_string((int)npc->GetProximityMinZ());
        }
        if (attribute == "proximityMaxZ") {
            return std::to_string((int)npc->GetProximityMaxZ());
        }
        if (attribute == "accuracy") {
            return std::to_string((int)npc->GetAccuracyRating());
        }
        if (attribute == "slow_mitigation") {
            return std::to_string((int)npc->GetSlowMitigation());
        }
        if (attribute == "min_hit") {
            return std::to_string((int)npc->GetMinDMG());
        }
        if (attribute == "max_hit") {
            return std::to_string((int)npc->GetMaxDMG());
        }
        if (attribute == "hp_regen") {
            return std::to_string((int)npc->GetHPRegen());
        }
        if (attribute == "attack_delay") {
            return std::to_string(npc->GetAttackDelay());
        }
        if (attribute == "spell_scale") {
            return std::to_string((int)npc->GetSpellScale());
        }
        if (attribute == "heal_scale") {
            return std::to_string((int)npc->GetHealScale());
        }
        if (attribute == "avoidance") {
            return std::to_string((int)npc->GetAvoidanceRating());
        }
        npc->GetNPCEmote(npc->GetEmoteID(), 0);
    }

    if (mob->IsClient()) {
        Client *client = mob->CastToClient();

        if (attribute == "shielding") {
            return std::to_string((int)client->GetShielding()) + " / " +
                   std::to_string((int) RuleI(Character, ItemShieldingCap));
        }
        if (attribute == "spell_shielding") {
            return std::to_string((int)client->GetSpellShield()) + " / " +
                   std::to_string((int) RuleI(Character, ItemSpellShieldingCap));
        }
        if (attribute == "dot_shielding") {
            return std::to_string((int)client->GetDoTShield()) + " / " +
                   std::to_string((int) RuleI(Character, ItemDoTShieldingCap));
        }
        if (attribute == "stun_resist") {
            return std::to_string((int)client->GetStunResist()) + " / " +
                   std::to_string((int) RuleI(Character, ItemStunResistCap));
        }
        if (attribute == "damage_shield") {
            return std::to_string((int)client->GetDS()) + " / " +
                   std::to_string((int) RuleI(Character, ItemDamageShieldCap));
        }
        if (attribute == "avoidance") {
            return std::to_string((int) client->GetAvoidance()) + " / " +
                   std::to_string((int) RuleI(Character, ItemAvoidanceCap));
        }
        if (attribute == "strikethrough") {
            return std::to_string((int) client->GetStrikeThrough()) + " / " +
                   std::to_string((int) RuleI(Character, ItemStrikethroughCap));
        }
        if (attribute == "accuracy") {
            return std::to_string((int) client->GetAccuracy()) + " / " +
                   std::to_string((int) RuleI(Character, ItemAccuracyCap));
        }
        if (attribute == "combat_effects") {
            return std::to_string((int) client->GetCombatEffects()) + " / " +
                   std::to_string((int) RuleI(Character, ItemCombatEffectsCap));
        }
        if (attribute == "heal_amount") {
            return std::to_string((int) client->GetHealAmt()) + " / " +
                   std::to_string((int) RuleI(Character, ItemHealAmtCap));
        }
        if (attribute == "spell_dmg") {
            return std::to_string((int) client->GetSpellDmg()) + " / " +
                   std::to_string((int) RuleI(Character, ItemSpellDmgCap));
        }
        if (attribute == "clairvoyance") {
            return std::to_string((int) client->GetClair()) + " / " +
                   std::to_string((int) RuleI(Character, ItemClairvoyanceCap));
        }
        if (attribute == "ds_mitigation") {
            return std::to_string((int) client->GetDSMit()) + " / " +
                   std::to_string((int) RuleI(Character, ItemDSMitigationCap));
        }
        if (attribute == "hp_regen") {
            return std::to_string((int) client->GetHPRegen()) + " / " +
                   std::to_string((int) RuleI(Character, ItemHealthRegenCap));
        }
        if (attribute == "mana_regen") {
            return std::to_string((int) client->GetManaRegen()) + " / " +
                   std::to_string((int) RuleI(Character, ItemManaRegenCap));
        }
        if (attribute == "end_regen") {
            return std::to_string((int) client->CalcEnduranceRegen()) + " / " +
                   std::to_string((int) client->CalcEnduranceRegenCap());
        }
    }

    if (attribute == "type") {
        std::string entity_type = "Mob";

        if (mob->IsCorpse()) {
            entity_type = "Corpse";
        }

        if (mob->IsNPC()) {
            entity_type = "NPC";
        }

        if (mob->IsClient()) {
            entity_type = "Client";
        }

        return entity_type;
    }

    return "null";
}

inline std::string WriteDisplayInfoSection(
        Mob *mob,
        const std::string &section_name,
        std::vector<std::string> attributes_list,
        int column_count = 3,
        bool display_section_name = false
)
{
    std::string text;

    if (display_section_name) {
        text += "<c \"#FFFF66\">" + section_name + "</c><br>";
    }

    text += "<table><tbody>";

    int  index     = 0;
    bool first_row = true;

    for (const auto &attribute : attributes_list) {
        if (index == 0) {
            if (first_row) {
                text += "<tr>\n";
                first_row = false;
            }
            else {
                text += "</tr><tr>\n";
            }

        }

        std::string attribute_name = attribute;

        if (attribute_name.find('_') != std::string::npos) {
            std::vector<std::string> split_string = split(attribute_name, '_');
            std::string new_attribute_name;
            for (std::string &string_value : split_string) {
                new_attribute_name += ucfirst(string_value) + " ";
            }
            attribute_name = new_attribute_name;
        }

        if (attribute_name.length() <= 3) {
            attribute_name = str_toupper(attribute_name);
        }
        if (attribute_name.length() > 3) {
            attribute_name = ucfirst(attribute_name);
        }

        std::string attribute_value = GetMobAttributeByString(mob, attribute);

        if (attribute_value.length() <= 0) {
            continue;
        }

        text += "<td>" + attribute_name + "</td><td>" + GetMobAttributeByString(mob, attribute) + "</td>";

        if (index == column_count) {
            index = 0;
            continue;
        }

        index++;
    }

    text += "</tr></tbody></table>";

    return text;
}

inline void NPCCommandsMenu(Client* client, NPC* npc)
{
    std::string menu_commands;

    if (npc->GetGrid() > 0) {
        menu_commands += "[" + EQEmu::SayLinkEngine::GenerateQuestSaylink("#grid show", false, "Grid Points") + "] ";
    }

    if (npc->GetEmoteID() > 0) {
        std::string saylink = StringFormat("#emotesearch %u", npc->GetEmoteID());
        menu_commands += "[" + EQEmu::SayLinkEngine::GenerateQuestSaylink(saylink, false, "Emotes") + "] ";
    }

    if (npc->GetLoottableID() > 0) {
        menu_commands += "[" + EQEmu::SayLinkEngine::GenerateQuestSaylink("#npcloot show", false, "Loot") + "] ";
    }

    if (npc->IsProximitySet()) {
        menu_commands += "[" + EQEmu::SayLinkEngine::GenerateQuestSaylink("#proximity show", false, "Proximity") + "] ";
    }

    if (menu_commands.length() > 0) {
        std::string dev_menu = "[" + EQEmu::SayLinkEngine::GenerateQuestSaylink("#devtools", false, "DevTools") + "] ";;
        client->Message(0, "| %s [Show Commands] %s", dev_menu.c_str(), menu_commands.c_str());
    }
}

void Mob::DisplayInfo(Mob *mob)
{
    if (!mob) {
        return;
    }

    // std::vector<std::string> general_stats = {
//
    // 	// "accuracy",
    // 	// "slow_mitigation",
    // 	// "atk",
    // 	// "min_hit",
    // 	// "max_hit",
    // 	// "hp_regen",
    // 	// "attack_delay",
    // 	// "special_abilities"
    // };

    if (this->IsClient()) {
        std::string window_text = "<c \"#FFFF66\">*Drag window open vertically to see all</c><br>";

        Client *client = this->CastToClient();

        if (!client->IsDevToolsWindowEnabled()) {
            return;
        }

        std::vector<std::string> info_attributes = {
                "name",
                "race",
                "surname",
                "class",
        };
        window_text += WriteDisplayInfoSection(mob, "Info", info_attributes, 1, false);

        std::vector<std::string> basic_attributes = {
                "type",
                "level",
                "hp",
                "mana",
                "end",
                "ac",
                "atk"
        };
        window_text += WriteDisplayInfoSection(mob, "Main", basic_attributes, 1, false);

        std::vector<std::string> stat_attributes = {
                "str",
                "sta",
                "agi",
                "dex",
                "wis",
                "int",
                "cha",
        };
        window_text += WriteDisplayInfoSection(mob, "Statistics", stat_attributes, 1, false);

        std::vector<std::string> resist_attributes = {
                "pr",
                "mr",
                "dr",
                "fr",
                "cr",
                "cor",
                "phy",
        };
        window_text += WriteDisplayInfoSection(mob, "Resists", resist_attributes, 1, false);

        std::vector<std::string> calculations = {
                "tohit",
                "total_to_hit",
                "defense",
                "total_defense",
                "offense",
                "mitigation_ac",
        };
        window_text += WriteDisplayInfoSection(mob, "Calculations", calculations, 1, true);

        if (mob->IsClient()) {
            std::vector<std::string> mods = {
                    "hp_regen",
                    "mana_regen",
                    "end_regen",
                    "heal_amount",
                    "spell_dmg",
                    "clairvoyance",
            };
            window_text += WriteDisplayInfoSection(mob, "Mods", mods, 1, true);

            std::vector<std::string> mod_defensive = {
                    "shielding",
                    "spell_shielding",
                    "dot_shielding",
                    "stun_resist",
                    "damage_shield",
                    "ds_mitigation",
                    "avoidance",
            };

            window_text += WriteDisplayInfoSection(mob, "Mod Defensive", mod_defensive, 1, true);

            std::vector<std::string> mod_offensive = {
                    "strikethrough",
                    "accuracy",
                    "combat_effects",
            };
            window_text += WriteDisplayInfoSection(mob, "Mod Offensive", mod_offensive, 1, true);
        }

        if (mob->IsNPC()) {
            NPC *npc = mob->CastToNPC();

            std::vector<std::string> npc_stats = {
                    "accuracy",
                    "slow_mitigation",
                    "min_hit",
                    "max_hit",
                    "hp_regen",
                    "attack_delay",
                    "spell_scale",
                    "heal_scale",
                    "avoidance",
            };

            window_text += WriteDisplayInfoSection(mob, "NPC Stats", npc_stats, 1, true);

            std::vector<std::string> npc_attributes = {
                    "npcid",
                    "texture",
                    "bodytype",
                    "gender",
                    "size",
                    "runspeed",
                    "walkspeed",
                    "spawngroup",
                    "grid",
                    "emote",
                    "see_invis",
                    "see_invis_undead",
                    "faction",
                    "loottable",
                    "prim_skill",
                    "sec_skill",
                    "melee_texture_1",
                    "melee_texture_2",
                    "aggrorange",
                    "assistrange",
                    "findable",
                    "trackable",
                    "flymode",
                    "spells_id",
                    "curbuffslots",
                    "maxbuffslots",
            };

            window_text += WriteDisplayInfoSection(mob, "NPC Attributes", npc_attributes, 1, true);

            /**
            * Print Roambox
            */
            if (npc->GetRoamboxMaxX() != 0 && npc->GetRoamboxMinX() != 0) {
                std::vector<std::string> npc_roambox = {
                        "roamboxMinX",
                        "roamboxMaxX",
                        "roamboxMinY",
                        "roamboxMaxY",
                        "roamboxMinDelay",
                        "roamboxDelay",
                        "roamboxDistance",
                };

                window_text += WriteDisplayInfoSection(mob, "Roambox", npc_roambox, 1, true);
            }

            if (npc->proximity != nullptr) {
                std::vector<std::string> npc_proximity = {
                        "proximity_min_x",
                        "proximity_max_x",
                        "proximity_min_y",
                        "proximity_max_y",
                        "proximity_min_z",
                        "proximity_max_z",
                };

                window_text += WriteDisplayInfoSection(mob, "Proximity", npc_proximity, 1, true);
            }

            client->Message(
                    0,
                    "| # Target: %s",
                    npc->GetCleanName());

            NPCCommandsMenu(client, npc);
        }

        std::cout << "Window Length: " << window_text.length() << std::endl;
        // std::cout << "Window " << window_text << std::endl;

        if (client->GetDisplayMobInfoWindow()) {
            client->SendFullPopup(
                    "GM: Entity Info",
                    window_text.c_str(),
                    EQEmu::popupresponse::MOB_INFO_DISMISS,
                    0,
                    100,
                    0,
                    "Snooze",
                    "OK"
            );
        }
    }
}
