/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "../common/global_define.h"
#include "../common/eqemu_logsys.h"
#include "../common/rulesys.h"
#include "../common/strings.h"
#include "../common/misc_functions.h"
#include "../common/events/player_event_logs.h"
#include "../common/repositories/trader_repository.h"

#include "client.h"
#include "entity.h"
#include "mob.h"
#include "parcels.h"

#include "quest_parser_collection.h"
#include "string_ids.h"
#include "worldserver.h"

class QueryServ;

extern WorldServer worldserver;
extern QueryServ* QServ;

void Client::SendBulkParcels(uint32 merchant_id)
{
	auto parcels = ParcelsRepository::GetWhere(database, fmt::format("char_id = {}", CharacterID()).c_str());

	for (auto const& p : parcels) {
		auto item = database.GetItem(p.serial_id);
		if (item) {
			auto charges = 15;
			auto inst = database.CreateItem(item, charges);
			if (inst) {
				auto item_price = static_cast<uint32>(item->Price * RuleR(Merchant, SellCostMod) * item->SellRate);
				inst->SetCharges(14);
				inst->SetMerchantCount(1);
				inst->SetMerchantSlot(p.id);

				SendParcelPacket(inst, p);
				safe_delete(inst);
			}
		}
	}
}

void Client::SendParcelPacket(const EQ::ItemInstance* inst, BaseParcelsRepository::Parcels p)
{
	struct Parcel_Struct
	{
		/*000*/	ItemPacketType	PacketType;
		/*004*/	char			SerializedItem[1];
		/*944*/	uint32			sent_time;
		/*948*/ uint32			player_name_length;
		/**/	char			player_name[1];
		/**/	uint32			note_length;
		/**/	char			note[1];
		/*xx*/
	};

	if (!inst) {
		return;
	}

	// Serialize item into |-delimited string (Titanium- uses '|' delimiter .. newer clients use pure data serialization)
	std::string packet = inst->Serialize(p.id);

	std::string player_name = p.from_name;
	std::string note = p.from_note;

	auto p_size = 16 + 4 + packet.length() + player_name.length() + note.length();
	auto out = new EQApplicationPacket(OP_ItemPacket, p_size);
	auto buffer = new char[p_size];
	char* bufptr = buffer;
	memset(buffer, 0, p_size);

	VARSTRUCT_ENCODE_TYPE(uint32, bufptr, ItemPacketParcel);
	memcpy(bufptr, packet.data(), packet.length());	bufptr += packet.length();
	VARSTRUCT_ENCODE_TYPE(uint32, bufptr, time(nullptr));
	VARSTRUCT_ENCODE_TYPE(uint32, bufptr, player_name.length());
	memcpy(bufptr, player_name.data(), player_name.length()); 
	bufptr += player_name.length();
	VARSTRUCT_ENCODE_TYPE(uint32, bufptr, note.length());
	memcpy(bufptr, note.data(), note.length());	bufptr += note.length();
	VARSTRUCT_ENCODE_TYPE(uint32, bufptr, packet.length()); 

	memcpy(out->pBuffer, buffer, p_size);
	safe_delete_array(buffer);
	FastQueuePacket(&out);
}

