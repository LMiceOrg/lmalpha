#ifndef _INCLUDE_LMJSON_H_
#define _INCLUDE_LMJSON_H_

#include <string>

/** jansson lib */
#include <jansson.h>



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


}  // namespace lmapi

#endif  //  _INCLUDE_LMJSON_H_
