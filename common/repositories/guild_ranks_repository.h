#ifndef EQEMU_GUILD_RANKS_REPOSITORY_H
#define EQEMU_GUILD_RANKS_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_guild_ranks_repository.h"

class GuildRanksRepository: public BaseGuildRanksRepository {
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
     * GuildRanksRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * GuildRanksRepository::GetWhereNeverExpires()
     * GuildRanksRepository::GetWhereXAndY()
     * GuildRanksRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here

	static int ReplaceOne(
		Database& db,
		const GuildRanks& e
	)
	{
		std::vector<std::string> v;

		std::vector<std::string> columns = { "guild_id", "rank", "title"};

		v.push_back(std::to_string(e.guild_id));
		v.push_back(std::to_string(e.rank));
		v.push_back("'" + Strings::Escape(e.title) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"REPLACE INTO {} ({}) VALUES({})",
				TableName(),
				Strings::Implode(", ", columns),
				Strings::Implode(", ", v)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int ReplaceMany(
		Database& db,
		const std::vector<GuildRanks>& entries
	)
	{
		std::vector<std::string> insert_chunks;
		std::vector<std::string> columns = { "guild_id", "rank", "title" };

		for (auto& e : entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.guild_id));
			v.push_back(std::to_string(e.rank));
			v.push_back("'" + Strings::Escape(e.title) + "'");

			insert_chunks.push_back("(" + Strings::Implode(",", v) + ")");
		}

		std::vector<std::string> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"REPLACE INTO {} ({}) VALUES{}",
				TableName(),
				Strings::Implode(", ", columns),
				Strings::Implode(", ", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_GUILD_RANKS_REPOSITORY_H
