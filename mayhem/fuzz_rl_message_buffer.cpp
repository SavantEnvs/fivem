// rl::MessageBuffer: the bit-level reader/writer behind the game's network sync messages.
#include <StdInc.h>

#include <state/RlMessageBuffer.h>

#include "script.h"

namespace rl
{
bool MessageBufferLengthHack::GetState()
{
	return false;
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
	if (size < 2)
	{
		return -1;
	}

	mayhem::Script header(data, 2);
	const size_t payloadSize = std::min<size_t>(header.U16(), size - 2);

	rl::MessageBuffer reader(data + 2, payloadSize);
	mayhem::Script script(data + 2 + payloadSize, size - 2 - payloadSize);

	rl::MessageBuffer writer(64);

	while (!script.Done())
	{
		const uint8_t op = script.U8();
		const int length = 1 + script.U8() % 32;

		switch (op % 13)
		{
		case 0: reader.Read<uint32_t>(length); break;
		case 1: reader.ReadSigned<int>(length); break;
		case 2: reader.ReadFloat(std::min(length, 30), 1.0f); break;
		case 3: reader.ReadSignedFloat(std::clamp(length, 2, 30), 1.0f); break;
		case 4: reader.ReadLong(1 + script.U8() % 64); break;
		case 5:
		{
			uint8_t tmp[64] = {};
			reader.ReadBits(tmp, script.U8() % 513);
			break;
		}
		case 6: reader.ReadBit(); break;
		case 7: reader.Align(); break;
		case 8: reader.SetCurrentBit(script.U16() % (payloadSize * 8 + 8)); break;
		case 9: reader.Clone().ReadBit(); break;
		case 10: reader.RequireLength(length); break;
		case 11:
		{
			// write then read back at the same position: the value must survive
			const uint32_t value = script.U32();
			const uint32_t mask = length == 32 ? 0xFFFFFFFFu : ((1u << length) - 1);
			const int pos = writer.GetCurrentBit();
			if (pos + length > 64 * 8)
			{
				writer.SetCurrentBit(0);
				break;
			}

			writer.Write<uint32_t>(length, value & mask);
			writer.SetCurrentBit(pos);
			if (writer.Read<uint32_t>(length) != (value & mask))
			{
				__builtin_trap();
			}
			break;
		}
		case 12:
		{
			uint8_t tmp[64] = {};
			for (auto& b : tmp)
			{
				b = script.U8();
			}
			writer.WriteBits(tmp, script.U8() % 513);
			break;
		}
		}
	}

	return 0;
}
