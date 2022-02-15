#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Bluetooth.Advertisement.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <winrt/Windows.Devices.Radios.h>

#include <iostream>
#include "siglist.h"
#include <memory>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include "make_guid.h"
#include <bitset>

#define GUID_FORMAT "%08x-%04hx-%04hx-%02hhx%02hhx-%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx"
#define GUID_ARG(guid) guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]


union uint16_t_union {
	uint16_t uint16;
	byte bytes[sizeof(uint16_t)];
};

std::vector<uint8_t> to_bytevc(IBuffer buffer) {
	auto reader = DataReader::FromBuffer(buffer);
	auto result = std::vector<uint8_t>(reader.UnconsumedBufferLength());
	reader.ReadBytes(result);
	return result;
}

IBuffer from_bytevc(std::vector<uint8_t> bytes) {
	auto writer = DataWriter();
	writer.WriteBytes(bytes);
	return writer.DetachBuffer();
}

std::string to_hexstring(std::vector<uint8_t> bytes) {
	auto ss = std::stringstream();
	for (auto b : bytes)
		ss << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(b);
	return ss.str();
}

std::string to_uuidstr(winrt::guid guid) {
	char chars[36 + 1];
	sprintf_s(chars, GUID_FORMAT, GUID_ARG(guid));
	return std::string{ chars };
}

