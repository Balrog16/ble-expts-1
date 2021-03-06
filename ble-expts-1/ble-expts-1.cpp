// ble-expts-1.cpp : This file contains the 'main' function. Program execution
// begins and ends there.
//
#include <windows.h>
#include <winrt/Windows.Devices.Bluetooth.Advertisement.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Radios.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.Streams.h>

#include "make_guid.h"
#include "siglist.h"
#include <algorithm>
#include <bitset>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>

#include <fstream>

#define GUID_FORMAT                                                            \
  "%08x-%04hx-%04hx-%02hhx%02hhx-%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx"
#define GUID_ARG(guid)                                                         \
  guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1],            \
      guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5],              \
      guid.Data4[6], guid.Data4[7]

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
  return std::string{chars};
}

std::vector<std::string> split(const std::string &str,
                               const std::string &delim) {
  std::vector<std::string> tokens;
  size_t prev = 0, pos = 0;
  do {
    pos = str.find(delim, prev);
    if (pos == std::string::npos)
      pos = str.length();
    std::string token = str.substr(prev, pos - prev);
    if (!token.empty())
      tokens.push_back(token);
    prev = pos + delim.length();
  } while (pos < str.length() && prev < str.length());
  return tokens;
}

std::string pretty_print_macaddr(uint64_t addr) {

  int8_t i;
  uint8_t p;
  std::ostringstream str;
  for (i = 5; i >= 0; i--) {
    p = (uint8_t)(addr >> (CHAR_BIT * i));
    str << std::hex << std::setw(2) << static_cast<int>(p)
        << (i == 0 ? ' ' : ':');
  }
  return str.str();
}

std::vector<uint8_t>
parseManufacturerData(BluetoothLEAdvertisement advertisement) {
  if (advertisement.ManufacturerData().Size() == 0)
    return std::vector<uint8_t>();

  auto manufacturerData = advertisement.ManufacturerData().GetAt(0);
  // FIXME Compat with REG_DWORD_BIG_ENDIAN
  uint8_t *prefix = uint16_t_union{manufacturerData.CompanyId()}.bytes;
  auto result = std::vector<uint8_t>{prefix, prefix + sizeof(uint16_t_union)};

  auto data = to_bytevc(manufacturerData.Data());
  result.insert(result.end(), data.begin(), data.end());

  std::cout << "Company: "
            << ((idToCompany.count(manufacturerData.CompanyId()) > 0)
                    ? idToCompany.at(manufacturerData.CompanyId())
                    : "Not yet in the table --raise a ticket")
            << std::endl;

  return result;
}

bool checkRadio() {

  bool retVal = false;
  using namespace winrt::Windows::Devices::Radios;
  // detect presence of bluetooth radio and to support for BLE
  try {
    std::cout << "Entered bluetooth adapter get method" << std::endl;
    BluetoothAdapter blueAdapter = BluetoothAdapter::GetDefaultAsync().get();
    std::cout << "Get radio handle" << std::endl;
    auto blueRadio = blueAdapter.GetRadioAsync().get();
    std::cout << "Get radio name:";
    auto name = blueRadio.Name().c_str();
    std::wcout << name << std::endl;
    std::cout << "Get radio state" << std::endl;
    auto state = blueRadio.State();
    std::cout << "BLE Radio state (1 - ON, 2- OFF, 3- Disabled, 0 - Unknown) "
                 "and here we have... "
              << static_cast<int>(state) << std::endl;
    if (static_cast<int>(state)) // If the BT radio is ON then,
    {
      // Check if it supports LE
      if (blueAdapter.IsLowEnergySupported()) {
        std::cout << "BLE Support detected" << std::endl;
        // If it does, then query Phy (Can't until it is win 11) :-(
        retVal = true;
      }
    }
  } catch (...) {
    std::cout << "Unable to get bluetooth adapter" << std::endl;
  }
  return retVal;
  ;
}

