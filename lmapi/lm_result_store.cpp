#include "lmapi.h"

#include "lmsql.h"

/**

  create table factor_info (id bigint primary key identity,
  name varchar(64) not null,
  author varchar(32),
  date varchar(32)
  )

  create table factor_result(id bigint primary key identity,
  factor varchar(64) not null,
  date int,
  time int,
  value float
  )

  create table factor_info (id bigint primary key identity,name varchar(64) not
  null,author varchar(32),date varchar(32))

  create table factor_result(id bigint primary key identity,factor varchar(64)
  not null,date int,time int,value float)
*/

namespace lmapi {

factor_result::factor_result(const std::string& conns,
                             const std::string& name) {
  sql_internal* sql;
  int ret;

  sql = new sql_internal();
  pdata = sql;
  factor = name;

  ret = sql->connect(conns);
  if (ret != 0) {
    return;
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

  try {
    // 1. check factor info
    sql->os.open(5,
                 "select top 1 name from factor_info where name=:f1<char[64]>",
                 sql->db);
    sql->os << info.factor_name;
    exists = sql->os.get_prefetched_row_count();
    sql->os.close();

    if (exists >= 1) {
      //  2. update factor info
      sql->os.open(1,
                   "update factor_info set author=:f1<char[32]>, "
                   "date=:f2<char[32]> where name =:f3<char[64]>",
                   sql->db);
      sql->os << info.factor_author << info.factor_date << info.factor_name;
      sql->os.close();
    } else {
      //  3. insert factor info
      sql->os.open(
          1,
          "insert into factor_info values(:f1<char[64]>, :f2<char[32]>, "
          ":f3<char[32]>)",
          sql->db);
      sql->os << info.factor_name << info.factor_author << info.factor_date;
      sql->os.close();
    }

    return 0;
  } catch (otl_exception& e) {
    // intercept OTL exceptions
    std::ostringstream os;
    os << e.msg << "\t" << e.stm_text << "\t" << e.sqlstate << "\t"
       << e.var_info << std::endl;
    sql->err_msg += os.str();
    printf("sql err %s\n", sql->err_msg.c_str());
    return 0;
  }
}

int factor_result::store_result(const std::vector<lmapi_result_data>& dataset) {
  sql_internal* sql;
  std::ostringstream os;
  std::string value;
  int exists = 0;

  sql = reinterpret_cast<sql_internal*>(pdata);

  try {
    //  1. check factor result exists
    sql->os.open(
        5, "select top 1 factor from factor_result where factor=:f1<char[64]>",
        sql->db);
    sql->os << factor;
    exists = sql->os.get_prefetched_row_count();
    sql->os.close();

    //  2. delete existing result
    if (exists > 0) {
      sql->os.open(1, "delete from factor_result where factor=:f1<char[64]>",
                   sql->db);
      sql->os << factor;
      sql->os.close();
    }

    //  3. insert factor result
    sql->os.open(1,
                 "insert into factor_result values(:f1<char[64]>, :f2<int>, "
                 ":f3<int>,:f4<double>)",
                 sql->db);
    for (size_t i = 0; i < dataset.size(); ++i) {
      const lmapi_result_data& ds = dataset[i];
      os << ds.value;
      value = os.str();
      sql->os << factor << ds.date << ds.time << ds.value;
    }
    sql->os.close();

    return 0;
  } catch (otl_exception& e) {
    // intercept OTL exceptions
    std::ostringstream os;
    os << e.msg << "\t" << e.stm_text << "\t" << e.sqlstate << "\t"
       << e.var_info << std::endl;
    sql->err_msg += os.str();

    printf("sql err %s\n", sql->err_msg.c_str());
    return 0;
  }
}

}  // namespace lmapi
