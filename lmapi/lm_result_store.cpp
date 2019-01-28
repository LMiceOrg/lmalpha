/** Copyright 2018, 2019 He Hao<hehaoslj@sina.com> */

#include "lmapi.h"
#include "lmsql.h"

/**

  create table factor_info (id bigint primary key identity,
  name varchar(32) not null,
  author varchar(32),
  last_update varchar(32)
  )

  create table factor_result(id bigint primary key identity,
  factor varchar(64) not null,
  code varchar(16) not null,
  date int,
  time int,
  value float
  )

*/

namespace lmapi {

factor_result::factor_result(const std::string& conns,
                             const std::string& name) {
  sql_internal* sql;
  int ret;
  int exists_info = 0;
  int exists_result = 0;

  sql = new sql_internal();
  pdata = sql;
  factor = name;

  ret = sql->connect(conns);
  if (ret != 0) {
    printf("%s\n", sql->err_msg.c_str());
    return;
  }

  // 1. check table exists
  char query_format[128] =
      "SELECT top 1 * FROM alpha.dbo.sysobjects "
      "WHERE xtype = 'U' "
      "AND name='%s'";
  char query[256];

  memset(query, 0, sizeof(query));
  snprintf(query, sizeof(query), query_format, "factor_info");
  sql->select(query);
  exists_info = sql->row_count;

  memset(query, 0, sizeof(query));
  snprintf(query, sizeof(query), query_format, "factor_result");
  sql->select(query);
  exists_result = sql->row_count;

  // printf("factor_result %d %s\n", exists, sql->err_msg.c_str());

  // 2. create factor result tables
  if (exists_info == 0) {
    sql->execute(
        "create table alpha.dbo.factor_info ("
        "id bigint primary key identity,"
        "name varchar(32) not null,"
        "author varchar(32),"
        "last_update varchar(32)"
        ")");
  }
  if (exists_result == 0) {
    sql->execute(
        "create table alpha.dbo.factor_result("
        "id bigint primary key identity,"
        "name varchar(32) not null,"
        "code varchar(32) not null,"
        "date int,"
        "time int,"
        "value float"
        ")");
  }
}

factor_result::~factor_result() {
  sql_internal* sql;

  sql = reinterpret_cast<sql_internal*>(pdata);
  delete sql;
}

const std::string& factor_result::error() const {
  sql_internal* sql;

  sql = reinterpret_cast<sql_internal*>(pdata);
  return sql->err_msg;
}

int factor_result::store_factor(const lmapi_result_info& info) {
  sql_internal* sql;
  std::ostringstream os;
  int exists = 0;

  sql = reinterpret_cast<sql_internal*>(pdata);

  // 1. check factor info
  char query[256];
  memset(query, 0, sizeof(query));
  snprintf(query, sizeof(query),
           "select top 1 name from alpha.dbo.factor_info where "
           "name='%s'",
           info.factor_name);
  sql->select(query);

  exists = sql->row_count;

  if (exists >= 1) {
    // 2. update factor info
    memset(query, 0, sizeof(query));
    snprintf(query, sizeof(query),
        "update alpha.dbo.factor_info set "
        "author=:author<char[32],in>, "
        "last_update=:date<char[32],in> "
        "where name =:name<char[32],in> ");
    sql->execute_utf8str(query, 3,
             info.factor_author, info.factor_date, info.factor_name);
    
  } else {
    //  3. insert factor info
    memset(query, 0, sizeof(query));
    snprintf(query, sizeof(query),
        "insert into alpha.dbo.factor_info values(:name<char[32],in>, "
        ":author<char[32],in>, "
        ":date<char[32],in>) ");
    sql->execute_utf8str(query, 3, 
             info.factor_name, info.factor_author, info.factor_date);
    
  }

  if (!sql->err_msg.empty())
    return 1;
  else
    return 0;
}

int factor_result::store_result(const std::string& name, const std::string code,
                                const std::vector<lmapi_result_data>& dataset) {
  sql_internal* sql;
  std::ostringstream os;
  std::string value;
  int exists = 0;
  char query[256];

  sql = reinterpret_cast<sql_internal*>(pdata);

  //  1. check factor result exists
 /* memset(query, 0, sizeof(query));
  snprintf(query, sizeof(query),
           "select top 1 name from alpha.dbo.factor_result where "
           "name='%s' and code='%s'",
           name.c_str(), code.c_str());
  sql->select(query);
  exists = sql->row_count;*/

  //  2. delete existing result
   {
    memset(query, 0, sizeof(query));
    snprintf(query, sizeof(query),
        "delete from alpha.dbo.factor_result where "
        "name=:name<char[32],in> and code=:code<char[32],in>");
    sql->execute_utf8str(query, 2,
             name.c_str(), code.c_str());
    
  }

  //  3. insert factor result
  sql->insert(
      " insert into alpha.dbo.factor_result "
      "values(:name<char[32],in>,"
      ":code<char[32],in>,"
      ":f3<int,in>,"
      ":f4<int,in>,"
      ":f5<double,in>)",
      name, code, dataset);

  printf("  store %s result %lu\n", code.c_str(), dataset.size());
  if (sql->err_msg.size() != 0)
    return 1;
  else
    return 0;
}

int factor_result::store(
    const std::vector<std::string>& stocks,
    const std::vector<std::vector<lmapi_result_data> >& results) {
  lmapi_result_info info;
  int ret;
  sql_internal* sql;

  sql = reinterpret_cast<sql_internal*>(pdata);

  printf("1. store factor info\n");
  // 1. store info
  memset(&info, 0, sizeof(info));
  snprintf(info.factor_author, sizeof(info.factor_author), "lyalpha");
  snprintf(info.factor_name, sizeof(info.factor_name), "%s", factor.c_str());
  // ctime_r(0, info.factor_date);
  snprintf(info.factor_date, sizeof(info.factor_date), "2019-01-11");
  // printf("call store_factor\n");
  ret = store_factor(info);
  if (ret != 0) {
    printf("%s", sql->err_msg.c_str());
    sql->err_msg.clear();
    return ret;
  }

  printf("2. store factor result\n");
  // 2. store result
  for (size_t i = 0; i < stocks.size(); ++i) {
    const std::string& code = stocks[i];
    const std::vector<lmapi_result_data>& result = results[i];

    ret = store_result(factor, code, result);
    if (ret != 0) {
      printf("%s", sql->err_msg.c_str());
      sql->err_msg.clear();
      return ret;
    }
  }

  return 0;
}

}  // namespace lmapi
