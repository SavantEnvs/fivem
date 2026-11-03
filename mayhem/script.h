#pragma once

#include <cstddef>
#include <cstdint>

namespace mayhem
{
// Consumes the tail of a fuzz input as a stream of operation selectors / operands.
class Script
{
	const uint8_t* m_data;
	size_t m_size;

public:
	Script(const uint8_t* data, size_t size)
		: m_data(data), m_size(size)
	{
	}

	bool Done() const
	{
		return m_size == 0;
	}

	uint8_t U8()
	{
		if (m_size == 0)
		{
			return 0;
		}

		m_size--;
		return *m_data++;
	}

	uint16_t U16()
	{
		uint16_t lo = U8();
		return lo | (uint16_t(U8()) << 8);
	}

	uint32_t U32()
	{
		uint32_t lo = U16();
		return lo | (uint32_t(U16()) << 16);
	}
};
}
