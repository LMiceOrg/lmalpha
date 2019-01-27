/** Copyright 2018, 2019 He Hao<hehaoslj@sina.com> */

#ifndef INCLUDE_LMXML_H_
#define INCLUDE_LMXML_H_

/** xml2 */
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

/** cpp lib */
#include <string>
#include <vector>

namespace lmapi {

/** helper */
static inline xmlDocPtr xml_open(const std::string& name,
                                 const std::string& encoding) {
  xmlDocPtr doc;
  doc = xmlReadFile(name.c_str(), encoding.c_str(), XML_PARSE_RECOVER);
  return doc;
}

static inline xmlNodePtr obj_get(xmlDocPtr doc, const std::string& name) {
  xmlNodePtr root;
  xmlNodePtr node = nullptr;
  std::string key;
  size_t begin = 0;
  size_t end = 0;

  root = xmlDocGetRootElement(doc);
  end = name.find(".", begin);
  key = name.substr(begin, end);

  if (xmlStrcmp(root->name, BAD_CAST key.c_str()) == 0) {
    // only root node ,return
    if (end == name.npos) return root;

    // find child node
    begin = end + 1;
    do {
      end = name.find(".", begin);
      key = name.substr(begin, end);
      node = root->xmlChildrenNode;

      while (node != nullptr) {
        if (0 == xmlStrcmp(node->name, BAD_CAST key.c_str())) {
          root = node;
          break;
        }
        node = node->next;
      }
      if (node == nullptr) {
        break;
      }
      if (end == std::string::npos) {
        break;
      }
      begin = end + 1;
    } while (true);
  }

  return node;
}

static inline xmlAttrPtr attr_get(xmlNodePtr node, const std::string& prop) {
  xmlAttrPtr attr = node->properties;
  while (attr != NULL) {
    if (0 == xmlStrcmp(attr->name, BAD_CAST prop.c_str())) {
      break;  // break while: attr
    }
    attr = attr->next;
  }  //  while: attr

  return attr;
}

static inline std::string get_xml_attr(xmlDocPtr doc, const std::string& name,
                                       const std::string& prop) {
  xmlNodePtr node;
  xmlAttrPtr attr;
  xmlChar* xvalue;
  std::string svalue;

  node = obj_get(doc, name);
  if (node) {
    attr = attr_get(node, prop);
    if (attr) {
      xvalue = xmlGetProp(node, BAD_CAST prop.c_str());
      svalue = reinterpret_cast<char*>(xvalue);
      xmlFree(xvalue);
    }
  }
  return svalue;
}

static inline std::string get_xml_node(xmlDocPtr doc, const std::string& name) {
  xmlNodePtr node;
  xmlChar* xvalue;
  std::string svalue;
  node = obj_get(doc, name);
  if (node) {
    xvalue = xmlNodeGetContent(node);
    svalue = reinterpret_cast<char*>(xvalue);
    xmlFree(xvalue);
  }

  // printf("node %s:%s\n", name.c_str(), svalue.c_str());

  return svalue;
}

static inline std::string get_xml_val(xmlDocPtr doc, const std::string& name) {
  std::string sval;

  // 1. check node exists
  sval = get_xml_node(doc, name);
  if (sval.empty()) {
    size_t pos;
    // 2. check attr exists
    pos = name.rfind(".", name.size());
    if (pos != name.npos) {
      std::string node = name.substr(0, pos);
      std::string attr = name.substr(pos + 1, name.size());
      sval = get_xml_attr(doc, node, attr);
      // printf("attr %s:%s %s\n", node.c_str(), attr.c_str(), sval.c_str() );
    }
  }
  return sval;
}

template <class T>
static inline void get_xml_integer(xmlDocPtr doc, const std::string& name,
                                   T* t) {
  std::string sval;
  sval = get_xml_val(doc, name);
  *t = atoll(sval.c_str());
}

static inline void get_xml_float(xmlDocPtr doc, const std::string& name,
                                 double* t) {
  std::string sval;
  sval = get_xml_val(doc, name);
  *t = atof(sval.c_str());
}

static inline void get_xml_array(xmlDocPtr doc, const std::string& name,
                                 std::vector<std::string>* vec) {
  size_t pos;
  xmlNodePtr root;
  xmlNodePtr node;

  pos = name.rfind(".", name.size());
  if (pos == name.npos) return;

  std::string rnode = name.substr(0, pos);
  std::string anode = name.substr(pos + 1, name.size());

  /**  step1 node content array */
  root = obj_get(doc, rnode);
  if (root) {
    node = root->children;
    while (node != nullptr) {
      if (0 == xmlStrcmp(node->name, BAD_CAST anode.c_str())) {
        xmlChar* xvalue;
        std::string svalue;

        xvalue = xmlNodeGetContent(node);
        svalue = reinterpret_cast<char*>(xvalue);
        xmlFree(xvalue);

        vec->push_back(svalue);
      }
      node = node->next;
    }
  }

  /** step2 node property array */
  std::string prop = anode;
  pos = rnode.rfind(".", rnode.size());
  if (pos == name.npos) return;
  anode = rnode.substr(pos + 1, rnode.size());
  rnode = rnode.substr(0, pos);

  root = obj_get(doc, rnode);
  if (!root) return;

  node = root->children;
  while (node != nullptr) {
    if (0 == xmlStrcmp(node->name, BAD_CAST anode.c_str())) {
      xmlChar* xvalue;
      xmlAttrPtr attr;
      std::string svalue;

      attr = attr_get(node, prop);
      if (attr) {
        xvalue = xmlGetProp(node, BAD_CAST prop.c_str());
        svalue = reinterpret_cast<char*>(xvalue);
        xmlFree(xvalue);

        vec->push_back(svalue);
      }
    }
    node = node->next;
  }  // while: node
}

#define get_xml_array_string get_xml_array

template <class T>
static inline void get_xml_array_int(xmlDocPtr doc, const std::string& name,
                                     std::vector<T>* vec) {
  int64_t intval;
  std::vector<std::string> strlist;
  get_xml_array(doc, name, &strlist);

  for (const auto& str : strlist) {
    intval = atoll(str.c_str());
    vec->push_back(intval);
  }
}

template <class T>
static inline void get_xml_array_float(xmlDocPtr doc, const std::string& name,
                                       std::vector<T>* vec) {
  double dblval;
  std::vector<std::string> strlist;
  get_xml_array(doc, name, &strlist);

  for (const auto& str : strlist) {
    dblval = atof(str.c_str());
    vec->push_back(dblval);
  }
}

}  // namespace lmapi

#endif  // INCLUDE_LMXML_H_
