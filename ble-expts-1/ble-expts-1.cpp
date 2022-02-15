// ble-expts-1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
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
#include <iostream>
#include "make_guid.h"
#include <bitset>

#include <fstream>

#define GUID_FORMAT "%08x-%04hx-%04hx-%02hhx%02hhx-%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx"
#define GUID_ARG(guid) guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Storage::Streams;
using namespace winrt::Windows::Devices::Bluetooth;
using namespace winrt::Windows::Devices::Bluetooth::Advertisement;
using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;


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

std::vector<std::string> split(const std::string& str, const std::string& delim)
{
	std::vector<std::string> tokens;
	size_t prev = 0, pos = 0;
	do
	{
		pos = str.find(delim, prev);
		if (pos == std::string::npos) pos = str.length();
		std::string token = str.substr(prev, pos - prev);
		if (!token.empty()) tokens.push_back(token);
		prev = pos + delim.length();
	} while (pos < str.length() && prev < str.length());
	return tokens;
}

class bleCentral {

public:
	bleCentral();
	virtual ~bleCentral();
	void StartScan();
	void StopScan();
	void DiscoverServicesnChars(uint64_t bleaddress);
	std::vector<uint8_t> ReadChar(std::string servID, std::string charID);
	std::vector<uint8_t> WriteChar(std::string servID, std::string charID, std::string vWrite);
	void CheckEnableReadNotify(std::string servID, std::string charID);
	void charProperty(GattCharacteristicProperties charProp);


private:

	BluetoothLEAdvertisementWatcher bluetoothLEWatcher{ nullptr };
	BluetoothLEAdvertisementFilter bleAdvFilterObj{ nullptr };
	winrt::event_token bluetoothLEWatcherReceivedToken;
	winrt::event_token bluetoothLEWatcherOnValueChangedToken;

	// Delegate
	void BluetoothLEWatcher_Received(BluetoothLEAdvertisementWatcher sender, BluetoothLEAdvertisementReceivedEventArgs args);

	winrt::fire_and_forget ConnectAsync(uint64_t bluetoothAddress);
	// Delegate
	void BluetoothLEDevice_ConnectionStatusChanged(BluetoothLEDevice sender, IInspectable args);

	//Delegate 
	void OnCharValChanged(GattCharacteristic gattChar,
		GattValueChangedEventArgs const& eventargs);
	// Connection status
	std::optional<bool> bConnected;
	GattDeviceServicesResult servicesResult{ 0 };
	BluetoothLEDevice leDev{ 0 };

};

bool checkRadio() {

	bool retVal = false;
	using namespace winrt::Windows::Devices::Radios;
    // detect presence of bluetooth radio and to support for BLE
	try
	{
		std::cout << "Entered bluetooth adapter get method" << std::endl;
		BluetoothAdapter blueAdapter = BluetoothAdapter::GetDefaultAsync().get();
		std::cout << "Get radio handle" << std::endl;
		auto blueRadio = blueAdapter.GetRadioAsync().get();
		std::cout << "Get radio name:";
		auto name = blueRadio.Name().c_str();
		std::wcout << name << std::endl;
		std::cout << "Get radio state" << std::endl;
		auto state = blueRadio.State();
		std::cout << "BLE Radio state (1 - ON, 2- OFF, 3- Disabled, 0 - Unknown) and here we have... " << static_cast<int>(state) << std::endl;
		if (static_cast<int>(state)) // If the BT radio is ON then, 
		{
			//Check if it supports LE
			if (blueAdapter.IsLowEnergySupported())
			{
				std::cout << "BLE Support detected" << std::endl;
				//If it does, then query Phy (Can't until it is win 11) :-(
				retVal = true; 
			}

		}
	}
	catch (...)
	{
		std::cout << "Unable to get bluetooth adapter" << std::endl;
	}
	return retVal;;
}


int main(int argc, char* argv[])
{
    std::cout << "Device to work with... " << argv[1] << std::endl;
    /*if (checkRadio())
        std::cout << "BLE Radio found!!" << std::endl;*/
    
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
