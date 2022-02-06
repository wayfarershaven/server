#include "../client.h"

void command_reimburse(Client *c, const Seperator *sep) {
	if (sep->argnum > 0) {
		if (strcasecmp(sep->arg[2], "") == 0 && strcasecmp(sep->arg[3], "") == 0){
			c->Message(Chat::Red, "ERROR IN COMMAND FORMAT:");
			c->Message(Chat::Red, "#reimburse usage:");
			c->Message(Chat::Red, "--- #reimburse <character_name> <item_id> <reason> - Reimburses a single item ID");
		} else {
			std::string char_name = sep->arg[1];
			auto char_id = database.GetCharacterID(sep->arg[1]);
			if(char_id == 0) {
				c->Message(Chat::Red, "Character does not exist.");
				return;
			}
			uint32 item_id = atoi(sep->arg[2]);
			std::string item_name = "";

			if(item_id > 0) {
				const EQ::ItemData *item = nullptr;
				item = database.GetItem(item_id);

				if (item == nullptr) {
					c->Message(Chat::Red, "ERROR: ITEM DOES NOT EXIST");
					return;
				} else {
					item_name = item->Name;
				}

				if(item_name.length() == 0) {
					c->Message(Chat::Red, "ERROR: ITEM DOES NOT EXIST");
					return;
				}


			} else if (item_id <= 0){
				c->Message(Chat::Red, "ERROR IN COMMAND FORMAT:");
				c->Message(Chat::Red, "#reimburse usage:");
				c->Message(Chat::Red, "--- #reimburse <character_name> <item_id> <reason> - Reimburses a single item ID");
				return;
			}
			std::string message;
			int i = 3;
			while(1) {
				if(sep->arg[i][0] == 0) {
					break;
				}
				if(message.length() > 0) {
					message.push_back(' ');
				}
				message += sep->arg[i];
				++i;
			}
			if(message.length() == 0) {
				c->Message(Chat::Red, "ERROR IN COMMAND FORMAT:");
				c->Message(Chat::Red, "#reimburse usage:");
				c->Message(Chat::Red, "--- #reimburse <character_name> <item_id> <reason> - Reimburses a single item ID");
				return;
			}
			std::string query = StringFormat("INSERT INTO `cust_playerawards` (`CharID`, `Item_id`, `Reason`) VALUES (%i, %i, '%s')",
											 char_id, item_id, EscapeString(message).c_str());
			auto results = database.QueryDatabase(query);
			c->Message(Chat::Lime, "Successfully added item: %s (%i) to Vhanna for player: %s (%i) For Reason: (%s)", EscapeString(item_name).c_str(), item_id, EscapeString(char_name).c_str(), char_id, EscapeString(message).c_str());
		}
	} else {
		c->Message(Chat::Red, "#reimburse usage:");
		c->Message(Chat::Red, "--- #reimburse <character_name> <item_id> <reason> - Reimburses a single item ID");
	}
}
