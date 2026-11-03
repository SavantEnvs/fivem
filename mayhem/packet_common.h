#pragma once

#include <StdInc.h>

#include <ByteCounter.h>
#include <ByteReader.h>
#include <ByteWriter.h>
#include <SerializableComponent.h>
#include <SerializableProperty.h>

namespace mayhem
{
inline bool Ok(bool value)
{
	return value;
}

inline bool Ok(net::SerializableResult value)
{
	return value == net::SerializableResult::Success;
}

// Mirrors net::PacketHandler::ProcessPacket (size window, then Process on a ByteReader); on a successful
// parse the component is serialized again into a buffer sized by ByteCounter, so a counter that
// under-reports (or a writer that overruns) is caught by ASan.
template<typename Packet>
void ParseAndReserialize(const uint8_t* data, size_t size)
{
	static const size_t kMinSize = net::SerializableComponent::GetMinSize<Packet>();
	static const size_t kMaxSize = net::SerializableComponent::GetMaxSize<Packet>();

	if (size < kMinSize || size > kMaxSize)
	{
		return;
	}

	net::ByteReader reader(data, size);
	Packet packet;
	if (!Ok(packet.Process(reader)))
	{
		return;
	}

	const size_t encodedSize = net::SerializableComponent::GetSize(packet);
	if (encodedSize > kMaxSize)
	{
		return;
	}

	// exact-size heap block (never empty, so the writer never sees a null destination)
	std::unique_ptr<uint8_t[]> out(new uint8_t[encodedSize ? encodedSize : 1]);
	net::ByteWriter writer(out.get(), encodedSize);
	if (!Ok(packet.Process(writer)))
	{
		// ByteCounter said the component fits in encodedSize bytes but the writer disagrees
		__builtin_trap();
	}
}
}
