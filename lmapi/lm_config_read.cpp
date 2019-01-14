/** Copyright 2018, 2019 He Hao<hehaoslj@sina.com> */


#include <sstream>

#include "lmapi.h"
#include "lmjson.h"
#include "lmxml.h"
namespace lmapi {

struct config_internal {
  json_t* root;
  std::string name;
  xmlDocPtr doc;
  // 1:xml 2:json
  int type;
  enum config_type { xml_type, json_type };

  config_internal(const std::string& name_) : name(name_) {
    json_error_t err;

    if (name.find(".xml", 0) != name.npos) {
      doc = xml_open(name, "utf-8");
      type = xml_type;
    } else {
      root = json_load_file(name.c_str(), 0, &err);
      type = json_type;
    }
  }

  ~config_internal() {
    if (type == xml_type) {
      xmlFreeDoc(doc);
    } else {
      /** close json */
      json_decrefp(&(root));
    }
  }

  template <class T>
  void get_int(const std::string& name, T* t) {
    if (type == xml_type) {
      get_xml_integer(doc, name, t);
    } else {
      get_json_integer(root, name, t);
    }
  }

  template <class T>
  void get_float(const std::string& name, T* t) {
    double val = 0;
    if (type == xml_type) {
      get_xml_float(doc, name, &val);
    } else {
      get_json_float(root, name, &val);
    }
    *t = val;
  }

  void get_string(const std::string& name, std::string* val) {
    if (type == xml_type) {
      *val = get_xml_val(doc, name);
    } else {
      get_json_string(root, name, *val);
    }
  }

  void get_array_string(const std::string& name,
                        std::vector<std::string>* vec) {
    if (type == xml_type) {
      get_xml_array_string(doc, name, vec);
    } else {
      get_json_array_string(root, name, vec);
    }
  }
  template <class T>
  void get_array_int(const std::string& name, std::vector<T>* vec) {
    if (type == xml_type) {
      get_xml_array_int(doc, name, vec);
    } else {
      get_json_array_int(root, name, vec);
    }
  }

  template <class T>
  void get_array_float(const std::string& name, std::vector<T>* vec) {
    if (type == xml_type) {
      get_xml_array_float(doc, name, vec);
    } else {
      get_json_array_float(root, name, vec);
    }
  }

};  // config_internal

config::config(const std::string& name) : pdata(NULL) {
  config_internal* cfg = NULL;

  cfg = new config_internal(name);

  pdata = cfg;
}

config::~config() {
  config_internal* cfg;

  cfg = reinterpret_cast<config_internal*>(pdata);

  delete cfg;
}

bool config::is_open() const {
  config_internal* cfg;

  cfg = reinterpret_cast<config_internal*>(pdata);
  if (cfg->type == cfg->xml_type && cfg->doc != nullptr) {
    return true;
  } else if (cfg->type == cfg->json_type && cfg->root != nullptr) {
    return true;
  }
  return false;
}

/** value */
int config::get_int(const std::string& name) {
  config_internal* cfg;
  int result;

  cfg = reinterpret_cast<config_internal*>(pdata);
  cfg->get_int(name, &result);
  return result;
}

int64_t config::get_int64(const std::string& name) {
  config_internal* cfg;
  int64_t result;

  cfg = reinterpret_cast<config_internal*>(pdata);
  cfg->get_int(name, &result);
  return result;
}

float config::get_float(const std::string& name) {
  config_internal* cfg;
  float result;

  cfg = reinterpret_cast<config_internal*>(pdata);

  cfg->get_float(name, &result);

  return result;
}

double config::get_double(const std::string& name) {
  config_internal* cfg;
  double result;

  cfg = reinterpret_cast<config_internal*>(pdata);

  cfg->get_float(name, &result);
  return result;
}

std::string config::get_string(const std::string& name) {
  config_internal* cfg;
  std::string result;

  cfg = reinterpret_cast<config_internal*>(pdata);

  cfg->get_string(name, &result);

  return result;
}

/** array */
std::vector<int> config::get_array_int(const std::string& name) {
  std::vector<int> result;
  config_internal* cfg;

  cfg = reinterpret_cast<config_internal*>(pdata);

  cfg->get_array_int(name, &result);
  return result;
}

std::vector<int64_t> config::get_array_int64(const std::string& name) {
  std::vector<int64_t> result;
  config_internal* cfg;

  cfg = reinterpret_cast<config_internal*>(pdata);

  cfg->get_array_int(name, &result);
  return result;
}

std::vector<float> config::get_array_float(const std::string& name) {
  std::vector<float> result;
  config_internal* cfg;

  cfg = reinterpret_cast<config_internal*>(pdata);

  cfg->get_array_float(name, &result);
  return result;
}

std::vector<double> config::get_array_float64(const std::string& name) {
  std::vector<double> result;
  config_internal* cfg;

  cfg = reinterpret_cast<config_internal*>(pdata);

  cfg->get_array_float(name, &result);
  return result;
}

std::vector<std::string> config::get_array_string(const std::string& name) {
  std::vector<std::string> result;
  config_internal* cfg;

  cfg = reinterpret_cast<config_internal*>(pdata);

  cfg->get_array_string(name, &result);
  return result;
}

}  // namespace lmapi
