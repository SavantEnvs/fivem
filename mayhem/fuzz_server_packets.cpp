// Server -> client packets: what the game client parses from a (possibly hostile) server.
#include "packet_common.h"

#include <ArrayUpdate.h>
#include <ConVars.h>
#include <Frame.h>
#include <IHost.h>
#include <NetEvent.h>
#include <NetGameEventPacket.h>
#include <ObjectIds.h>
#include <PackedAcksPacket.h>
#include <PackedClonesPacket.h>
#include <ResourcePacket.h>
#include <RpcNative.h>
#include <StateBag.h>
#include <TimeSync.h>
#include <WorldGrid.h>

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

	switch (selector % 16)
	{
	case 0: mayhem::ParseAndReserialize<ServerConVars>(data, size); break;
	case 1: mayhem::ParseAndReserialize<ServerFrame>(data, size); break;
	case 2: mayhem::ParseAndReserialize<ServerResourceStart>(data, size); break;
	case 3: mayhem::ParseAndReserialize<ServerResourceStop>(data, size); break;
	case 4: mayhem::ParseAndReserialize<ServerPackedClones>(data, size); break;
	case 5: mayhem::ParseAndReserialize<ServerIHost>(data, size); break;
	case 6: mayhem::ParseAndReserialize<ServerRpcNative>(data, size); break;
	case 7: mayhem::ParseAndReserialize<ServerClientEvent>(data, size); break;
	case 8: mayhem::ParseAndReserialize<ServerArrayUpdate>(data, size); break;
	case 9: mayhem::ParseAndReserialize<ServerWorldGrid>(data, size); break;
	case 10: mayhem::ParseAndReserialize<ServerObjectIds<false>>(data, size); break;
	case 15: mayhem::ParseAndReserialize<ServerObjectIds<true>>(data, size); break;
	case 11: mayhem::ParseAndReserialize<ServerPackedAcks>(data, size); break;
	case 12: mayhem::ParseAndReserialize<ServerNetGameEventV2>(data, size); break;
	case 13: mayhem::ParseAndReserialize<TimeSyncResponse>(data, size); break;
	case 14: mayhem::ParseAndReserialize<StateBag>(data, size); break;
	}

	return 0;
}
