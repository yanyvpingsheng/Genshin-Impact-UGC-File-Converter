#include "Tson.h"
#include "Util.h"
#include <algorithm>

Tson::FileType Tson::_get_filetype(const char *ext)noexcept{
	for(const auto &e : _filetype_table){
		if(e.name == ext){
			return e.type;
		}
	}
	return FileType::Unknown;
}
Tson::DirType Tson::_get_dirtype(const char *s)noexcept{
	for(const auto &e : _dirtype_table){
		if(e.name == s){
			return e.type;
		}
	}
	return DirType::Unknown;
}
std::string_view Tson::_to_string(FileType type)noexcept{
	for(const auto &e : _filetype_table){
		if(e.type == type){
			return e.name;
		}
	}
	return "Unknown";
}
std::string_view Tson::_to_string(DirType type)noexcept{
	for(const auto &e : _dirtype_table){
		if(e.type == type){
			return e.name;
		}
	}
	return "Unknown";
}

bool Tson::preload_btson(const char *path_btson)noexcept{
	const char *last_sep1 = strrchr(path_btson, '/');
	const char *last_sep2 = strrchr(path_btson, '\\');
	const char *last_sep = last_sep1 > last_sep2 ? last_sep1 : last_sep2;
	const char *ext = strrchr(path_btson, '.');
	if(!ext || (last_sep && ext < last_sep)){
		return false;
	}
	++ext;
	_filetype = _get_filetype(ext);
	if(_filetype == FileType::Unknown){
		return false;
	}else if(_filetype == FileType::mihoyobin){
		if(!last_sep){
			return false;
		}
		string_t dir;
		try{
			dir.assign(path_btson, last_sep + 1);
		}catch(...){
			return false;
		}
		for(char &c : dir){
			if(c == '\\'){
				c = '/';
			}
		}
		static constexpr struct{std::string_view pattern;DirType type;} table[] = {
			{"/Json/Beyond/BeyondGlobal/", DirType::Beyond_BeyondGlobal},
			{"/Json/Beyond/Node/", DirType::Beyond_Node},
			{"/Json/Beyond/Official/Blueprint/OfficialCompoundNode/", DirType::Beyond_Official_Blueprint_OfficialCompoundNode},
			{"/Json/Beyond/Official/OfficialPrefab/", DirType::Beyond_Official_OfficialPrefab},
			{"/Json/Beyond/Official/Struct/", DirType::Beyond_Official_Struct},
			{"/Json/Config/JsonConfig/ShortCutKey/", DirType::Config_JsonConfig_ShortCutKey},
			{"/Json/Config/JsonConfig/SynonymsLibrary/", DirType::Config_JsonConfig_SynonymsLibrary},
			{"/Json/TextMap/", DirType::TextMap},
		};
		auto sv = std::string_view(dir);
		_dirtype = DirType::Unknown;
		for(const auto &e : table){
			if(sv.find(e.pattern) != std::string_view::npos){
				_dirtype = e.type;
				break;
			}
		}
	}
	return true;
}

void Tson::_crypt(data_t &data)noexcept{
	for(auto &v : data){
		v ^= 0xE5;
	}
}

