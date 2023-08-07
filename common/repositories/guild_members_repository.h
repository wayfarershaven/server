#ifndef EQEMU_GUILD_MEMBERS_REPOSITORY_H
#define EQEMU_GUILD_MEMBERS_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_guild_members_repository.h"

class GuildMembersRepository: public BaseGuildMembersRepository {
public:

    /**
     * This file was auto generated and can be modified and extended upon
     *
     * Base repository methods are automatically
     * generated in the "base" version of this repository. The base repository
     * is immutable and to be left untouched, while methods in this class
     * are used as extension methods for more specific persistence-layer
     * accessors or mutators.
     *
     * Base Methods (Subject to be expanded upon in time)
     *
     * Note: Not all tables are designed appropriately to fit functionality with all base methods
     *
     * InsertOne
     * UpdateOne
     * DeleteOne
     * FindOne
     * GetWhere(std::string where_filter)
     * DeleteWhere(std::string where_filter)
     * InsertMany
     * All
     *
     * Example custom methods in a repository
     *
     * GuildMembersRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * GuildMembersRepository::GetWhereNeverExpires()
     * GuildMembersRepository::GetWhereXAndY()
     * GuildMembersRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here

	static int UpdateMemberRank(
		Database& db,
		uint32	char_id,
		uint32	rank_id
	) {
		auto results = db.QueryDatabase(
			fmt::format("UPDATE `{}` SET `rank`= '{}' WHERE `char_id` = '{}';",
				TableName(),
				rank_id,
				char_id
			)
		);

		return results.Success() ? results.RowsAffected() : 0;
	}

	static int ReplaceOne(
		Database& db,
		const GuildMembers& e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(std::to_string(e.char_id));
		v.push_back(std::to_string(e.guild_id));
		v.push_back(std::to_string(e.rank));
		v.push_back(std::to_string(e.tribute_enable));
		v.push_back(std::to_string(e.total_tribute));
		v.push_back(std::to_string(e.last_tribute));
		v.push_back(std::to_string(e.banker));
		v.push_back("'" + Strings::Escape(e.public_note) + "'");
		v.push_back(std::to_string(e.alt));

		auto results = db.QueryDatabase(
			fmt::format(
				"REPLACE INTO {} ({}) VALUES({})",
				TableName(),
				ColumnsRaw(),
				Strings::Implode(", ", v)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}
};

#endif //EQEMU_GUILD_MEMBERS_REPOSITORY_H
