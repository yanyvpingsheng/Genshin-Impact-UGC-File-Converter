#include "Tson.h"
#include "Util.h"
#include <stdio.h>

int main(int argc, char *argv[]){

	enum class Mode{
		Default,
		Decode,
		Encode,
	};
	Mode mode = Mode::Default;
  const char *path_input = nullptr;
  const char *path_output = nullptr;
	const char *path_dtype = nullptr;
	for(int i = 1; i < argc; ++i){
		if(*argv[i] == '-'){
			switch(argv[i][1]){
			case 'd':
				mode = Mode::Decode;
				break;
			case 'e':
				mode = Mode::Encode;
				break;
			case 'o':
				if(i + 1 < argc){
					path_output = argv[++i];
				}else{
					fprintf(stderr, "Error: -o オプションに出力ファイル名がありません。\n");
					return 1;
				}
				break;
			case 't':
				if(i + 1 < argc){
					path_dtype = argv[++i];
				}else{
					fprintf(stderr, "Error: -t オプションにdtypeファイル名がありません。\n");
					return 1;
				}
				break;
			}
		}else{
      path_input = argv[i];
    }
  }
  if(!path_input){
    fprintf(stderr, "使い方: %s [オプション] 入力ファイル名\n", argv[0]);
    fprintf(stderr, "オプション:\n");
    fprintf(stderr, "  -d : デコードモード .gil .gia .mihoyobin → json\n");
		fprintf(stderr, "  -e : エンコードモード json → .gil .gia .mihoyobin\n");
		fprintf(stderr, "  -o 出力ファイル名 : 出力ファイル名を指定\n");
		fprintf(stderr, "  -t dtypeファイル名 : dtypeファイル名を指定\n");
    return 1;
  }

	// モード判定
	if(mode == Mode::Default){
		auto size = strlen(path_input);
		mode = (size >= 5 && _stricmp(&path_input[size - 5], ".json") == 0) ? Mode::Encode : Mode::Decode;
	}

	// デコード
	if(mode == Mode::Decode){
		Tson tson;
		if(!tson.preload_btson(path_input)){
			fprintf(stderr, "Error: 未対応の拡張子。入力ファイルの種類が不明です。\n");
			return 1;
		}
		if(!path_dtype){
			switch(tson.filetype()){
			case Tson::FileType::gia:
				path_dtype = "dtype/gia.csv";
				break;
			case Tson::FileType::gil:
				path_dtype = "dtype/gil.csv";
				break;
			case Tson::FileType::mihoyobin:
				switch(tson.dirtype()){
				case Tson::DirType::Beyond_BeyondGlobal:
					path_dtype = "dtype/mihoyobin_BeyondGlobal.csv";
					break;
				case Tson::DirType::Beyond_Node:
					path_dtype = "dtype/mihoyobin_BeyondNode.csv";
					break;
				case Tson::DirType::Beyond_Official_Blueprint_OfficialCompoundNode:
					path_dtype = "dtype/mihoyobin_OfficialCompoundNode.csv";
					break;
				case Tson::DirType::Beyond_Official_OfficialPrefab:
					path_dtype = "dtype/mihoyobin_OfficialPrefab.csv";
					break;
				case Tson::DirType::Beyond_Official_Struct:
					path_dtype = "dtype/mihoyobin_OfficialStruct.csv";
					break;
				case Tson::DirType::Config_JsonConfig_ShortCutKey:
					path_dtype = "dtype/mihoyobin_ConfigShortCutKey.csv";
					break;
				case Tson::DirType::Config_JsonConfig_SynonymsLibrary:
					path_dtype = "dtype/mihoyobin_ConfigSynonymsLibrary.csv";
					break;
				case Tson::DirType::TextMap:
					path_dtype = "dtype/mihoyobin_TextMap.csv";
					break;
				}
				break;
			}
		}
		if(path_dtype){
			std::string buf;
			if(LoadFile(buf, path_dtype)){
				tson.read_dtype(buf);
			}
		}
		if(!tson.load_btson(path_input)){
			fprintf(stderr, "Error: 入力ファイルが読み込めませんでした。\n");
			return 1;
		}
		Writer w;
		tson.write_dtype(w);
		if(path_dtype){
			SaveFile(path_dtype, w.data());
		}
		if(!path_output){
			path_output = "output.json";
		}
		if(!tson.save_json(path_output, w)){
			fprintf(stderr, "Error: 出力ファイルが書き込めませんでした。\n");
			return 1;
		}
	}

	// エンコード
	else{
		Tson tson;
		if(!tson.load_json(path_input)){
			fprintf(stderr, "Error: 入力ファイルが読み込めませんでした。\n");
			return 1;
		}
		if(!path_output){
			switch(tson.filetype()){
			case Tson::FileType::gia:
				path_output = "output.gia";
				break;
			case Tson::FileType::gil:
				path_output = "output.gil";
				break;
			case Tson::FileType::mihoyobin:
				path_output = "output.mihoyobin";
				break;
			default:
				fprintf(stderr, "Error: 出力ファイルの種類が不明です。\n");
				return 1;
			}
		}
		if(!tson.save_btson(path_output)){
			fprintf(stderr, "Error: 出力ファイルが書き込めませんでした。\n");
			return 1;
		}
	}

	return 0;
}