bool Tson::_load_btson(Json::Node &json_parent, Reader &r, const Dtype::id_t &dtype_parent_id)noexcept{
	Dtype::id_t path;
	json_parent.type(Json::Node::Type::Object);
	auto &parent = *const_cast<Json::Node::object_t *>(json_parent.as_object());
	while(!r.is_eof()){
		auto id = r.read_int();
		auto type = static_cast<CodeType>(id & 7);
		id >>= 3;
		try{
			path = dtype_parent_id;
			path.push_back(id);
		}catch(...){
			return false;
		}
		auto dtype = _dtype.get(path);
		if(!dtype){
			return false;
		}
		Json::Node node;
		switch(type){
		case CodeType::Int:
			{
				auto v = r.read_int();
				if(dtype->is_int_unknown() || dtype->is_int64()){
					node = Json::Node(v);
				}else if(dtype->is_int32()){
					node = Json::Node(static_cast<Json::Node::int64_t>(static_cast<int32_t>(v)));
				}else{
					return false;
				}
			}
			break;
		case CodeType::Data:
			{
        auto v = r.read_data(r.read_int());
				if(dtype->is_data_unknown()){
					try{
						//node = Json::Node("base64:" + Base64Encode(data_t(v.begin(), v.end())));
						node = Json::Node(Json::Node::string_t{});
					}catch(...){
						return false;
					}
				}else if(dtype->is_string()){
					try{
						node = Json::Node("string:" + Json::Node::string_t(reinterpret_cast<const char *>(v.begin()), v.size()));
					}catch(...){
						return false;
					}
				}else if(dtype->is_data()){
					try{
						node = Json::Node("base64:" + Base64Encode(data_t(v.begin(), v.end())));
					}catch(...){
						return false;
					}
				}else if(dtype->is_object()){
					if(!_load_btson(node, v, path)){
						return false;
					}
				}else{
					return false;
				}
			}
			break;
		case CodeType::Float32:
			{
				auto v = r.read_float32();
				if(dtype->is_float32()){
					node = Json::Node(static_cast<Json::Node::float64_t>(v));
				}else{
					return false;
				}
			}
			break;
		default:
			return false;
		}
		try{
			auto key = std::to_string(id);
			if(dtype->is_multiple_keys()){
				auto n = json_parent.get(key);
				if(!n){
					parent[key] = Json::Node(Json::Node::array_t{});
					n = &parent[key];
				}
				auto &list = *const_cast<Json::Node::array_t *>(n->as_array());
				list.push_back(std::move(node));
			}else{
				parent[key] = std::move(node);
			}
		}catch(...){
			return false;
		}
	}

	return true;
}

