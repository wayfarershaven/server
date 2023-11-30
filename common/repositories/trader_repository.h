#ifndef EQEMU_TRADER_REPOSITORY_H
#define EQEMU_TRADER_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_trader_repository.h"

#include "../eq_packet_structs.h"

class TraderRepository: public BaseTraderRepository {
public:
	struct BazaarDBSearchResults_Struct {
		uint32	count;
		uint32	trader_id;
		uint32	item_id;
		uint32	serial_number;
		uint32	charges;
		uint32	cost;
		uint32	slot_id;
		uint32	icon_id;
		uint32	sum_charges;
		uint32	trader_zone_id;
		uint32	trader_entity_id;
		bool	stackable;
		std::string	item_name;
		std::string serial_number_RoF;
	};
	struct DistinctTraders_Struct {
		uint32	trader_char_id;
		uint32	trader_zone_id;
		uint32	trader_entity_id;
		std::string	trader_name;
	};
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
     * TraderRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * TraderRepository::GetWhereNeverExpires()
     * TraderRepository::GetWhereXAndY()
     * TraderRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here

	static std::vector<BazaarDBSearchResults_Struct> GetBazaarSearchResults(Database& db, BazaarSearch_Struct search, uint32 char_zone_id)
	{
		std::string search_values = " COUNT(item_id), trader.char_id, trader.item_id, trader.serialnumber, trader.charges, trader.item_cost, trader.slot_id, items.name ";
		std::string search_criteria = " WHERE trader.item_id = items.id ";

		if (!search.search_scope) {
			search_criteria.append(fmt::format(" AND character_data.zone_id = {}", char_zone_id));
		}
		else if (search.trader_id > 0) {
			search_criteria.append(fmt::format(" AND trader.entity_id = {}", search.trader_id));
		}

		if (search.min_level != 1) {
			search_criteria.append(fmt::format(" AND items.reclevel >= {}", search.min_level));
		}

		if (search.max_level != RuleI(Character, MaxLevel)) {
			search_criteria.append(fmt::format(" AND items.reclevel <= {}", search.max_level));
		}

		if (search.min_cost != 0) {
			search_criteria.append(StringFormat(" AND trader.item_cost >= %i", search.min_cost));
		}

		if (search.max_cost != 0) {
			search_criteria.append(StringFormat(" AND trader.item_cost <= %i", search.max_cost));
		}

		if (!std::string(search.name).empty()) {
			search_criteria.append(fmt::format(" AND items.name LIKE \"\%{}\%\" ", search.name));
		}

		if (search._class != 0xFFFFFFFF) {
			search_criteria.append(StringFormat(" AND MID(REVERSE(BIN(items.classes)), %i, 1) = 1", search._class));
		}

		if (search.race != 0xFFFFFFFF) {
			search_criteria.append(StringFormat(" AND MID(REVERSE(BIN(items.races)), %i, 1) = 1", search.race));
		}

		if (search.slot != 0xFFFFFFFF) {
			search_criteria.append(StringFormat(" AND MID(REVERSE(BIN(items.slots)), %i, 1) = 1", search.slot + 1));
		}

		// not yet implemented
		//	if (search.prestige != 0) {
		// 	   0xffffffff prestige only, 0xfffffffe non-prestige, 0 all
		//		search_criteria.append(fmt::format(" AND items.type = {} ", search.prestige));
		//	}

		if (search.augment != 0) {
			search_criteria.append(fmt::format(" AND MID(REVERSE(BIN(items.augtype)), {}, 1) = 1", search.augment));
		}

		switch (search.type) {
		case 0xFFFFFFFF:
			break;
		case 0:
			// 1H Slashing
			search_criteria.append(" AND items.itemtype = 0 AND damage > 0");
			break;
		case 31:
			search_criteria.append(" AND items.itemclass = 2");
			break;
		case 46:
			search_criteria.append(" AND items.scrolleffect > 0 AND items.scrolleffect < 65000");
			break;
		case 47:
			search_criteria.append(" AND items.worneffect = 998");
			break;
		case 48:
			search_criteria.append(" AND items.worneffect >= 1298 AND items.worneffect <= 1307");
			break;
		case 49:
			search_criteria.append(" AND items.focuseffect > 0");
			break;

		default:
			search_criteria.append(StringFormat(" AND items.itemtype = %i", search.type));
		}

		switch (search.item_stat) {

		case STAT_AC:
			search_criteria.append(" AND items.ac > 0");
			search_values.append(", items.ac");
			break;

		case STAT_AGI:
			search_criteria.append(" AND items.aagi > 0");
			search_values.append(", items.aagi");
			break;

		case STAT_CHA:
			search_criteria.append(" AND items.acha > 0");
			search_values.append(", items.acha");
			break;

		case STAT_DEX:
			search_criteria.append(" AND items.adex > 0");
			search_values.append(", items.adex");
			break;

		case STAT_INT:
			search_criteria.append(" AND items.aint > 0");
			search_values.append(", items.aint");
			break;

		case STAT_STA:
			search_criteria.append(" AND items.asta > 0");
			search_values.append(", items.asta");
			break;

		case STAT_STR:
			search_criteria.append(" AND items.astr > 0");
			search_values.append(", items.astr");
			break;

		case STAT_WIS:
			search_criteria.append(" AND items.awis > 0");
			search_values.append(", items.awis");
			break;

		case STAT_COLD:
			search_criteria.append(" AND items.cr > 0");
			search_values.append(", items.cr");
			break;

		case STAT_DISEASE:
			search_criteria.append(" AND items.dr > 0");
			search_values.append(", items.dr");
			break;

		case STAT_FIRE:
			search_criteria.append(" AND items.fr > 0");
			search_values.append(", items.fr");
			break;

		case STAT_MAGIC:
			search_criteria.append(" AND items.mr > 0");
			search_values.append(", items.mr");
			break;

		case STAT_POISON:
			search_criteria.append(" AND items.pr > 0");
			search_values.append(", items.pr");
			break;

		case STAT_HP:
			search_criteria.append(" AND items.hp > 0");
			search_values.append(", items.hp");
			break;

		case STAT_MANA:
			search_criteria.append(" AND items.mana > 0");
			search_values.append(", items.mana");
			break;

		case STAT_ENDURANCE:
			search_criteria.append(" AND items.endur > 0");
			search_values.append(", items.endur");
			break;

		case STAT_ATTACK:
			search_criteria.append(" AND items.attack > 0");
			search_values.append(", items.attack");
			break;

		case STAT_HP_REGEN:
			search_criteria.append(" AND items.regen > 0");
			search_values.append(", items.regen");
			break;

		case STAT_MANA_REGEN:
			search_criteria.append(" AND items.manaregen > 0");
			search_values.append(", items.manaregen");
			break;

		case STAT_HASTE:
			search_criteria.append(" AND items.haste > 0");
			search_values.append(", items.haste");
			break;

		case STAT_DAMAGE_SHIELD:
			search_criteria.append(" AND items.damageshield > 0");
			search_values.append(", items.damageshield");
			break;

		default:
			search_values.append(", 0");
			break;
		}

		std::string query = fmt::format("SELECT {}, SUM(charges), items.stackable, items.icon, character_data.zone_id, trader.entity_id "
			"FROM trader, items, character_data {} AND trader.char_id = character_data.id GROUP BY items.id, charges, char_id LIMIT {}",
			search_values.c_str(),
			search_criteria.c_str(),
			search.max_results
		);

		std::vector<BazaarDBSearchResults_Struct> all_entries;
		auto results = db.QueryDatabase(query);

		if (!results.Success()) {
			return all_entries;
		}
				
		all_entries.reserve(results.RowCount());

		for (auto row : results) {
			BazaarDBSearchResults_Struct data{};

			data.charges			= Strings::ToInt(row[4]);
			data.cost				= Strings::ToInt(row[5]);
			data.count				= Strings::ToInt(row[0]);
			data.icon_id			= Strings::ToInt(row[11]);
			data.item_id			= Strings::ToInt(row[2]);
			data.serial_number		= Strings::ToInt(row[3]);
			data.slot_id			= Strings::ToInt(row[6]);
			data.stackable			= Strings::ToBool(row[10]);
			data.sum_charges		= Strings::ToInt(row[9]);
			data.trader_entity_id	= Strings::ToInt(row[13]);
			data.trader_id			= Strings::ToInt(row[1]);
			data.trader_zone_id		= Strings::ToInt(row[12]);
			data.serial_number_RoF	= fmt::format("{:016}\0", Strings::ToInt(row[2]));
			data.item_name          = fmt::format("{:.63}\0", std::string(row[7]).c_str());
			all_entries.push_back(data);
		}

		return all_entries;
	}

	static std::vector<DistinctTraders_Struct> GetDistinctTraders(Database& db)
	{
		std::vector<DistinctTraders_Struct> all_entries;

		auto results = db.QueryDatabase(
			"SELECT DISTINCT(t.char_id), c.zone_id, c.name, t.entity_id FROM trader AS t, character_data AS c WHERE t.char_id = c.id;"
		);

		all_entries.reserve(results.RowCount());

		for (auto row : results) {
			DistinctTraders_Struct e{};

			e.trader_char_id	= Strings::ToInt(row[0]);
			e.trader_zone_id	= Strings::ToInt(row[1]);
			e.trader_entity_id	= Strings::ToInt(row[3]);
			e.trader_name		= row[2] ? row[2] : "";
	
			all_entries.push_back(e);
		}

		return all_entries;
	}
};

#endif //EQEMU_TRADER_REPOSITORY_H
