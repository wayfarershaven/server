#include "../client.h"

void command_showbonusstats(Client *c, const Seperator *sep)
{
	if (c->GetTarget() == 0) {
		c->Message(Chat::White, "ERROR: No target!");
	}
	else if (!c->GetTarget()->IsMob() && !c->GetTarget()->IsClient()) {
		c->Message(Chat::White, "ERROR: Target is not a Mob or Player!");
	}
	else {
		bool bAll = false;
		if (sep->arg[1][0] == '\0' || strcasecmp(sep->arg[1], "all") == 0) {
			bAll = true;
		}
		if (bAll || (strcasecmp(sep->arg[1], "item") == 0)) {
			c->Message(Chat::White, "Target Item Bonuses:");
			c->Message(
				Chat::White,
				"  Accuracy: %i%%   Divine Save: %i%%",
				c->GetTarget()->GetItemBonuses().Accuracy,
				c->GetTarget()->GetItemBonuses().DivineSaveChance
			);
			c->Message(
				Chat::White,
				"  Flurry: %i%%     HitChance: %i%%",
				c->GetTarget()->GetItemBonuses().FlurryChance,
				c->GetTarget()->GetItemBonuses().HitChance / 15
			);
		}
		if (bAll || (strcasecmp(sep->arg[1], "spell") == 0)) {
			c->Message(Chat::White, "  Target Spell Bonuses:");
			c->Message(
				Chat::White,
				"  Accuracy: %i%%   Divine Save: %i%%",
				c->GetTarget()->GetSpellBonuses().Accuracy,
				c->GetTarget()->GetSpellBonuses().DivineSaveChance
			);
			c->Message(
				Chat::White,
				"  Flurry: %i%%     HitChance: %i%% ",
				c->GetTarget()->GetSpellBonuses().FlurryChance,
				c->GetTarget()->GetSpellBonuses().HitChance / 15
			);
		}
		if (bAll || (strcasecmp(sep->arg[1], "aa") == 0)) {
			c->Message(0, " Ambidexterity: %i%% ", c->GetTarget()->GetAABonuses().Ambidexterity);
			if (c->GetTarget()->IsPet()) {
				c->Message(0, " PetFlurry: %i%% ", c->GetTarget()->GetAABonuses().PetFlurry);
				c->Message(0, " PC_Pet_Flurry: %i%% ", c->GetTarget()->GetAABonuses().PC_Pet_Flurry);
				c->Message(0, " PC_Pet_Rampage: %i%% ", c->GetTarget()->GetAABonuses().PC_Pet_Rampage);
				c->Message(0, " PetCriticalHit: %i%% ", c->GetTarget()->GetAABonuses().PetCriticalHit);
			}
		}
		c->Message(Chat::White, "  Effective Casting Level: %i", c->GetTarget()->GetCasterLevel(0));
	}
}