bool Tson::_save_btson(Writer &w, const Json::Node &json_node, const Dtype::id_t &dtype_parent_id)const noexcept{

	Dtype::id_t path;
	uint64_t id = 0;
	bool r = true;
	auto write_key_int = [&](void)noexcept -> bool {
		return w.write_uint((id << 3) | static_cast<uint64_t>(CodeType::Int));
	};
	auto write_key_data = [&](void)noexcept -> bool {
		return w.write_uint((id << 3) | static_cast<uint64_t>(CodeType::Data));
	};
	auto write_key_float32 = [&](void)noexcept -> bool {
		return w.write_uint((id << 3) | static_cast<uint64_t>(CodeType::Float32));
	};

	auto object = json_node.as_object();
	if(!object){
		return false;
	}

	std::vector<std::pair<uint64_t, string_t>> nodes;
	try{
		for(auto &child : *object){
			nodes.push_back({strtoull(child.first.c_str(), nullptr, 10), child.first});
		}
	}catch(...){
		return false;
	}
	std::sort(nodes.begin(), nodes.end(), [](const auto &a, const auto &b)noexcept{
		return a.first < b.first;
	});
	for(auto &n : nodes){
		auto node = object->find(n.second);
		if(node == object->end()){
			return false;
		}
		try{
			id = n.first;
			path = dtype_parent_id;
			path.push_back(id);
		}catch(...){
			return false;
		}
		auto dtype = _dtype.get(path);
		if(dtype){
			if(dtype->is_multiple_keys()){
				auto array = node->second.as_array();
				if(!array){
					return false;
				}
				if(dtype->is_int_unknown()){
					for(auto &child : *array){
						r &= write_key_int();
						r &= w.write_uint(static_cast<uint64_t>(child.as_int64()));
					}
				}else if(dtype->is_int32()){
					for(auto &child : *array){
						r &= write_key_int();
						r &= w.write_uint(static_cast<uint32_t>(child.as_int64()));
					}
				}else if(dtype->is_int64()){
					for(auto &child : *array){
						r &= write_key_int();
						r &= w.write_uint(static_cast<uint64_t>(child.as_int64()));
					}
				}else if(dtype->is_float_unknown()){
					r = false;
				}else if(dtype->is_float32()){
					for(auto &child : *array){
						r &= write_key_float32();
						r &= w.write_float32(static_cast<float32_t>(child.as_float64()));
					}
				}else if(dtype->is_float64()){
					r = false;
				}else if(dtype->is_data_unknown()){
					for(auto &child : *array){
						constexpr const char *prefix = "base64:";
						constexpr size_t prefix_len = 7;
						auto s = child.as_string();
						if(s.size() >= prefix_len && s.compare(0, prefix_len, prefix) == 0){
							s.erase(0, prefix_len);
						}
						r &= write_key_data();
						r &= w.write_data(Base64Decode(s));
					}
				}else if(dtype->is_object()){
					for(auto &child : *array){
						Writer w2;
						r &= _save_btson(w2, child, path);
						r &= write_key_data();
						r &= w.write_data(w2.data());
					}
				}else if(dtype->is_string()){
					for(auto &child : *array){
						constexpr const char *prefix = "string:";
						constexpr size_t prefix_len = 7;
						auto s = child.as_string();
						if(s.size() >= prefix_len && s.compare(0, prefix_len, prefix) == 0){
							s.erase(0, prefix_len);
						}
						r &= write_key_data();
						r &= w.write_string(s);
					}
				}else if(dtype->is_data()){
					for(auto &child : *array){
						constexpr const char *prefix = "base64:";
						constexpr size_t prefix_len = 7;
						auto s = child.as_string();
						if(s.size() >= prefix_len && s.compare(0, prefix_len, prefix) == 0){
							s.erase(0, prefix_len);
						}
						r &= write_key_data();
						r &= w.write_data(Base64Decode(s));
					}
				}else{
					r = false;
				}
			}else{
				if(dtype->is_int_unknown()){
					r &= write_key_int();
					r &= w.write_uint(static_cast<uint64_t>(node->second.as_int64()));
				}else if(dtype->is_int32()){
					r &= write_key_int();
					r &= w.write_uint(static_cast<uint32_t>(node->second.as_int64()));
				}else if(dtype->is_int64()){
					r &= write_key_int();
					r &= w.write_uint(static_cast<uint64_t>(node->second.as_int64()));
				}else if(dtype->is_float_unknown()){
					r = false;
				}else if(dtype->is_float32()){
					r &= write_key_float32();
					r &= w.write_float32(static_cast<float32_t>(node->second.as_float64()));
				}else if(dtype->is_float64()){
					r = false;
				}else if(dtype->is_data_unknown()){
					constexpr const char *prefix = "base64:";
					constexpr size_t prefix_len = 7;
					auto s = node->second.as_string();
					if(s.size() >= prefix_len && s.compare(0, prefix_len, prefix) == 0){
						s.erase(0, prefix_len);
					}
					r &= write_key_data();
					r &= w.write_data(Base64Decode(s));
				}else if(dtype->is_object()){
					Writer w2;
					r &= _save_btson(w2, node->second, path);
					r &= write_key_data();
					r &= w.write_data(w2.data());
				}else if(dtype->is_string()){
					constexpr const char *prefix = "string:";
					constexpr size_t prefix_len = 7;
					auto s = node->second.as_string();
					if(s.size() >= prefix_len && s.compare(0, prefix_len, prefix) == 0){
						s.erase(0, prefix_len);
					}
					r &= write_key_data();
					r &= w.write_string(s);
				}else if(dtype->is_data()){
					constexpr const char *prefix = "base64:";
					constexpr size_t prefix_len = 7;
					auto s = node->second.as_string();
					if(s.size() >= prefix_len && s.compare(0, prefix_len, prefix) == 0){
						s.erase(0, prefix_len);
					}
					r &= write_key_data();
					r &= w.write_data(Base64Decode(s));
				}else{
					r = false;
				}
			}
		}else{
			r = false;
		}
		if(!r){
			return false;
		}
	}

	return true;
}

