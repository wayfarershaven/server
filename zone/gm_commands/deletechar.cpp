#include "../client.h"

void command_deletechar(Client *c, const Seperator *sep) {
	if (sep->arg[1][0] != 0) {
		if (!database.DeleteCharacter(sep->arg[1])) {
			c->Message(Chat::Red, "%s could not be deleted. Check the spelling of their name.", sep->arg[1]);
		} else {
			c->Message(Chat::Green, "%s successfully deleted!", sep->arg[1]);
		}
	} else {
		c->Message(Chat::White, "Usage: deletechar [charname] - WARNING THIS BYPASSES THE UNDELETE FUNCTION");
	}
}