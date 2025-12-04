#include "Dtype.h"
#include "Util.h"
#include <algorithm>

bool Dtype::analyze(Reader r, const id_t &parent_id)noexcept{
	id_t path;
	std::vector<id_t> ids;
	while(!r.is_eof()){
		auto id = r.read_int();
		auto type = static_cast<CodeType>(id & 7);
		id >>= 3;
		try{
			path = parent_id;
			path.push_back(id);
			ids.push_back(path);
		}catch(...){
			return false;
		}
		auto node = const_cast<Node *>(get(path));
		if(!node){
			try{
				_nodes[path] = {};
				node = &_nodes[path];
			}catch(...){
				return false;
			}
			node->_is_int32 = true;
			node->_is_int64 = true;
			node->_is_float32 = true;
			node->_is_float64 = true;
			node->_is_object = true;
			node->_is_string = true;
			node->_is_data = true;
		}
		++node->_reference_count;
		switch(type){
		case CodeType::Int:
			{
				auto v = r.read_int();
				node->_is_float32 = false;
				node->_is_float64 = false;
				node->_is_object = false;
				node->_is_string = false;
				node->_is_data = false;
				if(v > 0xFFFFFFFFll){
					node->_is_int32 = false;
				}else if(v == 0xFFFFFFFFll){
					node->_is_int64 = false;
				}
			}
			break;
		case CodeType::Data:
			{
        auto v = r.read_data(r.read_int());
				node->_is_int32 = false;
				node->_is_int64 = false;
				node->_is_float32 = false;
				node->_is_float64 = false;
				if(node->_is_object){
					if(!_is_error(v)){
						if(v.size()){
							node->_is_string = false;
							node->_is_data = false;
						}
						if(!analyze(v, path)){
							return false;
						}
						break;
					}
					node->_is_object = false;
					node->_is_string = true;
					node->_is_data = true;
				}
				auto size = v.size();
				node->_data_size = size;
				size = size > sizeof(node->_data) ? sizeof(node->_data) : size;
				memcpy_s(node->_data, sizeof(node->_data), v.begin(), size);
				if(node->_is_string && !is_valid_utf8(v.begin(), v.size())){
					node->_is_string = false;
				}
			}
			break;
		case CodeType::Float32:
			{
				auto v = r.read_float32();
				node->_is_int32 = false;
				node->_is_int64 = false;
				node->_is_float64 = false;
				node->_is_object = false;
				node->_is_string = false;
				node->_is_data = false;
			}
			break;
		default:
			return false;
		}
	}
	for(const auto &id : ids){
		auto &node = _nodes[id];
		node._reference_count = node._reference_count < 2 ? 0 : 2;
	}
	return !r.is_error();
}

bool Dtype::_is_error(Reader r)const noexcept{
	while(!r.is_eof()){
		auto id = r.read_int();
		auto type = static_cast<uint8_t>(id) & 7;
		id >>= 3;
		//if(id >= 0x10000){
		//	return true;
		//}
		switch(type){
		case 0:
			r.read_int();
			break;
		case 2:
			r.read_data(r.read_int());
			break;
		case 5:
			r.read_float32();
			break;
		default:
			return true;
		}
	}
	return r.is_error();
}

bool Dtype::read_csv(const string_t &string)noexcept{

	_nodes.clear();
	//_is_update = false;

	for(char *s = const_cast<char *>(string.c_str()), *n; *s; s = n){
		n = s;
		while(*n && *n != '\n' && *n != '\r')++n;
		if(*n){
			*(n++) = '\0';
		}
		if(*s == 0 || *s == '#'){
			continue;
		}
		auto col1 = s;
		while(*s && *s != ',')++s;
		if(*s){
			*(s++) = '\0';
		}
		auto col2 = s;
		while(*s && *s != ',')++s;
		if(*s){
			*(s++) = '\0';
		}
		auto col3 = s;
		while(*s && *s != ',')++s;
		if(*s){
			*(s++) = '\0';
		}
		auto col4 = s;
		Node node;
		id_t path;
		{
			uint64_t v = 0;
			bool has = false;
			for(; *col1; ++col1){
				if(*col1 == '/'){
					if(has){
						path.push_back(v);
						v = 0;
						has = false;
					}
				}else if(*col1 >= '0' && *col1 <= '9'){
					has = true;
					v = v * 10 + *col1 - '0';
				}
			}
			if(has){
				path.push_back(v);
			}
		}
		node._reference_count = (*col2 == '*') ? 2 : 0;
		if(strcmp(col3, "int") == 0){
			node._is_int32 = true;
			node._is_int64 = true;
		}else if(strcmp(col3,"int32") == 0){
			node._is_int32 = true;
		}else if(strcmp(col3,"int64") == 0){
			node._is_int64 = true;
		}else if(strcmp(col3, "float") == 0){
			node._is_float32 = true;
			node._is_float64 = true;
		}else if(strcmp(col3, "float32") == 0){
			node._is_float32 = true;
		}else if(strcmp(col3, "float64") == 0){
			node._is_float64 = true;
		}else if(strcmp(col3, "unknown") == 0){
			node._is_object = true;
			node._is_string = true;
			node._is_data = true;
		}else if(strcmp(col3, "object") == 0){
			node._is_object = true;
		}else if(strcmp(col3, "string") == 0){
			node._is_string = true;
		}else if(strcmp(col3, "data") == 0){
			node._is_data = true;
		}
		try{
			_nodes.emplace(path, node);
		}catch(...){
			return false;
		}
	}

	return true;
}
bool Dtype::write_csv(Writer &w)const noexcept{

	std::vector<std::pair<id_t, Node>> nodes;
	try{
		nodes.assign(_nodes.begin(), _nodes.end());
	}catch(...){
		return false;
	}
	std::sort(nodes.begin(), nodes.end(), [](const auto &a, const auto &b)noexcept{return a.first < b.first;});

	bool r = true;
	r &= w.fputs("# Path, Multiple keys, Type, Details\r\n");
  for(const auto &node : nodes){
		bool is_not_first = false;
		for(const auto &v : node.first){
			if(is_not_first){
				r &= w.fputc('/');
			}else{
				is_not_first = true;
			}
      r &= w.fprintf("%lld", v);
		}
		r &= w.fputc(',');
		auto &n = node.second;
		if(n._reference_count >= 2){
			r &= w.fputc('*');
		}
		r &= w.fputc(',');
		if(n.is_int_unknown()){
			r &= w.fputs("int");
		}else if(n.is_int32()){
			r &= w.fputs("int32");
		}else if(n.is_int64()){
			r &= w.fputs("int64");
		}else if(n.is_float_unknown()){
			r &= w.fputs("float");
		}else if(n.is_float32()){
			r &= w.fputs("float32");
		}else if(n.is_float64()){
			r &= w.fputs("float64");
		}else if(n.is_data_unknown()){
			r &= w.fputs("unknown,EmptyData");
		}else if(n.is_object()){
			r &= w.fputs("object");
		}else if(n.is_string()){
			r &= w.fputs("string");
		}else if(n.is_data()){
			r &= w.fputs("data");
			auto size = n._data_size;
			if(size){
				r &= w.fprintf(",size=0x%llX:", size);
				size = size > sizeof(n._data) ? sizeof(n._data) : size;
				for(const auto *s = n._data; size; --size, ++s){
					r &= w.fprintf(" %02X", *s);
				}
			}
		}
		r &= w.fputs("\r\n");
  }

	return r;
}