bool Tson::load_btson(const char *path_btson)noexcept{

	data_t buf;
	if(!LoadFile(buf, path_btson)){
		return false;
	}

	auto r = Reader(buf);
	switch(_filetype){
	case FileType::gia:
	case FileType::gil:
		{
			if(r.size() < 0x14){
				return false;
			}
			auto data1_size = bswap32(r.read_uint32());
			_info.r1 = bswap32(r.read_uint32());
			_info.r2 = bswap32(r.read_uint32());
			_info.r3 = bswap32(r.read_uint32());
			auto data2_size = bswap32(r.read_uint32());
			if(r.size() < data1_size + 4 || r.size() < data2_size + 0x14){
				return false;
			}
			auto data = r.read_data(data2_size);
			_info.r4 = bswap32(r.read_uint32());
			r = std::move(data);
		}
		break;
	case FileType::mihoyobin:
		_crypt(buf);
		break;
	default:
		return false;
	}

	return _dtype.analyze(r) && _load_btson(_json.root(), r);
}

bool Tson::save_btson(const char *path_btson)const noexcept{

	Writer w;
	switch(_filetype){
	case FileType::gia:
	case FileType::gil:
		{
			Writer data;
			if(!_save_btson(data, _json.root())){
				return false;
			}
			auto &d = data.data();
			w.write_uint32(bswap32(static_cast<uint32_t>(d.size()) + 0x14));
			w.write_uint32(bswap32(_info.r1));
			w.write_uint32(bswap32(_info.r2));
			w.write_uint32(bswap32(_info.r3));
			w.write_uint32(bswap32(static_cast<uint32_t>(d.size())));
			w.write_bytes(d.data(), d.size());
			w.write_uint32(bswap32(_info.r4));
		}
		break;
	case FileType::mihoyobin:
		{
			if(!_save_btson(w, _json.root())){
				return false;
			}
			_crypt(w.data());
		}
		break;
	default:
		return false;
	}

	FILE *fp = nullptr;
	if(fopen_s(&fp, path_btson, "wb") || fp == nullptr){
		return false;
	}
	fwrite(w.data().data(), 1, w.data().size(), fp);
	fclose(fp);

	return true;
}

bool Tson::load_json(const char *path_json)noexcept{

	Json json;
	if(!json.load_json(path_json)){
		return false;
	}

	auto &root = json.root();
	_filetype = _get_filetype(root.get_string("filetype").c_str());
	_dirtype = _get_dirtype(root.get_string("dirtype").c_str());
	auto info = const_cast<Json::Node *>(root.get("info"));
	if(info){
		_info.r1 = static_cast<uint32_t>(info->get_int64("1"));
		_info.r2 = static_cast<uint32_t>(info->get_int64("2"));
		_info.r3 = static_cast<uint32_t>(info->get_int64("3"));
		_info.r4 = static_cast<uint32_t>(info->get_int64("4"));
	}

	if(!_dtype.read_csv(root.get_string("dtype_csv"))){
		return false;
	}
	auto node = const_cast<Json::Node *>(root.get("json"));
	if(!node){
		return false;
	}
	_json.root(std::move(*node));

	return true;
}

bool Tson::save_json(const char *path_json, Writer &dtype_csv_w)const noexcept{

	Json json;
	auto &root = json.root();
	root.type(Json::Node::Type::Object);

	auto &parent = *const_cast<Json::Node::object_t *>(root.as_object());
	try{
		parent["filetype"] = Json::Node(string_t(_to_string(_filetype)));
		parent["dirtype"] = Json::Node(string_t(_to_string(_dirtype)));
		parent["info"] = Json::Node(Json::Node::object_t{});
		auto &info = *const_cast<Json::Node::object_t *>(root.get_object("info"));
		info["1"] = Json::Node(static_cast<Json::Node::int64_t>(_info.r1));
		info["2"] = Json::Node(static_cast<Json::Node::int64_t>(_info.r2));
		info["3"] = Json::Node(static_cast<Json::Node::int64_t>(_info.r3));
		info["4"] = Json::Node(static_cast<Json::Node::int64_t>(_info.r4));
		parent["json"] = _json.root();
		auto &data = dtype_csv_w.data();
		std::string str(reinterpret_cast<const char *>(data.data()), data.size());
		parent["dtype_csv"] = Json::Node(str);
	}catch(...){
		return false;
	}

	return json.save_json(path_json);
}
