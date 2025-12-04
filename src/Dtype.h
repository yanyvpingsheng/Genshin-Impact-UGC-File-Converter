#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include "Reader.h"
#include "Writer.h"

using string_t = std::string;

class Dtype{
public:
	Dtype()noexcept{}

	class Node{
	public:
		Node()noexcept{}
		bool is_multiple_keys()const noexcept{return _reference_count >= 2;}
		bool is_int32()const noexcept{return _is_int32;}
		bool is_int64()const noexcept{return _is_int64;}
		bool is_float32()const noexcept{return _is_float32;}
		bool is_float64()const noexcept{return _is_float64;}
		bool is_object()const noexcept{return _is_object;}
		bool is_string()const noexcept{return _is_string;}
		bool is_data()const noexcept{return _is_data;}
		bool is_int_unknown()const noexcept{return _is_int32 && _is_int64;}
		bool is_float_unknown()const noexcept{return _is_float32 && _is_float64;}
		bool is_data_unknown()const noexcept{return _is_object && _is_string && _is_data;}
	private:
		bool _is_int32{};
		bool _is_int64{};
		bool _is_float32{};
		bool _is_float64{};
		bool _is_object{};
		bool _is_string{};
		bool _is_data{};
		uint32_t _reference_count{};
		uint8_t _data[16]{};
		size_t _data_size{};
		friend class Dtype;
	};

	using id_t = std::vector<uint64_t>;

	const Node *get(const id_t &id)const noexcept{
		if(auto it = _nodes.find(id); it != _nodes.end()){
			return &it->second;
		}
		return nullptr;
	}

	//bool is_update(void)const noexcept{return _is_update;}
	bool analyze(Reader r, const id_t &parent_id={})noexcept;

	bool read_csv(const string_t &string)noexcept;
	bool write_csv(Writer &w)const noexcept;

private:
	enum class CodeType{
		Int = 0,
		// 1 不明
		Data = 2,
		// 3 不明
		// 4 不明
		Float32 = 5,
		// 6 不明
		// 7 不明
	};
	struct id_hash{
		size_t operator()(const id_t &v)const noexcept{
			size_t h = 0;
			for(auto &x : v){
				h ^= std::hash<uint64_t>{}(x) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
			}
			return h;
		}
	};
	struct id_equal{
		bool operator()(const id_t &a, const id_t &b)const noexcept{return a == b;}
	};
	std::unordered_map<id_t, Node, id_hash, id_equal> _nodes{};
	//bool _is_update{false};
	bool _is_error(Reader r)const noexcept;
};
