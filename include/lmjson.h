/** Copyright 2018, 2019 He Hao<hehaoslj@sina.com> */

#ifndef INCLUDE_LMJSON_H_
#define INCLUDE_LMJSON_H_

/** jansson lib */
#include <jansson.h>

/** cpp lib */
#include <string>
#include <vector>

namespace lmapi {

/** helper */
static inline json_t* obj_get(json_t* root, const std::string& name) {
  json_t* o = root;
  std::string key;
  size_t begin = 0;
  size_t end = 0;
  do {
    end = name.find(".", begin);
    key = name.substr(begin, end);
    o = json_object_get(o, key.c_str());
    if (end == std::string::npos) {
      break;
    }
    begin = end + 1;
  } while (true);

  return o;
}

static inline void get_json_string(json_t* root, const std::string& key,
                                   std::string& value) {
  json_t* obj = obj_get(root, key);
  if (json_is_string(obj)) {
    value = json_string_value(obj);
  }
}

template <class T>
static inline void get_json_integer(json_t* root, const std::string& key,
                                    T* value) {
  json_t* obj = obj_get(root, key);
  if (json_is_integer(obj)) {
    *value = json_integer_value(obj);
  }
}

template <class T>
static inline void get_json_float(json_t* root, const std::string& key,
                                  T* value) {
  json_t* obj = obj_get(root, key);
  if (json_is_number(obj)) {
    *value = json_number_value(obj);
  }
}

static inline void get_json_array_string(json_t* root, const std::string& key,
                                         std::vector<std::string>* vec) {
  size_t size;
  json_t* obj;
  json_t* item;

  obj = obj_get(root, key);
  if (json_is_array(obj)) {
    size = json_array_size(obj);
    for (size_t i = 0; i < size; ++i) {
      item = json_array_get(obj, i);
      if (json_is_string(item)) {
        vec->push_back(json_string_value(item));
      }
    }
  }
}

template <class T>
static inline void get_json_array_int(json_t* root, const std::string& key,
                                      std::vector<T>* vec) {
  size_t size;
  json_t* obj;
  json_t* item;

  obj = obj_get(root, key);
  if (json_is_array(obj)) {
    size = json_array_size(obj);
    for (size_t i = 0; i < size; ++i) {
      item = json_array_get(obj, i);
      if (json_is_integer(item)) {
        vec->push_back(json_integer_value(item));
      }
    }
  }
}

template <class T>
static inline void get_json_array_float(json_t* root, const std::string& key,
                                        std::vector<T>* vec) {
  size_t size;
  json_t* obj;
  json_t* item;

  obj = obj_get(root, key);
  if (json_is_array(obj)) {
    size = json_array_size(obj);
    for (size_t i = 0; i < size; ++i) {
      item = json_array_get(obj, i);
      if (json_is_real(item)) {
        vec->push_back(json_real_value(item));
      }
    }
  }
}

}  // namespace lmapi

#endif  //  INCLUDE_LMJSON_H_
