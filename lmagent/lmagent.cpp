/** Copyright 2018, 2019 He Hao<hehaoslj@sina.com> */

#include <stdio.h>

#include <jansson.h>

#include <lmice_trace.h>

#include <string>
#include <vector>

#include "lmagent.h"

struct lm_config_file{
    std::string factor_name;
    std::vector<std::string> instrument_array;
    std::string start_date;
    std::string end_date;
    lm_data_type data_type;
    int data_length;
    int data_step;
};

json_t *obj_get(json_t *root, const char *keys) {
  json_t *o = root;
  char *key = NULL;
  size_t nsz = strlen(keys);
  char *nkeys = (char *)malloc(nsz + 1);
  char *last = NULL;
  memcpy(nkeys, keys, nsz);
  nkeys[nsz] = '\0';
  // printf("keys:%s\n", keys);
  for (key = strtok_r(nkeys, ".", &last); key;
       key = strtok_r(NULL, ".", &last)) {
    // printf("key:%s\n", key);
    o = json_object_get(o, key);
  }

  free(nkeys);
  return o;
}

static inline void get_json_string(json_t *root, const char *key,
                                   std::string &value) {
  json_t *obj = obj_get(root, key);
  if (json_is_string(obj)) {
    value = json_string_value(obj);
  }
}

template <class T>
static inline void get_json_integer(json_t *root, const char *key, T *value) {
  json_t *obj = obj_get(root, key);
  if (json_is_integer(obj)) {
    *value = json_integer_value(obj);
  }
}

template <class T>
static inline void get_json_float(json_t *root, const char *key, T *value) {
  json_t *obj = obj_get(root, key);
  if (json_is_number(obj)) {
    *value = json_number_value(obj);
  }
}

std::string &replace_all(std::string &str, const std::string &old_value,
                         const std::string &new_value) {
  while (true) {
    std::string::size_type pos(0);
    if ((pos = str.find(old_value)) != std::string::npos)
      str.replace(pos, old_value.length(), new_value);
    else
      break;
  }
  return str;
}

int lm_run_file(const char *cfg_name) {

  json_t *root = NULL;
  json_error_t err;
  json_t *obj = NULL;
  json_t *arr = NULL;
  const char *key = NULL;
  const char *svalue = NULL;
  size_t i;

  lm_config_file cfg;

  lmice_info_print("Run file %s\n", cfg_name);

  /** Load json conf */
  root = json_load_file(cfg_name, 0, &err);
  if (root == NULL) {
    lmice_error_print("load config file error:%s at line %d, col %d\n",
                      err.text, err.line, err.column);
    return -1;
  }

  /** factor name */
  key = "factor.name";
  obj = obj_get(root, key);
  if (!json_is_string(obj)) {
    lmice_error_print("config keys[%s] is missing.\n", key);
    return -1;
  }
  svalue = json_string_value(obj);
  cfg.factor_name = svalue;

  lmice_info_print("run factor %s\n", cfg.factor_name.c_str());

  /* instrument list */
  key = "testing.instruments";
  obj = obj_get(root, key);
  if (json_is_string(obj)) {
    svalue = json_string_value(obj);
    cfg.instrument_array.push_back(svalue);

  } else if (json_is_array(obj)) {
    arr = obj;
    for (i = 0; i < json_array_size(arr) && (obj = json_array_get(arr, i));
         i++) {
      if (!json_is_string(obj)) {
        lmice_error_print("config keys[%s] got wrong type[string only].\n",
                          key);
        return -1;
      }
      svalue = json_string_value(obj);
      cfg.instrument_array.push_back(svalue);
    }
  }

  lmice_info_print("  %ld instruments  ", cfg.instrument_array.size());
  for (i = 0; i < cfg.instrument_array.size(); ++i) {
    if (i > 2) {
      break;
    }
    printf("%s  ", cfg.instrument_array[i].c_str());
  }
  printf("\n");

  /* date */
  key = "testing.start_date";
  obj = obj_get(root, key);
  if (!json_is_string(obj)) {
    lmice_error_print("config keys [%s] is mssing or wrong type.\n", key);
    return -1;
  }
  svalue = json_string_value(obj);
  cfg.start_date = svalue;

  key = "testing.end_date";
  obj = obj_get(root, key);
  if (!json_is_string(obj)) {
    lmice_error_print("config keys [%s] is mssing or wrong type.\n", key);
    return -1;
  }
  svalue = json_string_value(obj);
  cfg.end_date = svalue;

  lmice_info_print("  date range [%s -to- %s].\n", cfg.start_date.c_str(),
                   cfg.end_date.c_str());

  /** data type */
  key = "testing.data_type";
  obj = obj_get(root, key);
  if (!json_is_string(obj)) {
    lmice_error_print("config keys [%s] is mssing or wrong type.\n", key);
    return -1;
  }
  svalue = json_string_value(obj);
  if (strcmp(svalue, "5min") == 0) {
    cfg.data_type = K_5MIN_TYPE;
  } else if (strcmp(svalue, "tick") == 0) {
    cfg.data_type = TICK_TYPE;
  }

  lmice_info_print("  data type %d.\n", cfg.data_type);

  /** data length */
  key = "testing.data_length";
  obj = obj_get(root, key);
  if (!json_is_integer(obj)) {
    lmice_error_print("config keys [%s] is mssing or wrong type.\n", key);
    return -1;
  }
  cfg.data_length = json_integer_value(obj);

  lmice_info_print("  data length %d.\n", cfg.data_length);

  /** data step */
  key = "testing.data_step";
  obj = obj_get(root, key);
  if (!json_is_integer(obj)) {
    lmice_error_print("config keys [%s] is mssing or wrong type.\n", key);
    return -1;
  }
  cfg.data_step = json_integer_value(obj);

  lmice_info_print("  data step %d.\n", cfg.data_length);

  /** close json */
  json_decrefp(&root);

  return 0;
}
