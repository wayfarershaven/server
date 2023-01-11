#include "../client.h"

void command_xpinfo(Client *c, const Seperator *sep){

	Client *t;

	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	} else {
		t = c;
	}

	uint16 level = t->GetLevel();
	uint64 totalrequiredxp = t->GetEXPForLevel(level + 1);
	uint64 currentxp = t->GetEXP();
	float xpforlevel = totalrequiredxp - currentxp;
	float totalxpforlevel = totalrequiredxp - t->GetEXPForLevel(level);
	float xp_percent = 100.0 - ((xpforlevel/totalxpforlevel) * 100.0);

	uint64 exploss;
	t->GetExpLoss(nullptr, 0, exploss);
	float loss_percent = (exploss/totalxpforlevel) * 100.0;

	float maxaa = t->GetEXPForLevel(0, true);
	uint64 currentaaxp = t->GetAAXP();
	float aa_percent = (currentaaxp/maxaa) * 100.0;

	c->Message(Chat::Yellow, fmt::format("{} has {} of {} required XP.", t->GetName(), currentxp, totalrequiredxp).c_str());
	c->Message(Chat::Yellow, fmt::format("They need {} more to get to {}. They are {} percent towards this level.", xpforlevel, level+1, xp_percent).c_str());
	c->Message(Chat::Yellow, fmt::format("Their XP loss at this level is {} which is {} percent of their current level.", exploss, loss_percent).c_str());
	c->Message(Chat::Yellow, fmt::format("They have {} of {} towards an AA point. They are {} percent towards this point.", currentaaxp, maxaa, aa_percent).c_str());
}