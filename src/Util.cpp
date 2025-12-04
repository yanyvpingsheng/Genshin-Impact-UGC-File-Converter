#include "Util.h"
#include <stdio.h>

bool LoadFile(string_t &buf, const char *path)noexcept{

	FILE *fp = nullptr;
	if(fopen_s(&fp, path, "rb") || fp == nullptr){
		return false;
	}

	fseek(fp, 0, SEEK_END);
	auto tell = ftell(fp);
	if(tell > 0){
		auto size = static_cast<size_t>(tell);
		try{
			buf.resize(size);
		}catch(...){
			fclose(fp);
			return false;
		}
		fseek(fp, 0, SEEK_SET);
		if(fread(&buf[0], 1, size, fp) != size){
			fclose(fp);
			return false;
		}
	}

	fclose(fp);

	return true;
}

bool LoadFile(data_t &buf, const char *path)noexcept{

	FILE *fp = nullptr;
	if(fopen_s(&fp, path, "rb") || fp == nullptr){
		return false;
	}

	fseek(fp, 0, SEEK_END);
	auto tell = ftell(fp);
	if(tell > 0){
		auto size = static_cast<size_t>(tell);
		try{
			buf.resize(size);
		}catch(...){
			fclose(fp);
			return false;
		}
		fseek(fp, 0, SEEK_SET);
		if(fread(&buf[0], 1, size, fp) != size){
			fclose(fp);
			return false;
		}
	}

	fclose(fp);

	return true;
}

bool SaveFile(const char *path, const data_t &data)noexcept{

	FILE *fp = nullptr;
	if(fopen_s(&fp, path, "wb") || fp == nullptr){
		return false;
	}

	fwrite(data.data(), 1, data.size(), fp);

	fclose(fp);

	return true;
}

string_t Base64Encode(const data_t &data)noexcept{
	static const char *ENCODE_TABLE = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	std::string out;
	try{
		out.reserve((data.size() + 2) / 3 * 4);
	}catch(...){
		return "";
	}

	size_t i = 0;
	for(; i + 2 < data.size(); i += 3){
		uint32_t n = (data[i] << 16) | (data[i + 1] << 8) | data[i + 2];
		out.push_back(ENCODE_TABLE[(n >> 18) & 63]);
		out.push_back(ENCODE_TABLE[(n >> 12) & 63]);
		out.push_back(ENCODE_TABLE[(n >> 6)  & 63]);
		out.push_back(ENCODE_TABLE[n & 63]);
	}

	if(i + 1 == data.size()){
		uint32_t n = (data[i] << 16);
		out.push_back(ENCODE_TABLE[(n >> 18) & 63]);
		out.push_back(ENCODE_TABLE[(n >> 12) & 63]);
		out.push_back('=');
		out.push_back('=');
	}else if (i + 2 == data.size()){
		uint32_t n = (data[i] << 16) | (data[i + 1] << 8);
		out.push_back(ENCODE_TABLE[(n >> 18) & 63]);
		out.push_back(ENCODE_TABLE[(n >> 12) & 63]);
		out.push_back(ENCODE_TABLE[(n >> 6)  & 63]);
		out.push_back('=');
	}

	return out;
}

data_t Base64Decode(const string_t &string)noexcept{

	static const int DECODE_TABLE[256] = {
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
		52,53,54,55,56,57,58,59,60,61,-1,-1,-1, 0,-1,-1,
		-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
		15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
		-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
		41,42,43,44,45,46,47,48,49,50,51
	};

	data_t out;
	try{
		out.reserve(string.size() * 3 / 4);
	}catch(...){
		return {};
	}

	uint32_t buffer = 0;
	int bits_collected = 0;

	for(uint8_t c : string){

		if(std::isspace(c)){
			continue;
		}
		if(c == '='){
			break;
		}

		int val = DECODE_TABLE[c];
		if(val == -1){
			continue;
		}

		buffer = (buffer << 6) | val;
		bits_collected += 6;

		if(bits_collected >= 8){
			bits_collected -= 8;
			out.push_back(static_cast<uint8_t>((buffer >> bits_collected) & 0xFF));
		}

	}

	return out;
}

bool is_valid_utf8(const uint8_t *data, size_t len)noexcept{

	static constexpr uint8_t is_not_string[128] = {
		1,1,1,1,1,1,1,1,1,0,0,1,1,0,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	};

	for(size_t i = 0; i < len; ++i){
		uint8_t byte1 = data[i];

		if(byte1 <= 0x7F){
			if(is_not_string[byte1]){
				return false;
			}
			continue;
		}

		size_t need = 0;
		uint32_t codepoint = 0;
		if((byte1 & 0xE0) == 0xC0){
			need = 1;
			codepoint = byte1 & 0x1F;
		}else if((byte1 & 0xF0) == 0xE0){
			need = 2;
			codepoint = byte1 & 0x0F;
		}else if((byte1 & 0xF8) == 0xF0){
			need = 3;
			codepoint = byte1 & 0x07;
		}else{
			return false;
		}

		if(i + need >= len){
			return false;
		}

		for(size_t k = 1; k <= need; ++k){
			uint8_t b = data[i + k];
			if((b & 0xC0) != 0x80){
				return false;
			}
			codepoint = (codepoint << 6) | (b & 0x3F);
		}

		if(need == 1){
			if(codepoint < 0x80)return false;
		}else if(need == 2){
			if(codepoint < 0x800)return false;
			if(codepoint >= 0xD800 && codepoint <= 0xDFFF)return false;
		}else if(need == 3){
			if(codepoint < 0x10000)return false;
			if(codepoint > 0x10FFFF)return false;
		}

		i += need;
	}

	return true;
}
