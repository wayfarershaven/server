/**
 * DO NOT MODIFY THIS FILE
 *
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to the repository extending the base.
 * Any modifications to base repositories are to be made by the generator only
 *
 * @generator ./utils/scripts/generators/repository-generator.pl
 * @docs https://eqemu.gitbook.io/server/in-development/developer-area/repositories
 */

#ifndef EQEMU_BASE_FACTION_LIST_REPOSITORY_H
#define EQEMU_BASE_FACTION_LIST_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>


class BaseFactionListRepository {
public:
	struct FactionList {
		int32_t     id;
		std::string name;
		int16_t     base;
		int16_t     mod_c1;
		int16_t     mod_c2;
		int16_t     mod_c3;
		int16_t     mod_c4;
		int16_t     mod_c5;
		int16_t     mod_c6;
		int16_t     mod_c7;
		int16_t     mod_c8;
		int16_t     mod_c9;
		int16_t     mod_c10;
		int16_t     mod_c11;
		int16_t     mod_c12;
		int16_t     mod_c13;
		int16_t     mod_c14;
		int16_t     mod_c15;
		int16_t     mod_c16;
		int16_t     mod_r1;
		int16_t     mod_r2;
		int16_t     mod_r3;
		int16_t     mod_r4;
		int16_t     mod_r5;
		int16_t     mod_r6;
		int16_t     mod_r7;
		int16_t     mod_r8;
		int16_t     mod_r9;
		int16_t     mod_r10;
		int16_t     mod_r11;
		int16_t     mod_r12;
		int16_t     mod_r14;
		int16_t     mod_r42;
		int16_t     mod_r75;
		int16_t     mod_r108;
		int16_t     mod_r128;
		int16_t     mod_r130;
		int16_t     mod_r161;
		int16_t     mod_r330;
		int16_t     mod_r367;
		int16_t     mod_r522;
		int16_t     mod_d140;
		int16_t     mod_d201;
		int16_t     mod_d202;
		int16_t     mod_d203;
		int16_t     mod_d204;
		int16_t     mod_d205;
		int16_t     mod_d206;
		int16_t     mod_d207;
		int16_t     mod_d208;
		int16_t     mod_d209;
		int16_t     mod_d210;
		int16_t     mod_d211;
		int16_t     mod_d212;
		int16_t     mod_d213;
		int16_t     mod_d214;
		int16_t     mod_d215;
		int16_t     mod_d216;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"name",
			"base",
			"mod_c1",
			"mod_c2",
			"mod_c3",
			"mod_c4",
			"mod_c5",
			"mod_c6",
			"mod_c7",
			"mod_c8",
			"mod_c9",
			"mod_c10",
			"mod_c11",
			"mod_c12",
			"mod_c13",
			"mod_c14",
			"mod_c15",
			"mod_c16",
			"mod_r1",
			"mod_r2",
			"mod_r3",
			"mod_r4",
			"mod_r5",
			"mod_r6",
			"mod_r7",
			"mod_r8",
			"mod_r9",
			"mod_r10",
			"mod_r11",
			"mod_r12",
			"mod_r14",
			"mod_r42",
			"mod_r75",
			"mod_r108",
			"mod_r128",
			"mod_r130",
			"mod_r161",
			"mod_r330",
			"mod_r367",
			"mod_r522",
			"mod_d140",
			"mod_d201",
			"mod_d202",
			"mod_d203",
			"mod_d204",
			"mod_d205",
			"mod_d206",
			"mod_d207",
			"mod_d208",
			"mod_d209",
			"mod_d210",
			"mod_d211",
			"mod_d212",
			"mod_d213",
			"mod_d214",
			"mod_d215",
			"mod_d216",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"name",
			"base",
			"mod_c1",
			"mod_c2",
			"mod_c3",
			"mod_c4",
			"mod_c5",
			"mod_c6",
			"mod_c7",
			"mod_c8",
			"mod_c9",
			"mod_c10",
			"mod_c11",
			"mod_c12",
			"mod_c13",
			"mod_c14",
			"mod_c15",
			"mod_c16",
			"mod_r1",
			"mod_r2",
			"mod_r3",
			"mod_r4",
			"mod_r5",
			"mod_r6",
			"mod_r7",
			"mod_r8",
			"mod_r9",
			"mod_r10",
			"mod_r11",
			"mod_r12",
			"mod_r14",
			"mod_r42",
			"mod_r75",
			"mod_r108",
			"mod_r128",
			"mod_r130",
			"mod_r161",
			"mod_r330",
			"mod_r367",
			"mod_r522",
			"mod_d140",
			"mod_d201",
			"mod_d202",
			"mod_d203",
			"mod_d204",
			"mod_d205",
			"mod_d206",
			"mod_d207",
			"mod_d208",
			"mod_d209",
			"mod_d210",
			"mod_d211",
			"mod_d212",
			"mod_d213",
			"mod_d214",
			"mod_d215",
			"mod_d216",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(Strings::Implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(Strings::Implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("faction_list");
	}

	static std::string BaseSelect()
	{
		return fmt::format(
			"SELECT {} FROM {}",
			SelectColumnsRaw(),
			TableName()
		);
	}

	static std::string BaseInsert()
	{
		return fmt::format(
			"INSERT INTO {} ({}) ",
			TableName(),
			ColumnsRaw()
		);
	}

	static FactionList NewEntity()
	{
		FactionList e{};

		e.id       = 0;
		e.name     = "";
		e.base     = 0;
		e.mod_c1   = 0;
		e.mod_c2   = 0;
		e.mod_c3   = 0;
		e.mod_c4   = 0;
		e.mod_c5   = 0;
		e.mod_c6   = 0;
		e.mod_c7   = 0;
		e.mod_c8   = 0;
		e.mod_c9   = 0;
		e.mod_c10  = 0;
		e.mod_c11  = 0;
		e.mod_c12  = 0;
		e.mod_c13  = 0;
		e.mod_c14  = 0;
		e.mod_c15  = 0;
		e.mod_c16  = 0;
		e.mod_r1   = 0;
		e.mod_r2   = 0;
		e.mod_r3   = 0;
		e.mod_r4   = 0;
		e.mod_r5   = 0;
		e.mod_r6   = 0;
		e.mod_r7   = 0;
		e.mod_r8   = 0;
		e.mod_r9   = 0;
		e.mod_r10  = 0;
		e.mod_r11  = 0;
		e.mod_r12  = 0;
		e.mod_r14  = 0;
		e.mod_r42  = 0;
		e.mod_r75  = 0;
		e.mod_r108 = 0;
		e.mod_r128 = 0;
		e.mod_r130 = 0;
		e.mod_r161 = 0;
		e.mod_r330 = 0;
		e.mod_r367 = 0;
		e.mod_r522 = 0;
		e.mod_d140 = 0;
		e.mod_d201 = 0;
		e.mod_d202 = 0;
		e.mod_d203 = 0;
		e.mod_d204 = 0;
		e.mod_d205 = 0;
		e.mod_d206 = 0;
		e.mod_d207 = 0;
		e.mod_d208 = 0;
		e.mod_d209 = 0;
		e.mod_d210 = 0;
		e.mod_d211 = 0;
		e.mod_d212 = 0;
		e.mod_d213 = 0;
		e.mod_d214 = 0;
		e.mod_d215 = 0;
		e.mod_d216 = 0;

		return e;
	}

	static FactionList GetFactionList(
		const std::vector<FactionList> &faction_lists,
		int faction_list_id
	)
	{
		for (auto &faction_list : faction_lists) {
			if (faction_list.id == faction_list_id) {
				return faction_list;
			}
		}

		return NewEntity();
	}

	static FactionList FindOne(
		Database& db,
		int faction_list_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				faction_list_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			FactionList e{};

			e.id       = static_cast<int32_t>(atoi(row[0]));
			e.name     = row[1] ? row[1] : "";
			e.base     = static_cast<int16_t>(atoi(row[2]));
			e.mod_c1   = static_cast<int16_t>(atoi(row[3]));
			e.mod_c2   = static_cast<int16_t>(atoi(row[4]));
			e.mod_c3   = static_cast<int16_t>(atoi(row[5]));
			e.mod_c4   = static_cast<int16_t>(atoi(row[6]));
			e.mod_c5   = static_cast<int16_t>(atoi(row[7]));
			e.mod_c6   = static_cast<int16_t>(atoi(row[8]));
			e.mod_c7   = static_cast<int16_t>(atoi(row[9]));
			e.mod_c8   = static_cast<int16_t>(atoi(row[10]));
			e.mod_c9   = static_cast<int16_t>(atoi(row[11]));
			e.mod_c10  = static_cast<int16_t>(atoi(row[12]));
			e.mod_c11  = static_cast<int16_t>(atoi(row[13]));
			e.mod_c12  = static_cast<int16_t>(atoi(row[14]));
			e.mod_c13  = static_cast<int16_t>(atoi(row[15]));
			e.mod_c14  = static_cast<int16_t>(atoi(row[16]));
			e.mod_c15  = static_cast<int16_t>(atoi(row[17]));
			e.mod_c16  = static_cast<int16_t>(atoi(row[18]));
			e.mod_r1   = static_cast<int16_t>(atoi(row[19]));
			e.mod_r2   = static_cast<int16_t>(atoi(row[20]));
			e.mod_r3   = static_cast<int16_t>(atoi(row[21]));
			e.mod_r4   = static_cast<int16_t>(atoi(row[22]));
			e.mod_r5   = static_cast<int16_t>(atoi(row[23]));
			e.mod_r6   = static_cast<int16_t>(atoi(row[24]));
			e.mod_r7   = static_cast<int16_t>(atoi(row[25]));
			e.mod_r8   = static_cast<int16_t>(atoi(row[26]));
			e.mod_r9   = static_cast<int16_t>(atoi(row[27]));
			e.mod_r10  = static_cast<int16_t>(atoi(row[28]));
			e.mod_r11  = static_cast<int16_t>(atoi(row[29]));
			e.mod_r12  = static_cast<int16_t>(atoi(row[30]));
			e.mod_r14  = static_cast<int16_t>(atoi(row[31]));
			e.mod_r42  = static_cast<int16_t>(atoi(row[32]));
			e.mod_r75  = static_cast<int16_t>(atoi(row[33]));
			e.mod_r108 = static_cast<int16_t>(atoi(row[34]));
			e.mod_r128 = static_cast<int16_t>(atoi(row[35]));
			e.mod_r130 = static_cast<int16_t>(atoi(row[36]));
			e.mod_r161 = static_cast<int16_t>(atoi(row[37]));
			e.mod_r330 = static_cast<int16_t>(atoi(row[38]));
			e.mod_r367 = static_cast<int16_t>(atoi(row[39]));
			e.mod_r522 = static_cast<int16_t>(atoi(row[40]));
			e.mod_d140 = static_cast<int16_t>(atoi(row[41]));
			e.mod_d201 = static_cast<int16_t>(atoi(row[42]));
			e.mod_d202 = static_cast<int16_t>(atoi(row[43]));
			e.mod_d203 = static_cast<int16_t>(atoi(row[44]));
			e.mod_d204 = static_cast<int16_t>(atoi(row[45]));
			e.mod_d205 = static_cast<int16_t>(atoi(row[46]));
			e.mod_d206 = static_cast<int16_t>(atoi(row[47]));
			e.mod_d207 = static_cast<int16_t>(atoi(row[48]));
			e.mod_d208 = static_cast<int16_t>(atoi(row[49]));
			e.mod_d209 = static_cast<int16_t>(atoi(row[50]));
			e.mod_d210 = static_cast<int16_t>(atoi(row[51]));
			e.mod_d211 = static_cast<int16_t>(atoi(row[52]));
			e.mod_d212 = static_cast<int16_t>(atoi(row[53]));
			e.mod_d213 = static_cast<int16_t>(atoi(row[54]));
			e.mod_d214 = static_cast<int16_t>(atoi(row[55]));
			e.mod_d215 = static_cast<int16_t>(atoi(row[56]));
			e.mod_d216 = static_cast<int16_t>(atoi(row[57]));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int faction_list_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				faction_list_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const FactionList &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[2] + " = " + std::to_string(e.base));
		v.push_back(columns[3] + " = " + std::to_string(e.mod_c1));
		v.push_back(columns[4] + " = " + std::to_string(e.mod_c2));
		v.push_back(columns[5] + " = " + std::to_string(e.mod_c3));
		v.push_back(columns[6] + " = " + std::to_string(e.mod_c4));
		v.push_back(columns[7] + " = " + std::to_string(e.mod_c5));
		v.push_back(columns[8] + " = " + std::to_string(e.mod_c6));
		v.push_back(columns[9] + " = " + std::to_string(e.mod_c7));
		v.push_back(columns[10] + " = " + std::to_string(e.mod_c8));
		v.push_back(columns[11] + " = " + std::to_string(e.mod_c9));
		v.push_back(columns[12] + " = " + std::to_string(e.mod_c10));
		v.push_back(columns[13] + " = " + std::to_string(e.mod_c11));
		v.push_back(columns[14] + " = " + std::to_string(e.mod_c12));
		v.push_back(columns[15] + " = " + std::to_string(e.mod_c13));
		v.push_back(columns[16] + " = " + std::to_string(e.mod_c14));
		v.push_back(columns[17] + " = " + std::to_string(e.mod_c15));
		v.push_back(columns[18] + " = " + std::to_string(e.mod_c16));
		v.push_back(columns[19] + " = " + std::to_string(e.mod_r1));
		v.push_back(columns[20] + " = " + std::to_string(e.mod_r2));
		v.push_back(columns[21] + " = " + std::to_string(e.mod_r3));
		v.push_back(columns[22] + " = " + std::to_string(e.mod_r4));
		v.push_back(columns[23] + " = " + std::to_string(e.mod_r5));
		v.push_back(columns[24] + " = " + std::to_string(e.mod_r6));
		v.push_back(columns[25] + " = " + std::to_string(e.mod_r7));
		v.push_back(columns[26] + " = " + std::to_string(e.mod_r8));
		v.push_back(columns[27] + " = " + std::to_string(e.mod_r9));
		v.push_back(columns[28] + " = " + std::to_string(e.mod_r10));
		v.push_back(columns[29] + " = " + std::to_string(e.mod_r11));
		v.push_back(columns[30] + " = " + std::to_string(e.mod_r12));
		v.push_back(columns[31] + " = " + std::to_string(e.mod_r14));
		v.push_back(columns[32] + " = " + std::to_string(e.mod_r42));
		v.push_back(columns[33] + " = " + std::to_string(e.mod_r75));
		v.push_back(columns[34] + " = " + std::to_string(e.mod_r108));
		v.push_back(columns[35] + " = " + std::to_string(e.mod_r128));
		v.push_back(columns[36] + " = " + std::to_string(e.mod_r130));
		v.push_back(columns[37] + " = " + std::to_string(e.mod_r161));
		v.push_back(columns[38] + " = " + std::to_string(e.mod_r330));
		v.push_back(columns[39] + " = " + std::to_string(e.mod_r367));
		v.push_back(columns[40] + " = " + std::to_string(e.mod_r522));
		v.push_back(columns[41] + " = " + std::to_string(e.mod_d140));
		v.push_back(columns[42] + " = " + std::to_string(e.mod_d201));
		v.push_back(columns[43] + " = " + std::to_string(e.mod_d202));
		v.push_back(columns[44] + " = " + std::to_string(e.mod_d203));
		v.push_back(columns[45] + " = " + std::to_string(e.mod_d204));
		v.push_back(columns[46] + " = " + std::to_string(e.mod_d205));
		v.push_back(columns[47] + " = " + std::to_string(e.mod_d206));
		v.push_back(columns[48] + " = " + std::to_string(e.mod_d207));
		v.push_back(columns[49] + " = " + std::to_string(e.mod_d208));
		v.push_back(columns[50] + " = " + std::to_string(e.mod_d209));
		v.push_back(columns[51] + " = " + std::to_string(e.mod_d210));
		v.push_back(columns[52] + " = " + std::to_string(e.mod_d211));
		v.push_back(columns[53] + " = " + std::to_string(e.mod_d212));
		v.push_back(columns[54] + " = " + std::to_string(e.mod_d213));
		v.push_back(columns[55] + " = " + std::to_string(e.mod_d214));
		v.push_back(columns[56] + " = " + std::to_string(e.mod_d215));
		v.push_back(columns[57] + " = " + std::to_string(e.mod_d216));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static FactionList InsertOne(
		Database& db,
		FactionList e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.base));
		v.push_back(std::to_string(e.mod_c1));
		v.push_back(std::to_string(e.mod_c2));
		v.push_back(std::to_string(e.mod_c3));
		v.push_back(std::to_string(e.mod_c4));
		v.push_back(std::to_string(e.mod_c5));
		v.push_back(std::to_string(e.mod_c6));
		v.push_back(std::to_string(e.mod_c7));
		v.push_back(std::to_string(e.mod_c8));
		v.push_back(std::to_string(e.mod_c9));
		v.push_back(std::to_string(e.mod_c10));
		v.push_back(std::to_string(e.mod_c11));
		v.push_back(std::to_string(e.mod_c12));
		v.push_back(std::to_string(e.mod_c13));
		v.push_back(std::to_string(e.mod_c14));
		v.push_back(std::to_string(e.mod_c15));
		v.push_back(std::to_string(e.mod_c16));
		v.push_back(std::to_string(e.mod_r1));
		v.push_back(std::to_string(e.mod_r2));
		v.push_back(std::to_string(e.mod_r3));
		v.push_back(std::to_string(e.mod_r4));
		v.push_back(std::to_string(e.mod_r5));
		v.push_back(std::to_string(e.mod_r6));
		v.push_back(std::to_string(e.mod_r7));
		v.push_back(std::to_string(e.mod_r8));
		v.push_back(std::to_string(e.mod_r9));
		v.push_back(std::to_string(e.mod_r10));
		v.push_back(std::to_string(e.mod_r11));
		v.push_back(std::to_string(e.mod_r12));
		v.push_back(std::to_string(e.mod_r14));
		v.push_back(std::to_string(e.mod_r42));
		v.push_back(std::to_string(e.mod_r75));
		v.push_back(std::to_string(e.mod_r108));
		v.push_back(std::to_string(e.mod_r128));
		v.push_back(std::to_string(e.mod_r130));
		v.push_back(std::to_string(e.mod_r161));
		v.push_back(std::to_string(e.mod_r330));
		v.push_back(std::to_string(e.mod_r367));
		v.push_back(std::to_string(e.mod_r522));
		v.push_back(std::to_string(e.mod_d140));
		v.push_back(std::to_string(e.mod_d201));
		v.push_back(std::to_string(e.mod_d202));
		v.push_back(std::to_string(e.mod_d203));
		v.push_back(std::to_string(e.mod_d204));
		v.push_back(std::to_string(e.mod_d205));
		v.push_back(std::to_string(e.mod_d206));
		v.push_back(std::to_string(e.mod_d207));
		v.push_back(std::to_string(e.mod_d208));
		v.push_back(std::to_string(e.mod_d209));
		v.push_back(std::to_string(e.mod_d210));
		v.push_back(std::to_string(e.mod_d211));
		v.push_back(std::to_string(e.mod_d212));
		v.push_back(std::to_string(e.mod_d213));
		v.push_back(std::to_string(e.mod_d214));
		v.push_back(std::to_string(e.mod_d215));
		v.push_back(std::to_string(e.mod_d216));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<FactionList> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.base));
			v.push_back(std::to_string(e.mod_c1));
			v.push_back(std::to_string(e.mod_c2));
			v.push_back(std::to_string(e.mod_c3));
			v.push_back(std::to_string(e.mod_c4));
			v.push_back(std::to_string(e.mod_c5));
			v.push_back(std::to_string(e.mod_c6));
			v.push_back(std::to_string(e.mod_c7));
			v.push_back(std::to_string(e.mod_c8));
			v.push_back(std::to_string(e.mod_c9));
			v.push_back(std::to_string(e.mod_c10));
			v.push_back(std::to_string(e.mod_c11));
			v.push_back(std::to_string(e.mod_c12));
			v.push_back(std::to_string(e.mod_c13));
			v.push_back(std::to_string(e.mod_c14));
			v.push_back(std::to_string(e.mod_c15));
			v.push_back(std::to_string(e.mod_c16));
			v.push_back(std::to_string(e.mod_r1));
			v.push_back(std::to_string(e.mod_r2));
			v.push_back(std::to_string(e.mod_r3));
			v.push_back(std::to_string(e.mod_r4));
			v.push_back(std::to_string(e.mod_r5));
			v.push_back(std::to_string(e.mod_r6));
			v.push_back(std::to_string(e.mod_r7));
			v.push_back(std::to_string(e.mod_r8));
			v.push_back(std::to_string(e.mod_r9));
			v.push_back(std::to_string(e.mod_r10));
			v.push_back(std::to_string(e.mod_r11));
			v.push_back(std::to_string(e.mod_r12));
			v.push_back(std::to_string(e.mod_r14));
			v.push_back(std::to_string(e.mod_r42));
			v.push_back(std::to_string(e.mod_r75));
			v.push_back(std::to_string(e.mod_r108));
			v.push_back(std::to_string(e.mod_r128));
			v.push_back(std::to_string(e.mod_r130));
			v.push_back(std::to_string(e.mod_r161));
			v.push_back(std::to_string(e.mod_r330));
			v.push_back(std::to_string(e.mod_r367));
			v.push_back(std::to_string(e.mod_r522));
			v.push_back(std::to_string(e.mod_d140));
			v.push_back(std::to_string(e.mod_d201));
			v.push_back(std::to_string(e.mod_d202));
			v.push_back(std::to_string(e.mod_d203));
			v.push_back(std::to_string(e.mod_d204));
			v.push_back(std::to_string(e.mod_d205));
			v.push_back(std::to_string(e.mod_d206));
			v.push_back(std::to_string(e.mod_d207));
			v.push_back(std::to_string(e.mod_d208));
			v.push_back(std::to_string(e.mod_d209));
			v.push_back(std::to_string(e.mod_d210));
			v.push_back(std::to_string(e.mod_d211));
			v.push_back(std::to_string(e.mod_d212));
			v.push_back(std::to_string(e.mod_d213));
			v.push_back(std::to_string(e.mod_d214));
			v.push_back(std::to_string(e.mod_d215));
			v.push_back(std::to_string(e.mod_d216));

			insert_chunks.push_back("(" + Strings::Implode(",", v) + ")");
		}

		std::vector<std::string> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<FactionList> All(Database& db)
	{
		std::vector<FactionList> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			FactionList e{};

			e.id       = static_cast<int32_t>(atoi(row[0]));
			e.name     = row[1] ? row[1] : "";
			e.base     = static_cast<int16_t>(atoi(row[2]));
			e.mod_c1   = static_cast<int16_t>(atoi(row[3]));
			e.mod_c2   = static_cast<int16_t>(atoi(row[4]));
			e.mod_c3   = static_cast<int16_t>(atoi(row[5]));
			e.mod_c4   = static_cast<int16_t>(atoi(row[6]));
			e.mod_c5   = static_cast<int16_t>(atoi(row[7]));
			e.mod_c6   = static_cast<int16_t>(atoi(row[8]));
			e.mod_c7   = static_cast<int16_t>(atoi(row[9]));
			e.mod_c8   = static_cast<int16_t>(atoi(row[10]));
			e.mod_c9   = static_cast<int16_t>(atoi(row[11]));
			e.mod_c10  = static_cast<int16_t>(atoi(row[12]));
			e.mod_c11  = static_cast<int16_t>(atoi(row[13]));
			e.mod_c12  = static_cast<int16_t>(atoi(row[14]));
			e.mod_c13  = static_cast<int16_t>(atoi(row[15]));
			e.mod_c14  = static_cast<int16_t>(atoi(row[16]));
			e.mod_c15  = static_cast<int16_t>(atoi(row[17]));
			e.mod_c16  = static_cast<int16_t>(atoi(row[18]));
			e.mod_r1   = static_cast<int16_t>(atoi(row[19]));
			e.mod_r2   = static_cast<int16_t>(atoi(row[20]));
			e.mod_r3   = static_cast<int16_t>(atoi(row[21]));
			e.mod_r4   = static_cast<int16_t>(atoi(row[22]));
			e.mod_r5   = static_cast<int16_t>(atoi(row[23]));
			e.mod_r6   = static_cast<int16_t>(atoi(row[24]));
			e.mod_r7   = static_cast<int16_t>(atoi(row[25]));
			e.mod_r8   = static_cast<int16_t>(atoi(row[26]));
			e.mod_r9   = static_cast<int16_t>(atoi(row[27]));
			e.mod_r10  = static_cast<int16_t>(atoi(row[28]));
			e.mod_r11  = static_cast<int16_t>(atoi(row[29]));
			e.mod_r12  = static_cast<int16_t>(atoi(row[30]));
			e.mod_r14  = static_cast<int16_t>(atoi(row[31]));
			e.mod_r42  = static_cast<int16_t>(atoi(row[32]));
			e.mod_r75  = static_cast<int16_t>(atoi(row[33]));
			e.mod_r108 = static_cast<int16_t>(atoi(row[34]));
			e.mod_r128 = static_cast<int16_t>(atoi(row[35]));
			e.mod_r130 = static_cast<int16_t>(atoi(row[36]));
			e.mod_r161 = static_cast<int16_t>(atoi(row[37]));
			e.mod_r330 = static_cast<int16_t>(atoi(row[38]));
			e.mod_r367 = static_cast<int16_t>(atoi(row[39]));
			e.mod_r522 = static_cast<int16_t>(atoi(row[40]));
			e.mod_d140 = static_cast<int16_t>(atoi(row[41]));
			e.mod_d201 = static_cast<int16_t>(atoi(row[42]));
			e.mod_d202 = static_cast<int16_t>(atoi(row[43]));
			e.mod_d203 = static_cast<int16_t>(atoi(row[44]));
			e.mod_d204 = static_cast<int16_t>(atoi(row[45]));
			e.mod_d205 = static_cast<int16_t>(atoi(row[46]));
			e.mod_d206 = static_cast<int16_t>(atoi(row[47]));
			e.mod_d207 = static_cast<int16_t>(atoi(row[48]));
			e.mod_d208 = static_cast<int16_t>(atoi(row[49]));
			e.mod_d209 = static_cast<int16_t>(atoi(row[50]));
			e.mod_d210 = static_cast<int16_t>(atoi(row[51]));
			e.mod_d211 = static_cast<int16_t>(atoi(row[52]));
			e.mod_d212 = static_cast<int16_t>(atoi(row[53]));
			e.mod_d213 = static_cast<int16_t>(atoi(row[54]));
			e.mod_d214 = static_cast<int16_t>(atoi(row[55]));
			e.mod_d215 = static_cast<int16_t>(atoi(row[56]));
			e.mod_d216 = static_cast<int16_t>(atoi(row[57]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<FactionList> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<FactionList> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			FactionList e{};

			e.id       = static_cast<int32_t>(atoi(row[0]));
			e.name     = row[1] ? row[1] : "";
			e.base     = static_cast<int16_t>(atoi(row[2]));
			e.mod_c1   = static_cast<int16_t>(atoi(row[3]));
			e.mod_c2   = static_cast<int16_t>(atoi(row[4]));
			e.mod_c3   = static_cast<int16_t>(atoi(row[5]));
			e.mod_c4   = static_cast<int16_t>(atoi(row[6]));
			e.mod_c5   = static_cast<int16_t>(atoi(row[7]));
			e.mod_c6   = static_cast<int16_t>(atoi(row[8]));
			e.mod_c7   = static_cast<int16_t>(atoi(row[9]));
			e.mod_c8   = static_cast<int16_t>(atoi(row[10]));
			e.mod_c9   = static_cast<int16_t>(atoi(row[11]));
			e.mod_c10  = static_cast<int16_t>(atoi(row[12]));
			e.mod_c11  = static_cast<int16_t>(atoi(row[13]));
			e.mod_c12  = static_cast<int16_t>(atoi(row[14]));
			e.mod_c13  = static_cast<int16_t>(atoi(row[15]));
			e.mod_c14  = static_cast<int16_t>(atoi(row[16]));
			e.mod_c15  = static_cast<int16_t>(atoi(row[17]));
			e.mod_c16  = static_cast<int16_t>(atoi(row[18]));
			e.mod_r1   = static_cast<int16_t>(atoi(row[19]));
			e.mod_r2   = static_cast<int16_t>(atoi(row[20]));
			e.mod_r3   = static_cast<int16_t>(atoi(row[21]));
			e.mod_r4   = static_cast<int16_t>(atoi(row[22]));
			e.mod_r5   = static_cast<int16_t>(atoi(row[23]));
			e.mod_r6   = static_cast<int16_t>(atoi(row[24]));
			e.mod_r7   = static_cast<int16_t>(atoi(row[25]));
			e.mod_r8   = static_cast<int16_t>(atoi(row[26]));
			e.mod_r9   = static_cast<int16_t>(atoi(row[27]));
			e.mod_r10  = static_cast<int16_t>(atoi(row[28]));
			e.mod_r11  = static_cast<int16_t>(atoi(row[29]));
			e.mod_r12  = static_cast<int16_t>(atoi(row[30]));
			e.mod_r14  = static_cast<int16_t>(atoi(row[31]));
			e.mod_r42  = static_cast<int16_t>(atoi(row[32]));
			e.mod_r75  = static_cast<int16_t>(atoi(row[33]));
			e.mod_r108 = static_cast<int16_t>(atoi(row[34]));
			e.mod_r128 = static_cast<int16_t>(atoi(row[35]));
			e.mod_r130 = static_cast<int16_t>(atoi(row[36]));
			e.mod_r161 = static_cast<int16_t>(atoi(row[37]));
			e.mod_r330 = static_cast<int16_t>(atoi(row[38]));
			e.mod_r367 = static_cast<int16_t>(atoi(row[39]));
			e.mod_r522 = static_cast<int16_t>(atoi(row[40]));
			e.mod_d140 = static_cast<int16_t>(atoi(row[41]));
			e.mod_d201 = static_cast<int16_t>(atoi(row[42]));
			e.mod_d202 = static_cast<int16_t>(atoi(row[43]));
			e.mod_d203 = static_cast<int16_t>(atoi(row[44]));
			e.mod_d204 = static_cast<int16_t>(atoi(row[45]));
			e.mod_d205 = static_cast<int16_t>(atoi(row[46]));
			e.mod_d206 = static_cast<int16_t>(atoi(row[47]));
			e.mod_d207 = static_cast<int16_t>(atoi(row[48]));
			e.mod_d208 = static_cast<int16_t>(atoi(row[49]));
			e.mod_d209 = static_cast<int16_t>(atoi(row[50]));
			e.mod_d210 = static_cast<int16_t>(atoi(row[51]));
			e.mod_d211 = static_cast<int16_t>(atoi(row[52]));
			e.mod_d212 = static_cast<int16_t>(atoi(row[53]));
			e.mod_d213 = static_cast<int16_t>(atoi(row[54]));
			e.mod_d214 = static_cast<int16_t>(atoi(row[55]));
			e.mod_d215 = static_cast<int16_t>(atoi(row[56]));
			e.mod_d216 = static_cast<int16_t>(atoi(row[57]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static int DeleteWhere(Database& db, const std::string &where_filter)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {}",
				TableName(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int Truncate(Database& db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"TRUNCATE TABLE {}",
				TableName()
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int64 GetMaxId(Database& db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COALESCE(MAX({}), 0) FROM {}",
				PrimaryKey(),
				TableName()
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static int64 Count(Database& db, const std::string &where_filter = "")
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COUNT(*) FROM {} {}",
				TableName(),
				(where_filter.empty() ? "" : "WHERE " + where_filter)
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

};

#endif //EQEMU_BASE_FACTION_LIST_REPOSITORY_H
