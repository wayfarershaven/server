#include "../client.h"

void command_timers(Client *c, const Seperator *sep)
{
	auto target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	std::vector<std::pair<pTimerType, PersistentTimer *>> timers;
	target->GetPTimers().ToVector(timers);

	std::string popup_title = fmt::format(
		"Recast Timers for {}",
		c == target ?
		"Yourself" :
		fmt::format(
			"{} ({})",
			target->GetCleanName(),
			target->GetID()
		)
	);

	std::string popup_text = "<table>";

	popup_text += "<tr><td>Timer ID</td><td>Remaining</td></tr>";

	for (const auto& timer : timers) {
		auto remaining_time = timer.second->GetRemainingTime();
		if (remaining_time) {
			popup_text += fmt::format(
				"<tr><td>{}</td><td>{}</td></tr>",
				timer.first,
				ConvertSecondsToTime(remaining_time)
			);
		}
	}

	popup_text += "</table>";

	c->SendPopupToClient(
		popup_title.c_str(),
		popup_text.c_str()
	);
}

void command_timers_clear(Client* c, const Seperator* sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsClient())
	{
		c->Message(0, "Need a player target for timers_clear.");
	}
	Client* target = c->GetTarget()->CastToClient();
	c->GetPTimers().Clear(&database);

	c->Message(0, "Timers cleared for %s.", c->GetName());
}
