#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_parcels(Client *c, const Seperator *sep)
{
    const auto arguments = sep->argnum;
    if(!arguments) {
        SendParcelsSubCommands(c);
        return;
    }

    auto t = c;
    if(c->GetTarget() && c->GetTarget()->IsClient()) {
        t = c->GetTarget()->CastToClient();
    }

    bool is_listdb = !strcasecmp(sep->arg[1], "listdb");
    bool is_listmemory = !strcasecmp(sep->arg[1], "listmemory");
    bool is_details = !strcasecmp(sep->arg[1], "details");
    bool is_test = !strcasecmp(sep->arg[1], "test");

    if(!is_listdb && !is_listmemory && !is_details && !is_test) {
        SendParcelsSubCommands(c);
        return;
    }

    if(is_listdb) {
        auto player_name = std::string(sep->arg[2]);
        auto player = entity_list.GetClientByName(player_name.c_str());

        if(arguments < 2) {
            c->Message(Chat::White, "Usage: #parcels listdb [Character Name]");
        }
        else {
            if(player_name.empty()) {
                c->Message(Chat::White, fmt::format("You must provide a player name.").c_str());
                return;
            }
            else {
                auto results = ParcelsRepository::GetWhere(
                    database, fmt::format("to_name = '{}' ORDER BY slot_id ASC", player_name.c_str()));
                if(results.empty()) {
                    c->Message(Chat::White, fmt::format("No parcels could be found for {}", player_name).c_str());
                }
                else {
                    c->Message(Chat::Yellow,
                               fmt::format("Found {} parcels for {}.", results.size(), player_name).c_str());
                    for(auto const &p : results) {
                        c->Message(Chat::Yellow, fmt::format("Slot [{:02}] has item id [{:10}] with quantity [{}].",
                                   p.slot_id, p.item_id, p.quantity)
                                   .c_str());
                    }
                }
            }
        }
    }
    if(is_listmemory) {
        auto player_name = std::string(sep->arg[2]);
        auto player = entity_list.GetClientByName(player_name.c_str());

        if(arguments < 2) {
            c->Message(Chat::White, "Usage: #parcels listmemory [Character Name]");
        }
        else {
            if(!player) {
                c->Message(
                    Chat::White,
                    fmt::format(
                    "Player {} could not be found in this zone.  Ensure you are in the same zone as the player.",
                    player_name)
                    .c_str());
                return;
            }
            else {
                auto parcels = player->GetParcels();
                if(parcels.empty()) {
                    c->Message(Chat::White, fmt::format("No parcels could be found for {}", player_name).c_str());
                }
                c->Message(Chat::Yellow, fmt::format("Found {} parcels for {}.", parcels.size(), player_name).c_str());
                for(auto const &p : parcels) {
                    c->Message(Chat::Yellow, fmt::format("Slot [{:02}] has item id [{:10}] with quantity [{}].",
                               p.second.slot_id, p.second.item_id, p.second.quantity)
                               .c_str());
                }
            }
        }
    }
    else if(is_details) {
        auto player_name = std::string(sep->arg[2]);
        auto player = entity_list.GetClientByName(player_name.c_str());

        if(arguments < 2) {
            c->Message(Chat::White, "Usage: #parcels listdb [Character Name]");
        }
        else {
            if(player_name.empty()) {
                c->Message(Chat::White, fmt::format("You must provide a player name.").c_str());
                return;
            }
            else {
                if(!player) {
                    c->Message(Chat::White,
                               fmt::format("You must in the same zone as {} or {} is offline.",
                               player_name,
                               player_name).c_str()
                    );
                    return;
                }
                else {
                    c->Message(Chat::White,
                               fmt::format("{} parcel delay timer has {} seconds remaining.",
                               player_name,
                               player->GetParcelTimer()->GetRemainingTime() / 1000).c_str()
                    );
                    c->Message(Chat::White,
                               fmt::format("{} Engaged with a Parcel Merchant is: {}.",
                               player_name,
                               player->GetEngagedWithParcelMerchant()).c_str()
                    );
                    c->Message(Chat::White,
                               fmt::format("{} Able to send a parcel? {}.",
                               player_name,
                               player->GetParcelEnabled()).c_str()
                    );
                }
            }
        }
    }
    else if(is_test) {
        if(!sep->IsNumber(2)) {
            c->Message(Chat::White, "Usage: #parcels test []");
        }
        else {}
    }
}

void SendParcelsSubCommands(Client *c)
{
    c->Message(Chat::White, "#parcels listdb [Character Name]");
    c->Message(Chat::White, "#parcels listmemory [Character Name]");
    c->Message(Chat::White, "#parcels details [Character Name]");
}
