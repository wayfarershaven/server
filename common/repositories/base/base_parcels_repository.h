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

#ifndef EQEMU_BASE_PARCELS_REPOSITORY_H
#define EQEMU_BASE_PARCELS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>


class BaseParcelsRepository {
public:
	struct Parcels {
		uint64_t    id;
		uint32_t    char_id;
		uint32_t    serial_id;
		std::string from_name;
		std::string from_note;
		time_t      sent_date;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"char_id",
			"serial_id",
			"from_name",
			"from_note",
			"sent_date",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"char_id",
			"serial_id",
			"from_name",
			"from_note",
			"UNIX_TIMESTAMP(sent_date)",
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
		return std::string("parcels");
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

	static Parcels NewEntity()
	{
		Parcels e{};

		e.id        = 0;
		e.char_id   = 0;
		e.serial_id = 0;
		e.from_name = "";
		e.from_note = "";
		e.sent_date = 0;

		return e;
	}

	static Parcels GetParcels(
		const std::vector<Parcels> &parcelss,
		int parcels_id
	)
	{
		for (auto &parcels : parcelss) {
			if (parcels.id == parcels_id) {
				return parcels;
			}
		}

		return NewEntity();
	}

	static Parcels FindOne(
		Database& db,
		int parcels_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				parcels_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Parcels e{};

			e.id        = strtoull(row[0], nullptr, 10);
			e.char_id   = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.serial_id = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.from_name = row[3] ? row[3] : "";
			e.from_note = row[4] ? row[4] : "";
			e.sent_date = strtoll(row[5] ? row[5] : "-1", nullptr, 10);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int parcels_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				parcels_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Parcels &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.char_id));
		v.push_back(columns[2] + " = " + std::to_string(e.serial_id));
		v.push_back(columns[3] + " = '" + Strings::Escape(e.from_name) + "'");
		v.push_back(columns[4] + " = '" + Strings::Escape(e.from_note) + "'");
		v.push_back(columns[5] + " = FROM_UNIXTIME(" + (e.sent_date > 0 ? std::to_string(e.sent_date) : "null") + ")");

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

	static Parcels InsertOne(
		Database& db,
		Parcels e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.char_id));
		v.push_back(std::to_string(e.serial_id));
		v.push_back("'" + Strings::Escape(e.from_name) + "'");
		v.push_back("'" + Strings::Escape(e.from_note) + "'");
		v.push_back("FROM_UNIXTIME(" + (e.sent_date > 0 ? std::to_string(e.sent_date) : "null") + ")");

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
		const std::vector<Parcels> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.char_id));
			v.push_back(std::to_string(e.serial_id));
			v.push_back("'" + Strings::Escape(e.from_name) + "'");
			v.push_back("'" + Strings::Escape(e.from_note) + "'");
			v.push_back("FROM_UNIXTIME(" + (e.sent_date > 0 ? std::to_string(e.sent_date) : "null") + ")");

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

	static std::vector<Parcels> All(Database& db)
	{
		std::vector<Parcels> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Parcels e{};

			e.id        = strtoull(row[0], nullptr, 10);
			e.char_id   = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.serial_id = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.from_name = row[3] ? row[3] : "";
			e.from_note = row[4] ? row[4] : "";
			e.sent_date = strtoll(row[5] ? row[5] : "-1", nullptr, 10);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Parcels> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Parcels> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Parcels e{};

			e.id        = strtoull(row[0], nullptr, 10);
			e.char_id   = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.serial_id = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.from_name = row[3] ? row[3] : "";
			e.from_note = row[4] ? row[4] : "";
			e.sent_date = strtoll(row[5] ? row[5] : "-1", nullptr, 10);

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

#endif //EQEMU_BASE_PARCELS_REPOSITORY_H
