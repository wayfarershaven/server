#ifndef EQEMU_BUYER_BUY_LINES_REPOSITORY_H
#define EQEMU_BUYER_BUY_LINES_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_buyer_buy_lines_repository.h"
#include "buyer_trade_items_repository.h"
#include "character_data_repository.h"
#include "buyer_repository.h"

#include "../eq_packet_structs.h"

class BuyerBuyLinesRepository: public BaseBuyerBuyLinesRepository {
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
	 * BuyerBuyLinesRepository::GetByZoneAndVersion(int zone_id, int zone_version)
	 * BuyerBuyLinesRepository::GetWhereNeverExpires()
	 * BuyerBuyLinesRepository::GetWhereXAndY()
	 * BuyerBuyLinesRepository::DeleteWhereXAndY()
	 *
	 * Most of the above could be covered by base methods, but if you as a developer
	 * find yourself re-using logic for other parts of the code, its best to just make a
	 * method that can be re-used easily elsewhere especially if it can use a base repository
	 * method and encapsulate filters there
	 */

	// Custom extended repository methods here

	struct WelcomeData_Struct {
		uint32 count_of_buyers;
		uint32 count_of_items;
	};

	static int UpdateBuyLine(Database& db, const BuyerLineItems_Struct b, uint32 char_id)
	{
		auto buyer = BuyerRepository::GetWhere(db, fmt::format("`char_id` = '{}' LIMIT 1", char_id));
		if (buyer.empty()){
			return 0;
		}

		BuyerBuyLinesRepository::BuyerBuyLines buy_lines{};
		buy_lines.id            = 0;
		buy_lines.buyer_id      = buyer.front().id;
		buy_lines.char_id       = char_id;
		buy_lines.buy_slot_id   = b.slot;
		buy_lines.item_id       = b.item_id;
		buy_lines.item_name     = b.item_name;
		buy_lines.item_quantity = b.item_quantity;
		buy_lines.item_price    = b.item_cost;
		auto e = InsertOne(db, buy_lines);

		std::vector<BuyerTradeItemsRepository::BuyerTradeItems> queue;

		for (auto const &r: b.trade_items) {
			BuyerTradeItemsRepository::BuyerTradeItems bti{};
			bti.id                 = 0;
			bti.buyer_buy_lines_id = e.id;
			bti.item_id            = r.item_id;
			bti.item_qty           = r.item_quantity;
			bti.item_icon          = r.item_icon;
			bti.item_name          = r.item_name;

			bti.item_id ? queue.push_back(bti) : __nop() ;
		}

		if (!queue.empty()) {
			BuyerTradeItemsRepository::InsertMany(db, queue);
		}

		return e.id;
	}

	static int DeleteBuyLine(Database &db, uint32 char_id, int32 slot_id = 0xffffffff)
	{
		if (slot_id == 0xffffffff) {
			return DeleteWhere(db, fmt::format("`char_id` = '{}'", char_id));
		}
		else {
			return DeleteWhere(
				db,
				fmt::format(
					"`char_id` = '{}' AND `buy_slot_id` = '{}'",
					char_id,
					slot_id
				)
			);
		}
	}

	static std::vector<BuyerLineItems_Struct> GetBuyLines(Database &db, uint32 char_id)
	{
		std::vector<BuyerLineItems_Struct> all_entries{};

		auto  buy_line             = GetWhere(db, fmt::format("`char_id` = '{}';", char_id));
		auto  buy_line_trade_items = BuyerTradeItemsRepository::GetWhere(
			db,
			fmt::format(
				"`buyer_buy_lines_id` IN (SELECT b.id FROM buyer_buy_lines AS b WHERE b.char_id = '{}')",
				char_id
			)
		);

		for (auto const &l: buy_line) {
			BuyerLineItems_Struct bli{};
			bli.item_id       = l.item_id;
			bli.item_cost     = l.item_price;
			bli.item_quantity = l.item_quantity;
			strn0cpy(bli.item_name, l.item_name.c_str(), sizeof(bli.item_name));

			uint32 slot = 0;

			for (auto const &i: GetSubIDs(buy_line_trade_items, l.id)) {
				bli.trade_items[slot].item_id       = buy_line_trade_items.at(i).item_id;
				bli.trade_items[slot].item_icon     = buy_line_trade_items.at(i).item_icon;
				bli.trade_items[slot].item_quantity = buy_line_trade_items.at(i).item_qty;
				bli.trade_items[slot].item_id       = buy_line_trade_items.at(i).item_id;
				strn0cpy(
					bli.trade_items[slot].item_name,
					buy_line_trade_items.at(i).item_name.c_str(),
					sizeof(bli.trade_items[slot].item_name)
				);
				slot++;
			}
			all_entries.push_back(bli);
		}

		return all_entries;
	}