class bleCentral {

public:
  bleCentral();
  bleCentral(std::string);
  virtual ~bleCentral();
  void StartScan();
  /* void StopScan();
  void DiscoverServicesnChars(uint64_t bleaddress);
  std::vector<uint8_t> ReadChar(std::string servID, std::string charID);
  std::vector<uint8_t> WriteChar(std::string servID, std::string charID,
                                 std::string vWrite);
  void CheckEnableReadNotify(std::string servID, std::string charID);
  void charProperty(GattCharacteristicProperties charProp);*/

private:
  BluetoothLEAdvertisementWatcher bluetoothLEWatcher{nullptr};
  BluetoothLEAdvertisementFilter bleAdvFilterObj{nullptr};
  winrt::event_token bluetoothLEWatcherReceivedToken;
  winrt::event_token bluetoothLEWatcherOnValueChangedToken;

  // Delegate
  void
  BluetoothLEWatcher_Received(BluetoothLEAdvertisementWatcher sender,
                              BluetoothLEAdvertisementReceivedEventArgs args);

  winrt::fire_and_forget ConnectAsync(uint64_t bluetoothAddress);
  // Delegate
  void BluetoothLEDevice_ConnectionStatusChanged(BluetoothLEDevice sender,
                                                 IInspectable args);

  // Delegate
  void OnCharValChanged(GattCharacteristic gattChar,
                        GattValueChangedEventArgs const &eventargs);
  // Connection status
  std::optional<bool> bConnected;
  GattDeviceServicesResult servicesResult{0};
  BluetoothLEDevice leDev{0};
  std::optional<std::string> peripheralName;
};
bleCentral::bleCentral() {}
bleCentral::bleCentral(std::string deviceName) : peripheralName(deviceName) {}
bleCentral::~bleCentral() {}

void bleCentral::StartScan() {
  if (!bluetoothLEWatcher) {

    // Create a BluetoothLEAdverstisementFilter Obj
    bleAdvFilterObj = BluetoothLEAdvertisementFilter();
    // Fill it up with localName if available
    if (peripheralName.has_value())
      bleAdvFilterObj.Advertisement().LocalName(
          winrt::to_hstring(peripheralName.value()));
    // Create the object and register the delegate
    bluetoothLEWatcher = BluetoothLEAdvertisementWatcher(bleAdvFilterObj);
    bluetoothLEWatcher.AllowExtendedAdvertisements(TRUE);
    bluetoothLEWatcherReceivedToken = bluetoothLEWatcher.Received(
        {this, &bleCentral::BluetoothLEWatcher_Received});
  }
  bluetoothLEWatcher.Start(); // Enable scanning
}

void bleCentral::BluetoothLEWatcher_Received(
    BluetoothLEAdvertisementWatcher sender,
    BluetoothLEAdvertisementReceivedEventArgs args) {

  OutputDebugString(
      (L"Received " + winrt::to_hstring(args.BluetoothAddress()) + L"\n")
          .c_str());

  auto manufacturer_data = parseManufacturerData(args.Advertisement());

  std::string sLocalName;
  if (args.Advertisement().LocalName().empty())
    sLocalName = "unknown device";
  else
    sLocalName = winrt::to_string(args.Advertisement().LocalName()).c_str();

  std::vector<std::string> sInfo = {
      ("name " + sLocalName),
      ("device ID " + pretty_print_macaddr(args.BluetoothAddress())),
      ("rssi " + std::to_string(args.RawSignalStrengthInDBm())).c_str()};

  std::cout << "------------------------" << std::endl;
  for (auto &str : sInfo)
    std::cout << str << std::endl;
  std::cout << "------------------------" << std::endl;
}

int main(int argc, char *argv[]) {
  std::cout << "Device to work with... " << argv[1] << std::endl;

  if (checkRadio()) {
    bleCentral bleCentralDevice("Elsa_and_Ana");
    bleCentralDevice.StartScan();
    while (1)
      ;
  }
  
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started:
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add
//   Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project
//   and select the .sln file
