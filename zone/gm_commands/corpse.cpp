#include "../client.h"
#include "../corpse.h"

void command_corpse(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #corpse delete - Delete targeted corpse");
		c->Message(Chat::White, "Usage: #corpse deletenpccorpses - Deletes all NPC corpses");
		c->Message(Chat::White, "Usage: #corpse inspectloot - Inspects the loot on a corpse");
		c->Message(Chat::White, "Usage: #corpse listpc - Lists all player corpses");
		c->Message(Chat::White, "Usage: #corpse listnpc - Lists all NPC corpses");
		c->Message(Chat::White, "Usage: #corpse lock - Locks the corpse, only GMs can loot the corpse when it is locked");
		c->Message(Chat::White, "Usage: #corpse removecash - Removes the cash from a corpse");
		c->Message(Chat::White, "Usage: #corpse unlock - Unlocks the corpses, allowing non-GMs to loot the corpse");
		if (c->Admin() >= commandEditPlayerCorpses) {
			c->Message(Chat::White, "Usage: #corpse charid [Character ID] - Change player corpse's owner");
			c->Message(Chat::White, "Usage: #corpse deleteplayercorpses - Deletes all player corpses");
			c->Message(Chat::White, "Usage: #corpse depop [Bury] - Depops single target corpse.");
			c->Message(Chat::White, "Usage: #corpse depopall [Bury] - Depops all target player's corpses.");
			c->Message(Chat::White, "Usage: #corpse locate [all] - Located players corpse in zone [or all zones]");
			c->Message(Chat::White, "Usage: #corpse summon [corpseid/playername] - Summons targets corpses [or by corpseid/playername]");
			c->Message(Chat::White, "Usage: #corpse summonall - Summons all of targets corpses");
			c->Message(Chat::White, "Usage: #corpse buried [list/summon] - Lists or summons targets buried corpses");
			c->Message(Chat::White, "Usage: #corpse backup [list/summon] - Lists or summons targets backed-up corpses");
			c->Message(Chat::White, "Usage: #corpse moveallgraveyard - Moves all player corpses to the current zone's graveyard or non-instance");
			c->Message(Chat::White, "Note: Set bury to 0 to skip burying the corpses.");
		}
		return;
	}

	Mob *target = c->GetTarget();
	bool is_character_id = !strcasecmp(sep->arg[1], "charid");
	bool is_delete = !strcasecmp(sep->arg[1], "delete");
	bool is_delete_npc_corpses = !strcasecmp(sep->arg[1], "deletenpccorpses");
	bool is_delete_player_corpses = !strcasecmp(sep->arg[1], "deleteplayercorpses");
	bool is_depop = !strcasecmp(sep->arg[1], "depop");
	bool is_depop_all = !strcasecmp(sep->arg[1], "depopall");
	bool is_inspect_loot = !strcasecmp(sep->arg[1], "inspectloot");
	bool is_list_npc = !strcasecmp(sep->arg[1], "listnpc");
	bool is_list_player = !strcasecmp(sep->arg[1], "listpc");
	bool is_lock = !strcasecmp(sep->arg[1], "lock");
	bool is_move_all_to_graveyard = !strcasecmp(sep->arg[1], "moveallgraveyard");
	bool is_remove_cash = !strcasecmp(sep->arg[1], "removecash");
	bool is_reset_looter = !strcasecmp(sep->arg[1], "resetlooter");
	bool is_unlock = !strcasecmp(sep->arg[1], "unlock");
	bool is_locate = !strcasecmp(sep->arg[1], "locate");
	bool is_summon = !strcasecmp(sep->arg[1], "summon");
	bool is_summonall = !strcasecmp(sep->arg[1], "summonall");
	bool is_buried = !strcasecmp(sep->arg[1], "buried");
	bool is_backup = !strcasecmp(sep->arg[1], "backup");
	if (
		!is_character_id &&
		!is_delete &&
		!is_delete_npc_corpses &&
		!is_delete_player_corpses &&
		!is_depop &&
		!is_depop_all &&
		!is_inspect_loot &&
		!is_list_npc &&
		!is_list_player &&
		!is_lock &&
		!is_move_all_to_graveyard &&
		!is_remove_cash &&
		!is_reset_looter &&
		!is_unlock &&
		!is_locate &&
		!is_summon &&
		!is_summonall &&
		!is_buried &&
		!is_backup
	) {
		c->Message(Chat::White, "Usage: #corpse delete - Delete targeted corpse");
		c->Message(Chat::White, "Usage: #corpse deletenpccorpses - Deletes all NPC corpses");
		c->Message(Chat::White, "Usage: #corpse inspectloot - Inspects the loot on a corpse");
		c->Message(Chat::White, "Usage: #corpse listnpc - Lists all NPC corpses");
		c->Message(Chat::White, "Usage: #corpse lock - Locks the corpse, only GMs can loot the corpse when it is locked");
		c->Message(Chat::White, "Usage: #corpse removecash - Removes the cash from a corpse");
		c->Message(Chat::White, "Usage: #corpse unlock - Unlocks the corpses, allowing non-GMs to loot the corpse");
		if (c->Admin() >= commandEditPlayerCorpses) {
			c->Message(Chat::White, "Usage: #corpse charid [Character ID] - Change player corpse's owner");
			c->Message(Chat::White, "Usage: #corpse deleteplayercorpses - Deletes all player corpses");
			c->Message(Chat::White, "Usage: #corpse depop [Bury] - Depops single target corpse.");
			c->Message(Chat::White, "Usage: #corpse depopall [Bury] - Depops all target player's corpses.");
			c->Message(Chat::White, "Usage: #corpse locate [all] - Located players corpse in zone [or all zones]");
			c->Message(Chat::White, "Usage: #corpse summon [corpseid/playername] - Summons targets corpse [or by corpseid/playername]");
			c->Message(Chat::White, "Usage: #corpse summonall - Summons all of targets corpses");
			c->Message(Chat::White, "Usage: #corpse buried [list/summon] - Lists or summons targets buried corpses");
			c->Message(Chat::White, "Usage: #corpse backup [list/summon] - Lists or summons targets backed-up corpses");
			c->Message(Chat::White, "Usage: #corpse listplayer - Lists all player corpses");
			c->Message(Chat::White, "Usage: #corpse moveallgraveyard - Moves all player corpses to the current zone's graveyard or non-instance");
			c->Message(Chat::White, "Note: Set bury to 0 to skip burying the corpses.");
		}
		return;
	}


	if (is_delete_player_corpses) {
		if (c->Admin() >= commandEditPlayerCorpses) {
			auto corpses_deleted = entity_list.DeletePlayerCorpses();
			auto deleted_string = (
				corpses_deleted ?
				fmt::format(
					"{} Player corpse{} deleted.",					
					corpses_deleted,
					corpses_deleted != 1 ? "s" : ""
				) :
				"There are no player corpses to delete."
			);
			c->Message(Chat::White, deleted_string.c_str());
		} else {
			c->Message(Chat::White, "Your status is not high enough to delete player corpses.");
			return;
		}
	} else if (is_delete) {
		if (!target || !target->IsCorpse()) {
			c->Message(Chat::White, "You must target a corpse to use this command.");
			return;
		}

		if (target->IsPlayerCorpse() && c->Admin() < commandEditPlayerCorpses) {
			c->Message(Chat::White, "Your status is not high enough to delete a player corpse.");
			return;
		}

		if (
			target->IsNPCCorpse() || 
			c->Admin() >= commandEditPlayerCorpses
		) {
			c->Message(
				Chat::White,
				fmt::format(
					"Deleting {} corpse {}.",
					target->IsNPCCorpse() ? "NPC" : "player",
					c->GetTargetDescription(target)
				).c_str()
			);
			target->CastToCorpse()->Delete();
		} 
	} else if (is_list_npc) {
		entity_list.ListNPCCorpses(c);
	} else if (is_list_player) {
		if (c->Admin() < commandEditPlayerCorpses) {
			c->Message(Chat::White, "Your status is not high enough to list player corpses.");
			return;
		}

		entity_list.ListPlayerCorpses(c);
	} else if (is_delete_npc_corpses) {
		auto corpses_deleted = entity_list.DeleteNPCCorpses();
		auto deleted_string = (
			corpses_deleted ?
			fmt::format(
				"{} NPC corpse{} deleted.",
				corpses_deleted,
				corpses_deleted != 1 ? "s" : ""
			) :
			"There are no NPC corpses to delete."
		);
		c->Message(Chat::White, deleted_string.c_str());
	} else if (is_character_id) {
		if (c->Admin() >= commandEditPlayerCorpses) {
			if (!target || !target->IsPlayerCorpse()) {
				c->Message(Chat::White, "You must target a player corpse to use this command.");
				return;
			}

			if (!sep->IsNumber(2)) {
				c->Message(Chat::White, "Usage: #corpse charid [Character ID] - Change player corpse's owner");
				return;
			}

			auto character_id = std::stoi(sep->arg[2]);
			c->Message(
				Chat::White,
				fmt::format(
					"Setting the owner to {} ({}) for the player corpse {}.",
					database.GetCharNameByID(character_id),
					target->CastToCorpse()->SetCharID(character_id),
					c->GetTargetDescription(target)
				).c_str()
			);
		} else {
			c->Message(Chat::White, "Your status is not high enough to modify a player corpse's owner.");
			return;
		}
	} else if (is_reset_looter) {
		if (!target || !target->IsCorpse()) {
			c->Message(Chat::White, "You must target a corpse to use this command.");
			return;
		}

		if (target->IsPlayerCorpse() && c->Admin() < commandEditPlayerCorpses) {
			c->Message(Chat::White, "Your status is not high enough to reset looter on a player corpse.");
			return;
		}
		
		target->CastToCorpse()->ResetLooter();
		c->Message(
			Chat::White,
			fmt::format(
				"Reset looter for {} corpse {}.",
				target->IsNPCCorpse() ? "NPC" : "player",
				c->GetTargetDescription(target)
			).c_str()
		);
	} else if (is_remove_cash) {
		if (!target || !target->IsCorpse()) {
			c->Message(Chat::White, "You must target a corpse to use this command.");
			return;
		}

		if (target->IsPlayerCorpse() && c->Admin() < commandEditPlayerCorpses) {
			c->Message(Chat::White, "Your status is not high enough to remove cash from a player corpse.");
			return;
		}

		if (
			target->IsNPCCorpse() || 
			c->Admin() >= commandEditPlayerCorpses
		) {
			target->CastToCorpse()->RemoveCash();
			c->Message(
				Chat::White,
				fmt::format(
					"Removed cash from {} corpse {}.",
					target->IsNPCCorpse() ? "NPC" : "player",
					c->GetTargetDescription(target)
				).c_str()
			);
		}
	} else if (is_inspect_loot) {
		if (!target || !target->IsCorpse()) {
			c->Message(Chat::White, "You must target a corpse to use this command.");
			return;
		}

		if (target->IsPlayerCorpse() && c->Admin() < commandEditPlayerCorpses) {
			c->Message(Chat::White, "Your status is not high enough to inspect the loot of a player corpse.");
			return;
		}
		
		target->CastToCorpse()->QueryLoot(c);
	} else if (is_lock) {
		if (!target || !target->IsCorpse()) {
			c->Message(Chat::White, "You must target a corpse to use this command.");
			return;
		}

		if (target->IsPlayerCorpse() && c->Admin() < commandEditPlayerCorpses) {
			c->Message(Chat::White, "Your status is not high enough to lock player corpses.");
			return;
		}

		target->CastToCorpse()->Lock();
		c->Message(
			Chat::White,
			fmt::format(
				"Locking {} corpse {}.",
				target->IsNPCCorpse() ? "NPC" : "player",
				c->GetTargetDescription(target)
			).c_str()
		);
	} else if (is_unlock) {
		if (!target || !target->IsCorpse()) {
			c->Message(Chat::White, "You must target a corpse to use this command.");
			return;
		}

		if (target->IsPlayerCorpse() && c->Admin() < commandEditPlayerCorpses) {
			c->Message(Chat::White, "Your status is not high enough to unlock player corpses.");
			return;
		}

		target->CastToCorpse()->UnLock();
		c->Message(
			Chat::White,
			fmt::format(
				"Unlocking {} corpse {}.",
				target->IsNPCCorpse() ? "NPC" : "player",
				c->GetTargetDescription(target)
			).c_str()
		);
	} else if (is_depop) {
		if (!target || !target->IsPlayerCorpse()) {
			c->Message(Chat::White, "You must target a player corpse to use this command.");
			return;
		}

		if (c->Admin() >= commandEditPlayerCorpses) {
			bool bury_corpse = (
				sep->IsNumber(2) ?
				(
					std::stoi(sep->arg[2]) != 0 ?
					true :
					false
				) :
				false
			);
			c->Message(
				Chat::White,
				fmt::format(
					"Depopping player corpse {}.",
					c->GetTargetDescription(target)
				).c_str()
			);
			target->CastToCorpse()->DepopPlayerCorpse();
			if (bury_corpse) {
				target->CastToCorpse()->Bury();
			}
		} else {
			c->Message(Chat::White, "Your status is not high enough to depop a player corpse.");
			return;
		}
	} else if (is_depop_all) {
		if (!target || !target->IsClient()) {
			c->Message(Chat::White, "You must target a player to use this command.");
			return;
		}

		if (c->Admin() >= commandEditPlayerCorpses) {
			bool bury_corpse = (
				sep->IsNumber(2) ?
				(
					std::stoi(sep->arg[2]) != 0 ?
					true :
					false
				) :
				false
			);
			c->Message(
				Chat::White,
				fmt::format(
					"Depopping all player corpses for {}.",
					c->GetTargetDescription(target)
				).c_str()
			);
			target->CastToClient()->DepopAllCorpses();
			if (bury_corpse) {
				target->CastToClient()->BuryPlayerCorpses();
			}
		} else {
			c->Message(Chat::White, "Your status is not high enough to depop all of a player's corpses.");
			return;
		}
	} else if (is_move_all_to_graveyard) {
		int moved_count = entity_list.MovePlayerCorpsesToGraveyard(true);
		if (c->Admin() >= commandEditPlayerCorpses) {
			if (moved_count) {
				c->Message(
					Chat::White,
					fmt::format(
						"Moved {} player corpse{} to graveyard in {} ({}).",
						moved_count,
						moved_count != 1 ? "s" : "",
						ZoneLongName(zone->GetZoneID()),
						ZoneName(zone->GetZoneID())
					).c_str()
				);
			} else {
				c->Message(Chat::White, "There are no player corpses to move to the graveyard.");
			}
		} else {
			c->Message(Chat::White, "Your status is not high enough to move all player corpses to the graveyard.");
		}
	} else if (is_locate) {
		if (c->Admin() >= commandEditPlayerCorpses) {
			if (target == 0) {
				c->Message(Chat::White, "Error: Must have a target to locate their corpse.");
			} else {
				if (strlen(sep->arg[2]) > 0 && strcasecmp(sep->arg[2], "all") == 0 && target->IsClient()) {
					database.ListCharacterCorpses(target->CastToClient(), c, false, false);
				} else {
					entity_list.ListAllCorpses(c, target);
				}
			}
		} else {
			c->Message(Chat::White, "Your status is not high enough to move all player corpses to the graveyard.");
		}
	} else if (is_summon) {
		if (c->Admin() >= commandEditPlayerCorpses) {
			Corpse *crps;
			const char *name;

			if (strlen(sep->arg[2]) > 0) {
				if (sep->IsNumber(2)) {
					crps = entity_list.GetCorpseByID(atoi(sep->arg[2]));
					name = crps ? crps->GetOwnerName() : sep->arg[2];
				} else {
					name = sep->arg[2];
					crps = entity_list.GetCorpseByOwnerName(name);
				}
			} else {
				if (target == 0) {
					c->Message(Chat::White, "Must have target if not specifying a name of corpse to summon.");
				} else {
					name = target->GetName();
					crps = entity_list.GetCorpseByOwner(target->CastToClient());
				}
			}

			if (crps) {
				crps->Summon(c, true, false);
				c->Message(Chat::White, "Summoning corpse with name or ID: %s...", name);
			} else {
				c->Message(Chat::White, "No corpse to summon with name or ID: %s...", name);
			}
		}
	} else if (is_summonall) {
		if (c->Admin() >= commandEditPlayerCorpses) {
			if (target == 0) {
				c->Message(0, "Must have a valid target.");
			} else {
				target->CastToClient()->SummonAllCorpses(c->GetPosition());
			}
		}
	} else if (is_buried) {
		if (c->Admin() >= commandEditPlayerCorpses) {
			if (target == 0 || !target->IsClient()) {
				c->Message(Chat::White, "Error: Target must be a player to list or summon their buried corpses.");
			} else if (strlen(sep->arg[2]) > 0) {
				if (strcasecmp(sep->arg[2], "list") == 0) {
					c->Message(Chat::Red, "Listing buried corpses");
					database.ListCharacterCorpses(target->CastToClient(), c, true, false);
				} else if (strcasecmp(sep->arg[2], "summon") == 0) {
					if (c->Admin() >= commandEditPlayerCorpses) {
						uint32 corpseid;
						Client *t;

						if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM()) {
							t = c->GetTarget()->CastToClient();
						} else {
							c->Message(Chat::White, "You must first turn your GM flag on and select a target!");
							return;
						}

						if (!sep->IsNumber(3)) {
							c->Message(Chat::White, "Usage: #corpse buried summon [corpse_id].");
							return;
						} else {
							corpseid = atoi(sep->arg[3]);
						}

						if (!database.IsValidCorpse(corpseid)) {
							c->Message(Chat::Red,
								   "Corpse %i has been found! Please summon or delete it before attempting to restore trying to unbury it.",
								   atoi(sep->arg[2]));
							return;
						} else if (!database.IsCorpseOwner(corpseid, t->CharacterID(), false)) {
							c->Message(Chat::Red, "Targetted player is not the owner of the specified corpse!");
							return;
						} else {
							Corpse *PlayerCorpse = database.SummonCharacterCorpse(corpseid, t->CharacterID(),
									t->GetZoneID(), zone->GetInstanceID(),
									t->GetPosition());
							database.BuryCharacterCorpse(corpseid, 0); // unbury corpse
							if (!PlayerCorpse) {
								c->Message(Chat::White, "Summoning of backup corpse failed. Please escalate this issue.");
							}
							return;
						}
					} else {
						c->Message(Chat::White, "Insufficient status to summon backup corpses.");
					}
				}
			} else {
				c->Message(Chat::White, "Must provide argument 'list' or 'summon', ex: #corpse backup list");
			}
		}
	} else if (is_backup) {
		if (c->Admin() >= commandEditPlayerCorpses) {
			if (target == 0 || !target->IsClient()) {
				c->Message(Chat::White, "Error: Target must be a player to list their backups.");
			} else if (strlen(sep->arg[2]) > 0) {
				if (strcasecmp(sep->arg[2], "list") == 0) {
					c->Message(Chat::Red, "Listing backup corpses");
					database.ListCharacterCorpses(target->CastToClient(), c, false, true);
				} else if (strcasecmp(sep->arg[2], "summon") == 0) {
					if (c->Admin() >= commandEditPlayerCorpses) {
						uint32 corpseid;
						Client *t;

						if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM()) {
							t = c->GetTarget()->CastToClient();
						} else {
							c->Message(Chat::White, "You must first turn your GM flag on and select a target!");
							return;
						}

						if (!sep->IsNumber(3)) {
							c->Message(Chat::White, "Usage: #corpse backup summon [corpse_id].");
							return;
						} else {
							corpseid = atoi(sep->arg[3]);
						}

						// Check if backup corpse exists
						if (!database.IsValidCorpseBackup(corpseid)) {
							c->Message(Chat::Red, "Backup corpse %i not found.", corpseid);
							return;
						}

						// Check if corpse exists and is in buried list
						else if (database.IsValidCorpse(corpseid)) {
							c->Message(Chat::Red,
								   "Corpse %i has been found and is buried! Please summon or delete it before attempting to restore from a backup.",
								   corpseid);
							return;
						}

						// Check if corpse is owner of target
						else if (!database.IsCorpseOwner(corpseid, t->CharacterID(), true)) {
							c->Message(Chat::Red, "Targetted player is not the owner of the specified corpse!");
							return;
						} else {
							if (database.RestoreCorpseFromBackup(corpseid)) {
								Corpse *PlayerCorpse = database.SummonCharacterCorpse(corpseid, t->CharacterID(),
										t->GetZoneID(), zone->GetInstanceID(),
										t->GetPosition());
								database.BuryCharacterCorpse(corpseid, 0); // unbury corpse
								if (!PlayerCorpse) {
									c->Message(Chat::White,
											"Summoning of backup corpse failed. Please escalate this issue.");
								}
								return;
							} else {
								c->Message(Chat::Red, "There was an error copying corpse %i. Please contact a DB admin.",
										corpseid);
							return;
							}
						}
					} else {
						c->Message(Chat::White, "Insufficient status to summon backup corpses.");
					}
				} else {
					c->Message(Chat::White, "Must provide argument 'list' or 'summon', ex: #corpse backup list");
				}
			} else {
				c->Message(Chat::White, "Must provide argument 'list' or 'summon', ex: #corpse backup list");
			}
		}	
	}
}

