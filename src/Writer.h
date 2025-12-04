#pragma once

#include <cstdint>
#include <vector>
#include <string>

using float32_t = float;
using data_t = std::vector<uint8_t>;
using string_t = std::string;

class Writer{
public:
	Writer()noexcept{}

	data_t &data()noexcept{return _data;}

	bool write_bytes(const uint8_t *data, size_t size)noexcept;
	bool write_uint(uint64_t value)noexcept;
	bool write_uint32(uint32_t value)noexcept;
	bool write_float32(float32_t value)noexcept;
	bool write_data(const data_t &data)noexcept;
	bool write_string(const string_t &string)noexcept;

	bool fputc(char c)noexcept;
	bool fputs(const char *s)noexcept;
	bool fprintf(const char *fmt, ...)noexcept;

private:
	bool _ensure(size_t add)noexcept;
	data_t _data{};
};
