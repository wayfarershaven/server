/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2020 EQEmulator Development Team (https://github.com/EQEmu/Server)
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

#include "expedition_database.h"
#include "expedition.h"
#include "expedition_lockout_timer.h"
#include "zonedb.h"
#include "../common/database.h"
#include <fmt/core.h>

uint32_t ExpeditionDatabase::InsertExpedition(
        uint32_t instance_id, const std::string& expedition_name, uint32_t leader_id,
        uint32_t min_players, uint32_t max_players, bool has_replay_lockout)
{
    LogExpeditionsDetail("Inserting new expedition [{}] leader [{}]", expedition_name, leader_id);

    std::string query = fmt::format(SQL(
                                            INSERT INTO expedition_details
                                            (instance_id, expedition_name, leader_id, min_players, max_players, has_replay_timer)
                                            VALUES
                                                    ({}, '{}', {}, {}, {}, {});
                                    ), instance_id, expedition_name, leader_id, min_players, max_players, has_replay_lockout);

    auto results = database.QueryDatabase(query);
    if (!results.Success())
    {
        LogExpeditions("Failed to obtain an expedition id for [{}]", expedition_name);
        return 0;
    }

    return results.LastInsertedID();
}

MySQLRequestResult ExpeditionDatabase::LoadExpedition(uint32_t expedition_id)
{
    LogExpeditionsDetail("Loading expedition [{}]", expedition_id);

    // no point caching expedition if no members, inner join instead of left
    std::string query = fmt::format(SQL(
                                            SELECT
                                            expedition_details.id,
                                            expedition_details.instance_id,
                                            expedition_details.expedition_name,
                                            expedition_details.leader_id,
                                            expedition_details.min_players,
                                            expedition_details.max_players,
                                            expedition_details.has_replay_timer,
                                            character_data.name leader_name,
                                            expedition_lockouts.event_name,
                                            UNIX_TIMESTAMP(expedition_lockouts.expire_time),
                                            expedition_lockouts.duration,
                                            expedition_lockouts.is_inherited
                                                    FROM expedition_details
                                                    INNER JOIN character_data ON expedition_details.leader_id = character_data.id
                                                    LEFT JOIN expedition_lockouts
                                                    ON expedition_details.id = expedition_lockouts.expedition_id
                                                    AND expedition_lockouts.expire_time > NOW()
                                                    WHERE expedition_details.id = {};
                                    ), expedition_id);

    auto results = database.QueryDatabase(query);
    return results;
}

MySQLRequestResult ExpeditionDatabase::LoadAllExpeditions()
{
    LogExpeditionsDetail("Loading all expeditions from database");

    // load all active expeditions and members to current zone cache
    std::string query = SQL(
            SELECT
            expedition_details.id,
            expedition_details.instance_id,
            expedition_details.expedition_name,
            expedition_details.leader_id,
            expedition_details.min_players,
            expedition_details.max_players,
            expedition_details.has_replay_timer,
            character_data.name leader_name,
            expedition_lockouts.event_name,
            UNIX_TIMESTAMP(expedition_lockouts.expire_time),
            expedition_lockouts.duration,
            expedition_lockouts.is_inherited
                    FROM expedition_details
                    INNER JOIN character_data ON expedition_details.leader_id = character_data.id
                    LEFT JOIN expedition_lockouts
                    ON expedition_details.id = expedition_lockouts.expedition_id
                    AND expedition_lockouts.expire_time > NOW()
                    ORDER BY expedition_details.id;
    );

    auto results = database.QueryDatabase(query);
    return results;
}

MySQLRequestResult ExpeditionDatabase::LoadCharacterLockouts(uint32_t character_id)
{
    LogExpeditionsDetail("Loading character [{}] lockouts", character_id);

    auto query = fmt::format(SQL(
                                     SELECT
                                     UNIX_TIMESTAMP(expire_time),
                                     duration,
                                     expedition_name,
                                     event_name
                                             FROM expedition_character_lockouts
                                             WHERE character_id = {} AND is_pending = FALSE AND expire_time > NOW();
                             ), character_id);

    return database.QueryDatabase(query);
}

MySQLRequestResult ExpeditionDatabase::LoadCharacterLockouts(
        uint32_t character_id, const std::string& expedition_name)
{
    LogExpeditionsDetail("Loading character [{}] lockouts for [{}]", character_id, expedition_name);

    auto query = fmt::format(SQL(
                                     SELECT
                                     UNIX_TIMESTAMP(expire_time),
                                     duration,
                                     event_name
                                             FROM expedition_character_lockouts
                                             WHERE
                                                     character_id = {}
                                             AND is_pending = FALSE
                                             AND expire_time > NOW()
                                             AND expedition_name = '{}';
                             ), character_id, expedition_name);

    return database.QueryDatabase(query);
}

MySQLRequestResult ExpeditionDatabase::LoadExpeditionMembers(uint32_t expedition_id)
{
    LogExpeditionsDetail("Loading all members for expedition [{}]", expedition_id);

    std::string query = fmt::format(SQL(
                                            SELECT
                                            expedition_members.character_id,
                                            expedition_members.is_current_member,
                                            character_data.name
                                                    FROM expedition_members
                                                    INNER JOIN character_data ON expedition_members.character_id = character_data.id
                                                    WHERE expedition_id = {};
                                    ), expedition_id);

    auto results = database.QueryDatabase(query);
    if (!results.Success())
    {
        LogExpeditions("Failed to load expedition [{}] members from db", expedition_id);
    }
    return results;
}

MySQLRequestResult ExpeditionDatabase::LoadValidationData(
        const std::string& character_names, const std::string& expedition_name)
{
    LogExpeditionsDetail("Loading multiple characters data for [{}] request validation", expedition_name);

    // for create validation, loads each character's lockouts and possible current expedition
    auto query = fmt::format(SQL(
                                     SELECT
                                     character_data.id,
                                     character_data.name,
                                     member.expedition_id,
                                     UNIX_TIMESTAMP(lockout.expire_time),
                                     lockout.duration,
                                     lockout.event_name
                                             FROM character_data
                                             LEFT JOIN expedition_character_lockouts lockout
                                             ON character_data.id = lockout.character_id
                                             AND lockout.is_pending = FALSE
                                             AND lockout.expire_time > NOW()
                                             AND lockout.expedition_name = '{}'
                                             LEFT JOIN expedition_members member
                                             ON character_data.id = member.character_id
                                             AND member.is_current_member = TRUE
                                             WHERE character_data.name IN ({})
                                             ORDER BY character_data.id;
                             ), expedition_name, character_names);

    auto results = database.QueryDatabase(query);
    return results;
}

void ExpeditionDatabase::DeleteAllCharacterLockouts(uint32_t character_id)
{
    LogExpeditionsDetail("Deleting all character [{}] lockouts", character_id);

    if (character_id != 0)
    {
        std::string query = fmt::format(SQL(
                                                DELETE FROM expedition_character_lockouts
                                                WHERE character_id = {};
                                        ), character_id);

        database.QueryDatabase(query);
    }
}

void ExpeditionDatabase::DeleteAllCharacterLockouts(
        uint32_t character_id, const std::string& expedition_name)
{
    LogExpeditionsDetail("Deleting all character [{}] lockouts for [{}]", character_id, expedition_name);

    if (character_id != 0 && !expedition_name.empty())
    {
        std::string query = fmt::format(SQL(
                                                DELETE FROM expedition_character_lockouts
                                                WHERE character_id = {} AND expedition_name = '{}';
                                        ), character_id, expedition_name);

        database.QueryDatabase(query);
    }
}

void ExpeditionDatabase::DeleteCharacterLockout(
        uint32_t character_id, const std::string& expedition_name, const std::string& event_name)
{
    LogExpeditionsDetail("Deleting character [{}] lockout: [{}]:[{}]", character_id, expedition_name, event_name);

    auto query = fmt::format(SQL(
                                     DELETE FROM expedition_character_lockouts
                                     WHERE
                                             character_id = {}
                                     AND is_pending = FALSE
                                     AND expedition_name = '{}'
                                     AND event_name = '{}';
                             ), character_id, expedition_name, event_name);

    auto results = database.QueryDatabase(query);
    if (!results.Success())
    {
        LogExpeditions(
                "Failed to delete [{}] event [{}] lockout from character [{}]",
                expedition_name, event_name, character_id
        );
    }
}

void ExpeditionDatabase::DeleteMembersLockout(
        const std::vector<ExpeditionMember>& members,
        const std::string& expedition_name, const std::string& event_name)
{
    LogExpeditionsDetail("Deleting members lockout: [{}]:[{}]", expedition_name, event_name);

    std::string query_character_ids;
    for (const auto& member : members)
    {
        fmt::format_to(std::back_inserter(query_character_ids), "{},", member.char_id);
    }

    if (!query_character_ids.empty())
    {
        query_character_ids.pop_back(); // trailing comma

        auto query = fmt::format(SQL(
                                         DELETE FROM expedition_character_lockouts
                                         WHERE character_id
                                         IN ({})
                                         AND is_pending = FALSE
                                         AND expedition_name = '{}'
                                         AND event_name = '{}';
                                 ), query_character_ids, expedition_name, event_name);

        auto results = database.QueryDatabase(query);
        if (!results.Success())
        {
            LogExpeditions("Failed to delete [{}] event [{}] lockouts", expedition_name, event_name);
        }
    }
}

void ExpeditionDatabase::AssignPendingLockouts(uint32_t character_id, const std::string& expedition_name)
{
    LogExpeditionsDetail("Assigning character [{}] pending lockouts [{}]", character_id, expedition_name);

    auto query = fmt::format(SQL(
                                     UPDATE expedition_character_lockouts
                                     SET is_pending = FALSE
                                     WHERE
                                             character_id = {}
                                     AND is_pending = TRUE
                                     AND expedition_name = '{}';
                             ), character_id, expedition_name);

    database.QueryDatabase(query);
}

void ExpeditionDatabase::DeletePendingLockouts(uint32_t character_id)
{
    LogExpeditionsDetail("Deleting character [{}] pending lockouts", character_id);

    auto query = fmt::format(SQL(
                                     DELETE FROM expedition_character_lockouts
                                     WHERE character_id = {} AND is_pending = TRUE;
                             ), character_id);

    database.QueryDatabase(query);
}

void ExpeditionDatabase::DeleteExpedition(uint32_t expedition_id)
{
    LogExpeditionsDetail("Deleting expedition [{}]", expedition_id);

    auto query = fmt::format("DELETE FROM expedition_details WHERE id = {}", expedition_id);
    auto results = database.QueryDatabase(query);
    if (!results.Success())
    {
        LogExpeditions("Failed to delete expedition [{}]", expedition_id);
    }
}

void ExpeditionDatabase::DeleteLockout(uint32_t expedition_id, const std::string& event_name)
{
    LogExpeditionsDetail("Deleting expedition [{}] lockout event [{}]", expedition_id, event_name);

    auto query = fmt::format(SQL(
                                     DELETE FROM expedition_lockouts
                                     WHERE expedition_id = {} AND event_name = '{}';
                             ), expedition_id, event_name);

    auto results = database.QueryDatabase(query);
    if (!results.Success())
    {
        LogExpeditions("Failed to delete expedition [{}] lockout [{}]", expedition_id, event_name);
    }
}

void ExpeditionDatabase::DeleteAllMembers(uint32_t expedition_id)
{
    LogExpeditionsDetail("Deleting all members of expedition [{}]", expedition_id);

    auto query = fmt::format(SQL(
                                     DELETE FROM expedition_members WHERE expedition_id = {};
                             ), expedition_id);

    auto results = database.QueryDatabase(query);
    if (!results.Success())
    {
        LogExpeditions("Failed to delete all members of expedition [{}]", expedition_id);
    }
}

uint32_t ExpeditionDatabase::GetExpeditionIDFromCharacterID(uint32_t character_id)
{
    LogExpeditionsDetail("Getting expedition id for character [{}]", character_id);

    uint32_t expedition_id = 0;
    auto query = fmt::format(SQL(
                                     SELECT expedition_id FROM expedition_members
                                     WHERE character_id = {} AND is_current_member = TRUE;
                             ), character_id);

    auto results = database.QueryDatabase(query);
    if (results.Success() && results.RowCount() > 0)
    {
        auto row = results.begin();
        expedition_id = strtoul(row[0], nullptr, 10);
    }
    return expedition_id;
}

uint32_t ExpeditionDatabase::GetExpeditionIDFromInstanceID(uint32_t instance_id)
{
    LogExpeditionsDetail("Getting expedition id for instance [{}]", instance_id);

    uint32_t expedition_id = 0;
    auto query = fmt::format(
            "SELECT id FROM expedition_details WHERE instance_id = {};", instance_id
    );

    auto results = database.QueryDatabase(query);
    if (results.Success() && results.RowCount() > 0)
    {
        auto row = results.begin();
        expedition_id = std::strtoul(row[0], nullptr, 10);
    }
    return expedition_id;
}

ExpeditionMember ExpeditionDatabase::GetExpeditionLeader(uint32_t expedition_id)
{
    LogExpeditionsDetail("Getting expedition leader for expedition [{}]", expedition_id);

    auto query = fmt::format(SQL(
                                     SELECT expedition_details.leader_id, character_data.name
            FROM expedition_details
            INNER JOIN character_data ON expedition_details.leader_id = character_data.id
            WHERE expedition_id = {}
                             ), expedition_id);

    ExpeditionMember leader;
    auto results = database.QueryDatabase(query);
    if (results.Success() && results.RowCount() > 0)
    {
        auto row = results.begin();
        leader.char_id = strtoul(row[0], nullptr, 10);
        leader.name    = row[1];
    }
    return leader;
}

uint32_t ExpeditionDatabase::GetExpeditionMemberCount(uint32_t expedition_id)
{
    auto query = fmt::format(SQL(
                                     SELECT COUNT(IF(is_current_member = TRUE, 1, NULL)) member_count
                                     FROM expedition_members
                                     WHERE expedition_id = {};
                             ), expedition_id);

    auto results = database.QueryDatabase(query);

    uint32_t member_count = 0;
    if (results.Success() && results.RowCount() > 0)
    {
        auto row = results.begin();
        member_count = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
    }
    return member_count;
}

void ExpeditionDatabase::InsertCharacterLockouts(
        uint32_t character_id, const std::vector<ExpeditionLockoutTimer>& lockouts,
        bool update_expire_times, bool is_pending)
{
    LogExpeditionsDetail("Inserting character [{}] lockouts", character_id);

    std::string insert_values;
    for (const auto& lockout : lockouts)
    {
        fmt::format_to(std::back_inserter(insert_values),
                       "({}, FROM_UNIXTIME({}), {}, '{}', '{}', {}),",
                       character_id,
                       lockout.GetExpireTime(),
                       lockout.GetDuration(),
                       lockout.GetExpeditionName(),
                       lockout.GetEventName(),
                       is_pending
        );
    }

    if (!insert_values.empty())
    {
        insert_values.pop_back(); // trailing comma

        std::string on_duplicate;
        if (update_expire_times) {
            on_duplicate = "expire_time = VALUES(expire_time)";
        } else {
            on_duplicate = "character_id = VALUES(character_id)";
        }

        auto query = fmt::format(SQL(
                                         INSERT INTO expedition_character_lockouts
                                         (character_id, expire_time, duration, expedition_name, event_name, is_pending)
                                         VALUES {}
                                         ON DUPLICATE KEY UPDATE {};
                                 ), insert_values, on_duplicate);

        database.QueryDatabase(query);
    }
}

void ExpeditionDatabase::InsertMembersLockout(
        const std::vector<ExpeditionMember>& members, const ExpeditionLockoutTimer& lockout)
{
    LogExpeditionsDetail(
            "Inserting members lockout [{}]:[{}] with expire time [{}]",
            lockout.GetExpeditionName(), lockout.GetEventName(), lockout.GetExpireTime()
    );

    std::string insert_values;
    for (const auto& member : members)
    {
        fmt::format_to(std::back_inserter(insert_values),
                       "({}, FROM_UNIXTIME({}), {}, '{}', '{}'),",
                       member.char_id,
                       lockout.GetExpireTime(),
                       lockout.GetDuration(),
                       lockout.GetExpeditionName(),
                       lockout.GetEventName()
        );
    }

    if (!insert_values.empty())
    {
        insert_values.pop_back(); // trailing comma

        auto query = fmt::format(SQL(
                                         INSERT INTO expedition_character_lockouts
                                         (character_id, expire_time, duration, expedition_name, event_name)
                                         VALUES {}
                                         ON DUPLICATE KEY UPDATE expire_time = VALUES(expire_time);
                                 ), insert_values);

        database.QueryDatabase(query);
    }
}

void ExpeditionDatabase::InsertLockout(
        uint32_t expedition_id, const ExpeditionLockoutTimer& lockout)
{
    LogExpeditionsDetail(
            "Inserting expedition [{}] lockout: [{}]:[{}] expire time: [{}]",
            expedition_id, lockout.GetExpeditionName(), lockout.GetEventName(), lockout.GetExpireTime()
    );

    auto query = fmt::format(SQL(
                                     INSERT INTO expedition_lockouts
                                     (expedition_id, event_name, expire_time, duration, is_inherited)
                                     VALUES
                                     ({}, '{}', FROM_UNIXTIME({}), {}, FALSE)
                                     ON DUPLICATE KEY UPDATE expire_time = VALUES(expire_time);
                             ), expedition_id, lockout.GetEventName(), lockout.GetExpireTime(), lockout.GetDuration());

    auto results = database.QueryDatabase(query);
    if (!results.Success())
    {
        LogExpeditions("Failed to insert expedition lockouts");
    }
}

void ExpeditionDatabase::InsertLockouts(
        uint32_t expedition_id, const std::unordered_map<std::string, ExpeditionLockoutTimer>& lockouts)
{
    LogExpeditionsDetail("Inserting expedition [{}] lockouts", expedition_id);

    std::string insert_values;
    for (const auto& lockout : lockouts)
    {
        fmt::format_to(std::back_inserter(insert_values),
                       "({}, '{}', FROM_UNIXTIME({}), {}, {}),",
                       expedition_id,
                       lockout.second.GetEventName(),
                       lockout.second.GetExpireTime(),
                       lockout.second.GetDuration(),
                       lockout.second.IsInherited()
        );
    }

    if (!insert_values.empty())
    {
        insert_values.pop_back(); // trailing comma

        auto query = fmt::format(SQL(
                                         INSERT INTO expedition_lockouts
                                         (expedition_id, event_name, expire_time, duration, is_inherited)
                                         VALUES {}
                                         ON DUPLICATE KEY UPDATE expire_time = VALUES(expire_time);
                                 ), insert_values);

        auto results = database.QueryDatabase(query);
        if (!results.Success())
        {
            LogExpeditions("Failed to insert expedition lockouts");
        }
    }
}

void ExpeditionDatabase::InsertMember(uint32_t expedition_id, uint32_t character_id)
{
    LogExpeditionsDetail("Inserting character [{}] into expedition [{}]", character_id, expedition_id);

    auto query = fmt::format(SQL(
                                     INSERT INTO expedition_members
                                     (expedition_id, character_id, is_current_member)
                                     VALUES
                                     ({}, {}, TRUE)
                                     ON DUPLICATE KEY UPDATE is_current_member = TRUE;
                             ), expedition_id, character_id);

    auto results = database.QueryDatabase(query);
    if (!results.Success())
    {
        LogExpeditions("Failed to insert [{}] to expedition [{}]", character_id, expedition_id);
    }
}

void ExpeditionDatabase::InsertMembers(
        uint32_t expedition_id, const std::vector<ExpeditionMember>& members)
{
    LogExpeditionsDetail("Inserting characters into expedition [{}]", expedition_id);

    std::string insert_values;
    for (const auto& member : members)
    {
        fmt::format_to(std::back_inserter(insert_values),
                       "({}, {}, TRUE),",
                       expedition_id, member.char_id
        );
    }

    if (!insert_values.empty())
    {
        insert_values.pop_back(); // trailing comma

        auto query = fmt::format(SQL(
                                         INSERT INTO expedition_members (expedition_id, character_id, is_current_member)
                                         VALUES {};
                                 ), insert_values);

        auto results = database.QueryDatabase(query);
        if (!results.Success())
        {
            LogExpeditions("Failed to save expedition members to database");
        }
    }
}

void ExpeditionDatabase::UpdateLeaderID(uint32_t expedition_id, uint32_t leader_id)
{
    LogExpeditionsDetail("Updating leader [{}] for expedition [{}]", leader_id, expedition_id);

    auto query = fmt::format(SQL(
                                     UPDATE expedition_details SET leader_id = {} WHERE id = {}
                             ), leader_id, expedition_id);

    auto results = database.QueryDatabase(query);
    if (!results.Success())
    {
        LogExpeditions("Failed to update expedition [{}] leader", expedition_id);
    }
}

void ExpeditionDatabase::UpdateMemberRemoved(uint32_t expedition_id, uint32_t character_id)
{
    LogExpeditionsDetail("Removing member [{}] from expedition [{}]", character_id, expedition_id);

    auto query = fmt::format(SQL(
                                     UPDATE expedition_members SET is_current_member = FALSE
                                     WHERE expedition_id = {} AND character_id = {};
                             ), expedition_id, character_id);

    auto results = database.QueryDatabase(query);
    if (!results.Success())
    {
        LogExpeditions("Failed to remove [{}] from expedition [{}]", character_id, expedition_id);
    }
}