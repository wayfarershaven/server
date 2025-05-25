#include "../client.h"
void command_seasoninfo(Client *c, const Seperator *sep)
{
	DataBucketKey k;
	k.character_id 	= c->CharacterID();
	k.key 		 	= "Season-LoginCount";

	int arguments   = sep->argnum;
	int login_count = Strings::ToInt(DataBucket::GetData(k).value);

	c->Message(Chat::White, "Welcome to Wayfarer's Haven first seasonal event! This is a special, time-limited event with character and time-locked progression, unique rewards, and unique challenges! You may not access certain features as a seasonal character, nor meaningfully interact with non-seasonal characters. See the Discord server for more information! This character was automatically included in the event, but you can remove it at any time using the #disable_seasonal command. This work is thanks to Catapultam and Aporia for their fantastic server and work!");

	c->Message(Chat::Yellow, "You have logged in %d days during this event!", login_count);
}
