#pragma once

#include <cstdint>
#include <string>
#include <vector>

using string_t = std::string;
using data_t = std::vector<uint8_t>;

bool LoadFile(string_t &buf, const char *path)noexcept;
bool LoadFile(data_t &buf, const char *path)noexcept;
bool SaveFile(const char *path, const data_t &data)noexcept;

string_t Base64Encode(const data_t &data)noexcept;
data_t Base64Decode(const string_t &string)noexcept;

bool is_valid_utf8(const uint8_t *data, size_t len)noexcept;

static inline constexpr uint32_t bswap32(uint32_t v)noexcept{return (v << 24) | ((v & 0xFF00) << 8) | ((v & 0xFF0000) >> 8) | (v >> 24);}
