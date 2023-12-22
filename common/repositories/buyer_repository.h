#ifndef EQEMU_BUYER_REPOSITORY_H
#define EQEMU_BUYER_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_buyer_repository.h"

#include "../eq_packet_structs.h"

class BuyerRepository: public BaseBuyerRepository {
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
     * BuyerRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * BuyerRepository::GetWhereNeverExpires()
     * BuyerRepository::GetWhereXAndY()
     * BuyerRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here

	static bool UpdateBuyLine(Database& db, const BuyerLineItems_Struct b, uint32 char_id)
	{
			std::string query = fmt::format("REPLACE INTO buyer VALUES({}, {}, {}, \"{}\", {}, {})",
				char_id, 
				b.slot, 
				b.item_id, 
				b.item_name, 
				b.item_quantity, 
				b.item_cost
			);
			auto results = db.QueryDatabase(query);
			if (!results.Success()) {
				return false;
			}
			for (auto const& i : b.trade_items) {
				if (i.item_id) {
					std::string query = fmt::format("REPLACE INTO `buyer_trade_items` (`char_id`, `slot_id`, `item_id`, `item_qty`, `Item_icon`, `item_name`) "
						"VALUES({}, {}, {}, {}, {}, \"{}\")",
						char_id,
						b.slot,
						i.item_id,
						i.item_quantity,
						i.item_icon,
						i.item_name
					);
					auto results = db.QueryDatabase(query);
					if (!results.Success()) {
						return false;
					}
				}
			}
		return true;
	}

	static bool ClearBuyerTables(Database& db)
	{
		std::string query = fmt::format("DELETE FROM buyer");
		auto results = db.QueryDatabase(query);
		if (!results.Success()) {
			return false;
		}

		query = fmt::format("DELETE FROM buyer_trade_items");
		results = db.QueryDatabase(query);
		if (!results.Success()) {
			return false;
		}
		return true;
	}

	static bool DeleteBuyLine(Database& db, uint32 char_id, int32 slot_id = 0xffffffff)
	{
		if (slot_id == 0xffffffff) {
			std::string query = fmt::format("DELETE FROM buyer WHERE charid = {}",
				char_id
			);
			auto results = db.QueryDatabase(query);
			if (!results.Success()) {
				return false;
			}

			query = fmt::format("DELETE FROM buyer_trade_items WHERE char_id = {}",
				char_id
			);
			results = db.QueryDatabase(query);
			if (!results.Success()) {
				return false;
			}
			return true;
		}
		else {
			std::string query = fmt::format("DELETE FROM buyer WHERE charid = {} AND buyslot = {}",
				char_id,
				slot_id
			);
			auto results = db.QueryDatabase(query);
			if (!results.Success()) {
				return false;
			}

			query = fmt::format("DELETE FROM buyer_trade_items WHERE char_id = {} AND slot_id = {}",
				char_id,
				slot_id
			);
			results = db.QueryDatabase(query);
			if (!results.Success()) {
				return false;
			}
			return true;
		}
	}

	static BuyerLine_Struct GetBuyLine(Database& db, uint32 char_id)
	{
		BuyerLine_Struct bl{};

		int string_size = 0;
		std::string query = fmt::format("SELECT `buyslot`, `itemid`, `itemname`, `quantity`, `price` "
			"FROM buyer WHERE buyer.charid = {};",
			char_id
		);
		auto results = db.QueryDatabase(query);
		if (!results.Success()) {
			return bl;
		}

		for (auto r : results) {
			BuyerLineItems_Struct b{};
			b.item_toggle   = 1;
			b.slot          = Strings::ToInt(r[0]);
			b.item_id       = Strings::ToInt(r[1]);
			b.item_quantity = Strings::ToInt(r[3]);
			b.item_cost     = Strings::ToInt(r[4]);
			strn0cpy(b.item_name, r[2] ? r[2] : "", sizeof(b.item_name));
			string_size += strlen(b.item_name);

			query = fmt::format("SELECT `item_id`, `item_qty`, `item_icon`, `item_name` "
				"FROM buyer_trade_items "
				"WHERE buyer_trade_items.char_id = {} AND slot_id = {};",
				char_id,
				b.slot
			);
			auto results_details = db.QueryDatabase(query);
			if (results_details.RowCount() > 0) {
				int i = 0;
				for (auto d : results_details) {
					b.trade_items[i].item_id = Strings::ToInt(d[0]);
					b.trade_items[i].item_quantity = Strings::ToInt(d[1]);
					b.trade_items[i].item_icon = Strings::ToInt(d[2]);
					strn0cpy(b.trade_items[i].item_name, d[3] ? d[3] : "", sizeof(b.trade_items[i].item_name));
					string_size += strlen(b.trade_items[i].item_name);
					i++;
				}
			}
			bl.buy_line.push_back(b);
		}
		bl.no_items = string_size;
		return bl;
	}

