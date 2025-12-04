#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include "Dtype.h"
#include "Json.h"
#include "Reader.h"
#include "Writer.h"

using string_t = std::string;
using data_t = std::vector<uint8_t>;

class Tson{
public:
	Tson()noexcept{}

	enum class FileType{
		Unknown,
		gia,
		gil,
		mihoyobin,
	};
	enum class DirType{
		Unknown,
		Beyond_BeyondGlobal,
		Beyond_Node,
		Beyond_Official_Blueprint_OfficialCompoundNode,
		Beyond_Official_OfficialPrefab,
		Beyond_Official_Struct,
		Config_JsonConfig_ShortCutKey,
		Config_JsonConfig_SynonymsLibrary,
		TextMap,
	};
	bool preload_btson(const char *path_btson)noexcept;
	FileType filetype()const noexcept{return _filetype;}
	DirType dirtype()const noexcept{return _dirtype;}

	bool read_dtype(const string_t &string)noexcept{return _dtype.read_csv(string);}
	bool write_dtype(Writer &w)const noexcept{return _dtype.write_csv(w);}

	bool load_btson(const char *path_btson)noexcept;
	bool save_btson(const char *path_btson)const noexcept;

	bool load_json(const char *path_json)noexcept;
	bool save_json(const char *path_json, Writer &dtype_csv_w)const noexcept;

private:
	struct FileTypeEntry{
		std::string_view name;
		FileType type;
	};
	struct DirTypeEntry{
		std::string_view name;
		Tson::DirType    type;
	};
	inline static constexpr FileTypeEntry _filetype_table[] = {
		{"gia",       FileType::gia},
		{"gil",       FileType::gil},
		{"mihoyobin", FileType::mihoyobin},
	};
	inline static constexpr DirTypeEntry _dirtype_table[] = {
		{"BeyondGlobal",          DirType::Beyond_BeyondGlobal},
		{"BeyondNode",            DirType::Beyond_Node},
		{"OfficialCompoundNode",  DirType::Beyond_Official_Blueprint_OfficialCompoundNode},
		{"OfficialPrefab",        DirType::Beyond_Official_OfficialPrefab},
		{"OfficialStruct",        DirType::Beyond_Official_Struct},
		{"ConfigShortCutKey",     DirType::Config_JsonConfig_ShortCutKey},
		{"ConfigSynonymsLibrary", DirType::Config_JsonConfig_SynonymsLibrary},
		{"TextMap",               DirType::TextMap},
	};
	static FileType _get_filetype(const char *ext)noexcept;
	static DirType _get_dirtype(const char *string)noexcept;
	static std::string_view _to_string(FileType type)noexcept;
	static std::string_view _to_string(DirType type)noexcept;
	FileType _filetype{};
	DirType _dirtype{};
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
	struct{
		uint32_t r1{};
		uint32_t r2{};
		uint32_t r3{};
		uint32_t r4{};
	}_info{};
	Dtype _dtype{};
	Json _json{};
	static void _crypt(data_t &data)noexcept;
	bool _load_btson(Json::Node &json_parent, Reader &r, const Dtype::id_t &dtype_parent_id={})noexcept;
	bool _save_btson(Writer &w, const Json::Node &json_node, const Dtype::id_t &dtype_parent_id={})const noexcept;
};
