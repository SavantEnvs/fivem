#include <StdInc.h>

#include <FormData.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
	if (size < 1)
	{
		return -1;
	}

	const std::string_view input(reinterpret_cast<const char*>(data + 1), size - 1);

	if (data[0] & 1)
	{
		std::string out;
		net::UrlDecode(input, out, (data[0] & 2) != 0);
	}
	else
	{
		net::DecodeFormData(input);
	}

	return 0;
}
