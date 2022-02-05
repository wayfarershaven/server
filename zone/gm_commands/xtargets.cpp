#include "../client.h"

void command_xtargets(Client *c, const Seperator *sep)
{
	Client *t;

	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}
	else {
		t = c;
	}
	c->Message(Chat::Red, "WFH doesn't have XTargets");
}

