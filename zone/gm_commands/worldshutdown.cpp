#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_worldshutdown(Client *c, const Seperator *sep)
{
	// GM command to shutdown world server and all zone servers
	uint32 time=0;
	uint32 interval=0;
	if (worldserver.Connected()) {
		if(sep->IsNumber(1) && sep->IsNumber(2) && ((time=atoi(sep->arg[1]))>0) && ((interval=atoi(sep->arg[2]))>0)) {
			worldserver.SendEmoteMessage(0,0,15,"<SYSTEMWIDE MESSAGE>:SYSTEM MSG:World coming down in %i minutes, everyone log out before this time.",  (time / 60 ));
			c->Message(Chat::White, "Sending shutdown packet now, World will shutdown in: %i minutes with an interval of: %i seconds",  (time / 60), interval);
			auto pack = new ServerPacket(ServerOP_ShutdownAll, sizeof(WorldShutDown_Struct));
			WorldShutDown_Struct* wsd = (WorldShutDown_Struct*)pack->pBuffer;
			wsd->time=time*1000;
			wsd->interval=(interval*1000);
			worldserver.SendPacket(pack);
			safe_delete(pack);
		}
		else if(strcasecmp(sep->arg[1], "now") == 0){
			worldserver.SendEmoteMessage(0,0,15,"<SYSTEMWIDE MESSAGE>:SYSTEM MSG:World coming down, everyone log out now.");
			c->Message(Chat::White, "Sending shutdown packet");
			auto pack = new ServerPacket;
			pack->opcode = ServerOP_ShutdownAll;
			pack->size=0;
			worldserver.SendPacket(pack);
			safe_delete(pack);
		}
		else if(strcasecmp(sep->arg[1], "disable") == 0){
			c->Message(Chat::White, "Shutdown prevented, next time I may not be so forgiving...");
			auto pack = new ServerPacket(ServerOP_ShutdownAll, sizeof(WorldShutDown_Struct));
			WorldShutDown_Struct* wsd = (WorldShutDown_Struct*)pack->pBuffer;
			wsd->time=0;
			wsd->interval=0;
			worldserver.SendPacket(pack);
			safe_delete(pack);
		}
		else{
			c->Message(Chat::White,"#worldshutdown - Shuts down the server and all zones.");
			c->Message(Chat::White,"Usage: #worldshutdown now - Shuts down the server and all zones immediately.");
			c->Message(Chat::White,"Usage: #worldshutdown disable - Stops the server from a previously scheduled shut down.");
			c->Message(Chat::White,"Usage: #worldshutdown [timer] [interval] - Shuts down the server and all zones after [timer] seconds and sends warning every [interval] seconds.");
		}
	}
	else
		c->Message(Chat::White, "Error: World server disconnected");
}
