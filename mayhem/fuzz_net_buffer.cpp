// net::Buffer: the byte cursor the server-side handlers read client packets with.
#include <StdInc.h>

#include <NetBuffer.h>

#include "script.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
	if (size < 3)
	{
		return -1;
	}

	mayhem::Script header(data, 2);
	const size_t payloadSize = std::max<size_t>(1, std::min<size_t>(header.U16(), size - 2));

	net::Buffer buffer(data + 2, payloadSize);
	net::Buffer sink;
	mayhem::Script script(data + 2 + payloadSize, size - 2 - payloadSize);

	while (!script.Done())
	{
		switch (script.U8() % 12)
		{
		case 0: buffer.Read<uint8_t>(); break;
		case 1: buffer.Read<uint16_t>(); break;
		case 2: buffer.Read<uint32_t>(); break;
		case 3: buffer.Read<uint64_t>(); break;
		case 4: buffer.Read<std::string>(script.U16()); break;
		case 5: buffer.Read<std::string_view>(script.U16()); break;
		case 6:
		{
			const uint16_t length = script.U16();
			std::vector<uint8_t> tmp(length + 1);
			buffer.Read(tmp.data(), tmp.size());
			break;
		}
		case 7:
		{
			const uint16_t length = script.U16();
			std::vector<uint8_t> tmp(length + 1, script.U8());
			sink.Write(tmp.data(), tmp.size());
			break;
		}
		case 8: buffer.ReadTo(sink, 1 + script.U16()); break;
		case 9: buffer.Seek(script.U16()); break;
		case 10: buffer.Reset(); break;
		case 11:
		{
			net::Buffer copy = buffer.Clone();
			copy.Read<uint32_t>();
			buffer.IsAtEnd();
			buffer.GetRemainingBytes();
			break;
		}
		}
	}

	return 0;
}
