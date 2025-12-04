#pragma once

#include <cstdint>
#include <vector>

using float32_t = float;
using data_t = std::vector<uint8_t>;

class Reader{
public:
	Reader(const uint8_t *data=nullptr, size_t size=0)noexcept: _begin(data), _end(data + size), _ptr(data){}
	Reader(const data_t &data)noexcept: _begin(data.data()), _end(data.data() + data.size()), _ptr(data.data()){}

	const uint8_t &operator[](size_t i)const noexcept{return _begin[i];}
  const uint8_t *begin(void)const noexcept{return _begin;}
  const uint8_t *end(void)const noexcept{return _end;}
  bool empty(void)const noexcept{return _begin == _end;}
	size_t size(void)const noexcept{return _end - _begin;}

	bool is_eof(void)const noexcept{return _ptr >= _end;}
	bool is_error(void)const noexcept{return _ptr > _end;}

	uint8_t read_uint8(void)noexcept;
	uint32_t read_uint32(void)noexcept;
	float32_t read_float32(void)noexcept;
	int64_t read_int(void)noexcept;
	Reader read_data(size_t size)noexcept;

private:
	const uint8_t *_begin{nullptr};
	const uint8_t *_end{nullptr};
	const uint8_t *_ptr{nullptr};
	void _error(void)noexcept{_ptr = _end + 1;}
};
