#include "Writer.h"
#include <cstdarg>

bool Writer::_ensure(size_t add)noexcept{
	constexpr size_t capacity_min = 1024;
	size_t required = _data.size() + add;
	if(required > _data.capacity()){
		size_t capacity = _data.capacity();
		if(capacity < capacity_min){
			capacity = capacity_min;
		}
		while(capacity < required){
			capacity <<= 1;
		}
		try{
			_data.reserve(capacity);
		}catch(...){
			return false;
		}
	}
	return true;
}

bool Writer::write_bytes(const uint8_t *data, size_t size)noexcept{
	if(size){
		if(!data){
			return false;
		}
		auto pos = _data.size();
		if(!_ensure(size)){
			return false;
		}
		_data.resize(pos + size);
		std::memcpy(&_data[pos], data, size);
	}
	return true;
}

bool Writer::write_uint(uint64_t value)noexcept{
	uint8_t data[10];
	size_t size = 0;
	for(; size == 0 || (value && size < 10); ++size){
		uint8_t v = value & 0x7F;
		value >>= 7;
		if(value){
			v |= 0x80;
		}
		data[size] = v;
	}
	return write_bytes(data, size);
}

bool Writer::write_uint32(uint32_t value)noexcept{
	return write_bytes(reinterpret_cast<const uint8_t *>(&value), sizeof(value));
}

bool Writer::write_float32(float32_t value)noexcept{
	return write_bytes(reinterpret_cast<const uint8_t *>(&value), sizeof(value));
}

bool Writer::write_data(const data_t &data)noexcept{
	return write_uint(static_cast<uint64_t>(data.size())) && write_bytes(data.data(), data.size());
}

bool Writer::write_string(const string_t &string)noexcept{
	return write_uint(static_cast<uint64_t>(string.size())) && write_bytes(reinterpret_cast<const uint8_t *>(string.c_str()), string.size());
}

bool Writer::fputc(char c)noexcept{
	if(!_ensure(1)){
		return false;
	}
	_data.push_back(static_cast<uint8_t>(c));
	return true;
}

bool Writer::fputs(const char *s)noexcept{
	if(s){
		auto len = std::strlen(s);
		if(!_ensure(len)){
			return false;
		}
		auto pos = _data.size();
		_data.resize(pos + len);
		std::memcpy(&_data[pos], s, len);
	}
	return true;
}

bool Writer::fprintf(const char *fmt, ...)noexcept{
	va_list ap;
	va_start(ap, fmt);
	auto len = std::vsnprintf(nullptr, 0, fmt, ap);
	va_end(ap);

	if(len <= 0){
		return len == 0;
	}
	if(!_ensure(static_cast<size_t>(len + 1))){
		return false;
	}
	size_t pos = _data.size();
	_data.resize(pos + len);
	va_start(ap, fmt);
	std::vsnprintf(reinterpret_cast<char *>(&_data[pos]), len + 1, fmt, ap);
	va_end(ap);
	return true;
}
