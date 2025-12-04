#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <list>
#include <variant>

class Json{
public:
  Json()noexcept{}

  class Node{
  public:

    enum class Type{
      Null,
      Bool,
      Int64,
      Float64,
      String,
      Object,
      Array,
    };

    using bool_t = bool;
    using int64_t = int64_t;
    using float64_t = double;
    using string_t = std::string;
    using object_t = std::unordered_map<string_t, Node>;
    using array_t = std::list<Node>;

    Node()noexcept: _type(Type::Null), _value(std::monostate{}){}
    Node(bool_t b)noexcept: _type(Type::Bool), _value(b){}
    Node(int64_t i)noexcept: _type(Type::Int64), _value(i){}
    Node(float64_t f)noexcept: _type(Type::Float64), _value(f){}
    Node(string_t s)noexcept: _type(Type::String), _value(std::move(s)){}
    Node(object_t o)noexcept: _type(Type::Object), _value(std::move(o)){}
    Node(array_t a)noexcept: _type(Type::Array), _value(std::move(a)){}

    // 型確認
    Type type()const noexcept{return _type;}
    constexpr void type(Type t)noexcept{
      _type = t;
      switch(t){
      case Type::Null: _value = std::monostate{}; break;
      case Type::Bool: _value = bool_t{}; break;
      case Type::Int64: _value = int64_t{}; break;
      case Type::Float64: _value = float64_t{}; break;
      case Type::String: _value = string_t{};  break;
      case Type::Object: _value = object_t{}; break;
      case Type::Array: _value = array_t{}; break;
      }
    }
    bool is_null()const noexcept{return _type == Type::Null;}
    bool is_bool()const noexcept{return _type == Type::Bool;}
    bool is_int64()const noexcept{return _type == Type::Int64;}
    bool is_float64()const noexcept{return _type == Type::Float64;}
    bool is_string()const noexcept{return _type == Type::String;}
    bool is_object()const noexcept{return _type == Type::Object;}
    bool is_array()const noexcept{return _type == Type::Array;}

    // 値取得
    bool_t as_bool(bool_t def=false)const noexcept{
      if(auto *v = std::get_if<bool_t>(&_value))return *v;
      return def;
    }
    int64_t as_int64(int64_t def=0)const noexcept{
      if(auto * v = std::get_if<int64_t>(&_value))return *v;
      if(auto * v = std::get_if<float64_t>(&_value))return static_cast<int64_t>(*v);
      return def;
    }
    float64_t as_float64(float64_t def=0.0)const noexcept{
      if(auto *v = std::get_if<float64_t>(&_value))return *v;
      if(auto *v = std::get_if<int64_t>(&_value))return static_cast<float64_t>(*v);
      return def;
    }
    const string_t &as_string(const string_t &def={})const noexcept{
      if(auto *v = std::get_if<string_t>(&_value))return *v;
      return def;
    }
    const object_t *as_object()const noexcept{
      return std::get_if<object_t>(&_value);
    }
    const array_t *as_array()const noexcept{
      return std::get_if<array_t>(&_value);
    }
    
    // オブジェクト用検索ヘルパー
    const Node *get(const string_t &key)const noexcept{
      if(const auto *obj = as_object()){
        auto it = obj->find(key);
        if(it != obj->end()){
          return &it->second;
        }
      }
      return nullptr;
    }
    bool_t get_bool(const string_t &key, bool_t def=false)const noexcept{
      if(auto node = get(key)){
			  return node->as_bool(def);
		  }
		  return def;
    }
    int64_t get_int64(const string_t &key, int64_t def=0)const noexcept{
      if(auto node = get(key)){
			  return node->as_int64(def);
		  }
		  return def;
    }
    float64_t get_float64(const string_t &key, float64_t def=0.0)const noexcept{
      if(auto node = get(key)){
			  return node->as_float64(def);
		  }
		  return def;
    }
    const string_t &get_string(const string_t &key, const string_t &def={})const noexcept{
      if(auto node = get(key)){
			  return node->as_string(def);
		  }
		  return def;
    }
    const object_t *get_object(const string_t &key)const noexcept{
      if(auto node = get(key)){
			  return node->as_object();
		  }
		  return nullptr;
    }
    const array_t *get_array(const string_t &key)const noexcept{
      if(auto node = get(key)){
			  return node->as_array();
		  }
		  return nullptr;
    }

  private:
    Type _type;
    std::variant<std::monostate, bool_t, int64_t, float64_t, string_t, object_t, array_t> _value;
    friend class Json;
  };

  Node &root()noexcept{return _root;}
  const Node &root()const noexcept{return _root;}
  void root(Node &&node)noexcept{_root = std::move(node);}

  bool load_json(const char *path_json)noexcept;
  bool save_json(const char *path_json)const noexcept;

private:
  Node _root{};
};