	static BuyerLineItems_Struct GetOneBuyLine(Database& db, uint32 buyer_id, uint32 slot_id)
	{
		BuyerLineItems_Struct b{};

		int string_size = 0;
		std::string query = fmt::format("SELECT `buyslot`, `itemid`, `itemname`, `quantity`, `price` "
			"FROM buyer WHERE buyer.charid = {} AND buyer.buyslot = {};",
			buyer_id,
			slot_id
		);
		auto results = db.QueryDatabase(query);
		if (!results.Success()) {
			return b;
		}

		for (auto r : results) {
			b.item_toggle = 1;
			b.slot = Strings::ToInt(r[0]);
			b.item_id = Strings::ToInt(r[1]);
			b.item_quantity = Strings::ToInt(r[3]);
			b.item_cost = Strings::ToInt(r[4]);
			strn0cpy(b.item_name, r[2] ? r[2] : "", sizeof(b.item_name));

			query = fmt::format("SELECT `item_id`, `item_qty`, `item_icon`, `item_name` "
				"FROM buyer_trade_items "
				"WHERE buyer_trade_items.char_id = {} AND slot_id = {};",
				buyer_id,
				b.slot
			);
			auto results_details = db.QueryDatabase(query);
			if (results_details.RowCount() > 0) {
				int i = 0;
				for (auto d : results_details) {
					b.trade_items[i].item_id = Strings::ToInt(d[0]);
					b.trade_items[i].item_quantity = Strings::ToInt(d[1]);
					b.trade_items[i].item_icon = Strings::ToInt(d[2]);
					strn0cpy(b.trade_items[i].item_name, d[3] ? d[3] : "", sizeof(b.trade_items[i].item_name));
					i++;
				}
			}
		}
		return b;
	}
	
	static BuyerLineSearch_Struct SearchBuyLines(Database& db, std::string& search_string)
	{
		BuyerLineSearch_Struct bl{};

		int string_size = 0;
		std::string query = fmt::format("SELECT `buyslot`, `itemid`, `itemname`, `quantity`, `price`, `charid` "
			"FROM buyer WHERE buyer.itemname LIKE '%{}%';",
			search_string.c_str()
		);
		auto results = db.QueryDatabase(query);
		if (!results.Success()) {
			return bl;
		}
		bl.no_items = results.RowCount();

		for (auto r : results) {
			BuyerLineItemsSearch_Struct b{};
			b.item_toggle = 1;
			b.slot = Strings::ToInt(r[0]);
			b.item_id = Strings::ToInt(r[1]);
			b.item_quantity = Strings::ToInt(r[3]);
			b.item_cost = Strings::ToInt(r[4]);
			b.buyer_id = Strings::ToInt(r[5]);
			strn0cpy(b.item_name, r[2] ? r[2] : "", sizeof(b.item_name));
			string_size += strlen(b.item_name);

			query = fmt::format("SELECT `item_id`, `item_qty`, `item_icon`, `item_name` "
				"FROM buyer_trade_items "
				"WHERE buyer_trade_items.char_id = {} AND slot_id = {};",
				b.buyer_id,
				b.slot
			);
			auto results_details = db.QueryDatabase(query);
			if (results_details.RowCount() > 0) {
				int i = 0;
				for (auto d : results_details) {
					b.trade_items[i].item_id = Strings::ToInt(d[0]);
					b.trade_items[i].item_quantity = Strings::ToInt(d[1]);
					b.trade_items[i].item_icon = Strings::ToInt(d[2]);
					strn0cpy(b.trade_items[i].item_name, d[3] ? d[3] : "", sizeof(b.trade_items[i].item_name));
					string_size += strlen(b.trade_items[i].item_name);
					i++;
				}
			}
			bl.buy_line.push_back(b);
		}
		return bl;
	}
};

#endif //EQEMU_BUYER_REPOSITORY_H