void Client::BuyTraderItemByParcel(TraderBuy_Struct* tbs, const EQApplicationPacket* app) {

	if (ClientVersion() < EQ::versions::ClientVersion::RoF)
	{
		return;
	}

	auto item_id = tbs->ItemID;
	auto item = TraderRepository::GetTraderItem(database, tbs->TraderID, tbs->ItemID, tbs->Price);

	if (!item.char_id) {
		Message(Chat::Yellow, "Unable to purchase {} {} from {}.  The item has already sold.",
			tbs->Quantity,
			tbs->ItemName,
			tbs->SellerName
		);
		LogTrading("Attempt to purchase {} {} from {} by {} though item could not be found on trader.  Likely already sold.",
			tbs->Quantity,
			tbs->ItemName,
			tbs->SellerName,
			GetName()
		);
		TradeRequestFailed(app);
		return;
	}

	auto item_inst = database.CreateItem(tbs->ItemID, tbs->Quantity);
	LogTrading("Buyitem: Name: [{}], IsStackable: [{}], Requested Quantity: [{}]",
		item_inst->GetItem()->Name, item_inst->IsStackable(), tbs->Quantity);

	auto outapp = new EQApplicationPacket(OP_TraderShop, sizeof(TraderBuy_Struct));
	auto data = (TraderBuy_Struct*)outapp->pBuffer;

	if (!item_inst->IsStackable())
		data->Quantity = 1;
	else {
		if (tbs->Quantity > item.charges) {
			Message(Chat::Yellow, "Unable to purchase {} {} from {}.  The requested quantity is no longer available.  Only {} remain.",
				tbs->Quantity,
				tbs->ItemName,
				tbs->SellerName,
				item.charges
			);
			LogTrading("Attempt to purchase {} {} from {} by {}.  The requested quantity is no longer available.  Only {} remain.",
				tbs->Quantity,
				tbs->ItemName,
				tbs->SellerName,
				GetName(),
				item.charges
			);
			return;
			TradeRequestFailed(app);
		}
		else {
			data->Quantity = tbs->Quantity;
		}
	}

	uint64 total_cost = static_cast<uint64>(tbs->Price) * static_cast<uint64>(data->Quantity);

	if (total_cost > MAX_TRANSACTION_VALUE) {
		Message(Chat::Red, "That would exceed the single transaction limit of %u platinum.", MAX_TRANSACTION_VALUE / 1000);
		//TradeRequestFailed(app);
		safe_delete(outapp);
		return;
	}

	// This cannot overflow assuming MAX_TRANSACTION_VALUE, checked above, is the default of 2000000000
	uint32 TotalCost = tbs->Price * data->Quantity;
	data->Price = tbs->Price;

	if (!TakeMoneyFromPP(TotalCost)) {
		RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{ .message = "Attempted to buy something in bazaar but did not have enough money." });
		//TradeRequestFailed(app);
		safe_delete(outapp);
		return;
	}

	LogTrading("Customer Paid: [{}] in Copper", TotalCost);

	uint32 platinum = TotalCost / 1000;
	TotalCost -= (platinum * 1000);
	uint32 gold = TotalCost / 100;
	TotalCost -= (gold * 100);
	uint32 silver = TotalCost / 10;
	TotalCost -= (silver * 10);
	uint32 copper = TotalCost;

	//Trader->AddMoneyToPP(copper, silver, gold, platinum, true);


	if (player_event_logs.IsEventEnabled(PlayerEvent::TRADER_PURCHASE)) {
		auto e = PlayerEvent::TraderPurchaseEvent{
			.item_id = item.item_id,
			.item_name = item_inst->GetItem()->Name,
			.trader_id = item.char_id,
			.trader_name = tbs->SellerName,
			.price = tbs->Price,
			.charges = data->Quantity,
			.total_cost = TotalCost,
			.player_money_balance = GetCarriedMoney(),
			.method = ByParcel
		};

		RecordPlayerEventLog(PlayerEvent::TRADER_PURCHASE, e);
	}

	if (player_event_logs.IsEventEnabled(PlayerEvent::TRADER_SELL)) {
		auto e = PlayerEvent::TraderSellEvent{
			.item_id = item.item_id,
			.item_name = item_inst->GetItem()->Name,
			.buyer_id = CharacterID(),
			.buyer_name = GetCleanName(),
			.price = tbs->Price,
			.charges = data->Quantity,
			.total_cost = TotalCost,
			.player_money_balance = 0,
			.method = ByParcel
		};
	}

	LogTrading("Trader Received: [{}] Platinum, [{}] Gold, [{}] Silver, [{}] Copper", platinum, gold, silver, copper);

	BaseParcelsRepository::Parcels parcel{};
	parcel.char_id = item.char_id;
	parcel.serial_id = item.item_id;
	parcel.sent_date = time(nullptr);
	parcel.from_name = tbs->SellerName;
	parcel.from_note = fmt::format("Thank you for your purchase of {} {}.",
		tbs->Quantity,
		tbs->ItemName
	);

	auto results = BaseParcelsRepository::InsertOne(database, parcel);
	if (!results.id > 0) {
		LogTrading("Error writing parcel delivery to database with id [{}]\.", results.id);
		Message(Chat::Red, "There was an error sending your parcel.");
		return;
	}

	data->Action   = BazaarBuyItem;
	data->Method   = ByParcel;
	data->TraderID = tbs->TraderID;
	data->ItemID   = tbs->ItemID;
	data->Price    = tbs->Price;
		
	strn0cpy(data->ItemName, item_inst->GetItem()->Name, sizeof(data->ItemName));
	strn0cpy(data->SellerName, tbs->SellerName, sizeof(data->SellerName));
	memcpy(data->SerialNumber, fmt::format("{:016}", tbs->ItemID).data(), sizeof(data->SerialNumber));

	QueuePacket(outapp);
	safe_delete(outapp);

//	ReturnTraderReq(app, data->Quantity, data->ItemID);
}
