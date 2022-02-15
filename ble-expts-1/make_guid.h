#include <assert.h>
#include <exception>
#include <string_view>

using namespace std::literals;

struct winrtguid {
  uint32_t Data1;
  uint16_t Data2;
  uint16_t Data3;
  uint8_t Data4[8];
};

constexpr bool operator==(winrtguid const &left,
                          winrtguid const &right) noexcept {
  return left.Data1 == right.Data1 && left.Data2 == right.Data2 &&
         left.Data3 == right.Data3 && left.Data4[0] == right.Data4[0] &&
         left.Data4[1] == right.Data4[1] && left.Data4[2] == right.Data4[2] &&
         left.Data4[3] == right.Data4[3] && left.Data4[4] == right.Data4[4] &&
         left.Data4[5] == right.Data4[5] && left.Data4[6] == right.Data4[6] &&
         left.Data4[7] == right.Data4[7];
}

constexpr uint32_t to_uint(char const value) noexcept {
  if (value >= '0' && value <= '9') {
    return value - '0';
  }

  if (value >= 'A' && value <= 'F') {
    return 10 + value - 'A';
  }

  if (value >= 'a' && value <= 'f') {
    return 10 + value - 'a';
  }

  std::terminate();
}

constexpr winrtguid make_guid(std::string_view const &value) noexcept {
  if (value.size() != 36 || value[8] != '-' || value[13] != '-' ||
      value[18] != '-' || value[23] != '-') {
    std::terminate();
  }

  return {
      ((to_uint(value[0]) * 16 + to_uint(value[1])) << 24) +
          ((to_uint(value[2]) * 16 + to_uint(value[3])) << 16) +
          ((to_uint(value[4]) * 16 + to_uint(value[5])) << 8) +
          (to_uint(value[6]) * 16 + to_uint(value[7])),

      static_cast<uint16_t>(
          ((to_uint(value[9]) * 16 + to_uint(value[10])) << 8) +
          (to_uint(value[11]) * 16 + to_uint(value[12]))),

      static_cast<uint16_t>(
          ((to_uint(value[14]) * 16 + to_uint(value[15])) << 8) +
          (to_uint(value[16]) * 16 + to_uint(value[17]))),

      {
          static_cast<uint8_t>(to_uint(value[19]) * 16 + to_uint(value[20])),
          static_cast<uint8_t>(to_uint(value[21]) * 16 + to_uint(value[22])),

          static_cast<uint8_t>(to_uint(value[24]) * 16 + to_uint(value[25])),
          static_cast<uint8_t>(to_uint(value[26]) * 16 + to_uint(value[27])),
          static_cast<uint8_t>(to_uint(value[28]) * 16 + to_uint(value[29])),
          static_cast<uint8_t>(to_uint(value[30]) * 16 + to_uint(value[31])),
          static_cast<uint8_t>(to_uint(value[32]) * 16 + to_uint(value[33])),
          static_cast<uint8_t>(to_uint(value[34]) * 16 + to_uint(value[35])),
      }};
}
// namespace guidkerr
/*static constexpr winrtguid a
{ 0x8aa90cad, 0xfed1, 0x4c54, { 0x89, 0xdb, 0x9b, 0x75, 0x22, 0xd8, 0xaa, 0x92 }
};

static constexpr winrtguid b
{ make_guid("8AA90CAD-fed1-4c54-89db-9B7522D8AA92"sv) };

static_assert(a == b);

int main()
{
    assert(a == b);
}*/