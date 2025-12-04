#include "Reader.h"

uint8_t Reader::read_uint8(void)noexcept{
	if(_ptr >= _end){
		_error();
		return 0;
	}
	return *(_ptr++);
}

uint32_t Reader::read_uint32(void)noexcept{
	if(_ptr + 4 > _end){
		_error();
		return 0;
	}
	auto v = *reinterpret_cast<const uint32_t *>(_ptr);
	_ptr += 4;
	return v;
}

float32_t Reader::read_float32(void)noexcept{
	if(_ptr + 4 > _end){
		_error();
		return 0;
	}
	auto v = *reinterpret_cast<const float32_t *>(_ptr);
	_ptr += 4;
	return v;
}

int64_t Reader::read_int(void)noexcept{
	int64_t r = 0;
	for(int i = 0; i < 70; i += 7){
		auto v = read_uint8();
		r |= static_cast<int64_t>(v & 0x7F) << i;
		if(!(v & 0x80))break;
	}
	return r;
}

Reader Reader::read_data(size_t size)noexcept{
	Reader r(_ptr, size);
	if(_ptr + size > _end){
		_error();
		r._error();
	}else{
		_ptr += size;
	}
	return r;
}
