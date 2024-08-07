#ifndef EQEMU_CHARACTER_DATA_REPOSITORY_H
#define EQEMU_CHARACTER_DATA_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_character_data_repository.h"



class CharacterDataRepository: public BaseCharacterDataRepository {
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
     * CharacterDataRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * CharacterDataRepository::GetWhereNeverExpires()
     * CharacterDataRepository::GetWhereXAndY()
     * CharacterDataRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here
	static uint32 GetSecondsSinceLastLogin(Database &db, const std::string& name)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT (UNIX_TIMESTAMP(NOW()) - last_login) FROM {} WHERE name = '{}'",
				TableName(),
				Strings::Escape(name)
			)
		);

		if (!results.RowCount() || !results.Success()) {
			return 0;
		}

		auto row = results.begin();

		return Strings::ToUnsignedInt(row[0]);
	}

	static CharacterData FindByName(
		Database& db,
		const std::string& character_name
	)
	{
		auto l = CharacterDataRepository::GetWhere(
			db,
			fmt::format(
				"`name` = '{}' LIMIT 1",
				Strings::Escape(character_name)
			)
		);

		return l.empty() ? CharacterDataRepository::NewEntity() : l.front();
	}
};

#endif //EQEMU_CHARACTER_DATA_REPOSITORY_H
