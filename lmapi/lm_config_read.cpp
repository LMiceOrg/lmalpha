/** Copyright 2018 He Hao<hehaoslj@sina.com> */

#include "lmapi.h"
#include "lmjson.h"
namespace lmapi {

struct config_internal {
  json_t* root;
  std::string name;
};

config::config(const std::string& name) : pdata(NULL) {
  config_internal* cfg = NULL;
  json_t* root = NULL;
  json_error_t err;
  // printf("cfg name %s\n", name.c_str());
  /** Load json conf */
  root = json_load_file(name.c_str(), 0, &err);
  if (root == NULL) {
    return;
  }

  // printf("load file\n");

  cfg = new config_internal;
  cfg->root = root;
  cfg->name = name;

  pdata = cfg;
}

config::~config() {
  config_internal* cfg;

  cfg = reinterpret_cast<config_internal*>(pdata);
  if (!cfg) return;

  /** close json */
  json_decrefp(&(cfg->root));
  delete cfg;
}

/** value */
int config::get_int(const std::string& name) {
  config_internal* cfg;
  int result;

  cfg = reinterpret_cast<config_internal*>(pdata);
  if (!cfg) return 0;

  get_json_integer(cfg->root, name, &result);
  return result;
}

int64_t config::get_int64(const std::string& name) {
  config_internal* cfg;
  int64_t result;

  cfg = reinterpret_cast<config_internal*>(pdata);
  if (!cfg) return 0;

  get_json_integer(cfg->root, name, &result);
  return result;
}

float config::get_float(const std::string& name) {
  config_internal* cfg;
  float result;

  cfg = reinterpret_cast<config_internal*>(pdata);
  if (!cfg) return 0;

  get_json_float(cfg->root, name, &result);
  return result;
}

double config::get_double(const std::string& name) {
  config_internal* cfg;
  double result;

  cfg = reinterpret_cast<config_internal*>(pdata);
  if (!cfg) return 0;

  get_json_float(cfg->root, name, &result);
  return result;
}

std::string config::get_string(const std::string& name) {
  config_internal* cfg;
  std::string result;

  cfg = reinterpret_cast<config_internal*>(pdata);
  if (!cfg) return result;

  get_json_string(cfg->root, name, result);
  return result;
}

/** array */
int config::get_array_size(const std::string& name) {
  config_internal* cfg;
  json_t* obj;
  int result = 0;

  cfg = reinterpret_cast<config_internal*>(pdata);
  if (!cfg) return 0;

  obj = obj_get(cfg->root, name);
  if (json_is_array(obj)) {
    result = json_array_size(obj);
  }

  return result;
}

int config::get_array_int(const std::string& name, int idx) {
  config_internal* cfg;
  json_t* obj;
  json_t* item;
  int result = 0;

  cfg = reinterpret_cast<config_internal*>(pdata);
  if (!cfg) return 0;

  obj = obj_get(cfg->root, name);
  if (json_is_array(obj)) {
    item = json_array_get(obj, idx);
    if (json_is_integer(item)) {
      result = json_integer_value(item);
    }
  }

  return result;
}
int64_t config::get_array_int64(const std::string& name, int idx) {
  config_internal* cfg;
  json_t* obj;
  json_t* item;
  int64_t result = 0;

  cfg = reinterpret_cast<config_internal*>(pdata);
  if (!cfg) return 0;

  obj = obj_get(cfg->root, name);
  if (json_is_array(obj)) {
    item = json_array_get(obj, idx);
    if (json_is_integer(item)) {
      result = json_integer_value(item);
    }
  }

  return result;
}

float config::get_array_float(const std::string& name, int idx) {
  config_internal* cfg;
  json_t* obj;
  json_t* item;
  float result = 0;

  cfg = reinterpret_cast<config_internal*>(pdata);
  if (!cfg) return 0;

  obj = obj_get(cfg->root, name);
  if (json_is_array(obj)) {
    item = json_array_get(obj, idx);
    if (json_is_real(item)) {
      result = json_real_value(item);
    }
  }

  return result;
}

double config::get_array_float64(const std::string& name, int idx) {
  config_internal* cfg;
  json_t* obj;
  json_t* item;
  double result = 0;

  cfg = reinterpret_cast<config_internal*>(pdata);
  if (!cfg) return 0;

  obj = obj_get(cfg->root, name);
  if (json_is_array(obj)) {
    item = json_array_get(obj, idx);
    if (json_is_real(item)) {
      result = json_real_value(item);
    }
  }

  return result;
}
std::string config::get_array_string(const std::string& name, int idx) {
  config_internal* cfg;
  json_t* obj;
  json_t* item;
  std::string result;

  cfg = reinterpret_cast<config_internal*>(pdata);
  if (!cfg) return 0;

  obj = obj_get(cfg->root, name);
  if (json_is_array(obj)) {
    item = json_array_get(obj, idx);
    if (json_is_string(item)) {
      result = json_string_value(item);
    }
  }

  return result;
}

}  // namespace lmapi
