// Client -> server packets: the untrusted input FXServer parses from every connected player.
#include "packet_common.h"

#include <ArrayUpdate.h>
#include <GameStateAck.h>
#include <GameStateNAck.h>
#include <HeHost.h>
#include <IHost.h>
#include <IQuit.h>
#include <MumbleMessage.h>
#include <NetEvent.h>
#include <NetGameEventPacket.h>
#include <ObjectIds.h>
#include <ReassembledEventPacket.h>
#include <Route.h>
#include <ServerCommand.h>
#include <StateBag.h>
#include <TimeSync.h>

using namespace net::packet;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
	if (size < 1)
	{
		return -1;
	}

	const uint8_t selector = data[0];
	data++;
	size--;

	switch (selector % 17)
	{
	case 0: mayhem::ParseAndReserialize<ClientHeHost>(data, size); break;
	case 1: mayhem::ParseAndReserialize<ClientGameStateAck>(data, size); break;
	case 2: mayhem::ParseAndReserialize<ClientGameStateNAck>(data, size); break;
	case 3: mayhem::ParseAndReserialize<ClientIQuit>(data, size); break;
	case 4: mayhem::ParseAndReserialize<ClientIHost>(data, size); break;
	case 5: mayhem::ParseAndReserialize<ClientArrayUpdate>(data, size); break;
	case 6: mayhem::ParseAndReserialize<ClientServerEvent>(data, size); break;
	case 7: mayhem::ParseAndReserialize<ClientNetGameEvent>(data, size); break;
	case 8: mayhem::ParseAndReserialize<ClientNetGameEventV2>(data, size); break;
	case 9: mayhem::ParseAndReserialize<ClientServerCommand>(data, size); break;
	case 10: mayhem::ParseAndReserialize<ClientRoute>(data, size); break;
	case 11: mayhem::ParseAndReserialize<ClientRequestObjectIds>(data, size); break;
	case 12: mayhem::ParseAndReserialize<ClientMumbleMessage>(data, size); break;
	case 13: mayhem::ParseAndReserialize<ReassembledEvent>(data, size); break;
	case 14: mayhem::ParseAndReserialize<ReassembledEventV2>(data, size); break;
	case 15: mayhem::ParseAndReserialize<TimeSyncRequest>(data, size); break;
	case 16: mayhem::ParseAndReserialize<StateBagV2>(data, size); break;
	}

	return 0;
}