	static BuyerLineSearch_Struct SearchBuyLines(Database& db, std::string& search_string)
	{
		BuyerLineSearch_Struct all_entries{};

		auto buy_line = GetWhere(db, fmt::format("`item_name` LIKE \"%{}%\";", search_string));

		std::vector<std::string> ids{};
		std::vector<std::string> char_ids{};
		for (auto const &bl : buy_line) {
			ids.push_back(std::to_string(bl.id));
			char_ids.push_back((std::to_string(bl.char_id)));
		}

		auto buy_line_trade_items = BuyerTradeItemsRepository::GetWhere(
			db,
			fmt::format(
				"`buyer_buy_lines_id` IN ({});",
				Strings::Implode(", ", ids)
			)
		);

		auto char_names = CharacterDataRepository::GetWhere(
			db,
			fmt::format(
				"`id` IN ({});",
				Strings::Implode(", ", char_ids)
			)
		);

		all_entries.no_items = buy_line.size();
		for (auto const &l: buy_line) {
			BuyerLineItemsSearch_Struct blis{};
			blis.item_id       = l.item_id;
			blis.item_cost     = l.item_price;
			blis.item_quantity = l.item_quantity;
			blis.buyer_id      = l.char_id;
			auto it = std::find_if(
				char_names.cbegin(),
				char_names.cend(),
				[&](BaseCharacterDataRepository::CharacterData e) { return e.id = l.char_id; }
			);
			blis.buyer_name = it != char_names.end() ? it->name : std::string("");
			strn0cpy(blis.item_name, l.item_name.c_str(), sizeof(blis.item_name));

			uint32 slot = 0;
			for (auto const &i: GetSubIDs(buy_line_trade_items, l.id)) {
				blis.trade_items[slot].item_id       = buy_line_trade_items.at(i).item_id;
				blis.trade_items[slot].item_icon     = buy_line_trade_items.at(i).item_icon;
				blis.trade_items[slot].item_quantity = buy_line_trade_items.at(i).item_qty;
				blis.trade_items[slot].item_id       = buy_line_trade_items.at(i).item_id;
				strn0cpy(
					blis.trade_items[slot].item_name,
					buy_line_trade_items.at(i).item_name.c_str(),
					sizeof(blis.trade_items[slot].item_name)
				);
				slot++;
			}
			all_entries.buy_line.push_back(blis);
		}

		return all_entries;
	}

	static std::vector<uint64> GetSubIDs(std::vector<BuyerTradeItemsRepository::BuyerTradeItems> &in, uint64 id) {
		std::vector<uint64> indices{};
		auto                it = in.begin();
		while ((it = std::find_if(
			it,
			in.end(),
			[&](BuyerTradeItemsRepository::BuyerTradeItems const &e)
			{
				return e.buyer_buy_lines_id == id;
			}
		))
			   != in.end()
			) {
			indices.push_back(std::distance(in.begin(), it));
			it++;
		}
		return indices;
	}

	static WelcomeData_Struct GetWelcomeData(Database &db)
	{
		WelcomeData_Struct e{};

		auto results = db.QueryDatabase("SELECT COUNT(DISTINCT char_id), COUNT(char_id) FROM buyer;");

		if (!results.RowCount()) {
			return e;
		}

		auto r = results.begin();
		e.count_of_buyers = Strings::ToInt(r[0]);
		e.count_of_items  = Strings::ToInt(r[1]);
		return e;
	}

};

#endif //EQEMU_BUYER_BUY_LINES_REPOSITORY_H
