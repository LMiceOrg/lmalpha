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
    return;
  }

  // check table exists
  try {
    sql->os.open(1,
                 "SELECT top 1 * FROM alpha.dbo.sysobjects "
                 "WHERE xtype = 'U'"
                 " AND name=:table_name<char[32],in>",
                 sql->db);

    sql->os << (OTL_UNICODE_CHAR_TYPE*)L"factor_info";  // "spt_fallback_db"; //
                                                        // "factor_info";
    exists_info = sql->os.get_prefetched_row_count();
    sql->os.flush();
    // printf("factor_info %d %s\n", exists, sql->err_msg.c_str());
    sql->os << (OTL_UNICODE_CHAR_TYPE*)L"factor_result";
    exists_result = sql->os.get_prefetched_row_count();
    sql->os.close();
    // printf("factor_result %d %s\n", exists, sql->err_msg.c_str());
    if (exists_info == 0) {
      sql->os.open(1,
                   "create table alpha.dbo.factor_info ("
                   "id bigint primary key identity,"
                   "name varchar(32) not null,"
                   "author varchar(32),"
                   "last_update varchar(32)"
                   ")",
                   sql->db);
      sql->os.close();
    }
    if (exists_result == 0) {
      sql->os.open(1,
                   "create table alpha.dbo.factor_result("
                   "id bigint primary key identity,"
                   "name varchar(32) not null,"
                   "code varchar(32) not null,"
                   "date int,"
                   "time int,"
                   "value float"
                   ")",
                   sql->db);
      sql->os.close();
    }
    // printf("create factor tables\n");
  } catch (otl_exception& e) {
    //    std::ostringstream os;
    //    os << e.msg << "\n\t" << e.stm_text << "\n\t" << e.sqlstate << "\n\t"
    //       << e.var_info << std::endl;
    //    sql->err_msg += os.str();
    //    printf("sql err %s\n", sql->err_msg.c_str());
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
  return 0;

  //  try {
  //    // 1. check factor info
  //    sql->os.open(1,
  //                 "select top 1 name from alpha.dbo.factor_info where "
  //                 "name=:f1<char[32]>",
  //                 sql->db);
  //    sql->os << info.factor_name;
  //    exists = sql->os.get_prefetched_row_count();
  //    sql->os.close();
  //    // printf("chck info\n");

  //    if (exists >= 1) {
  //      //  2. update factor info
  //      sql->os.open(1,
  //                   "update alpha.dbo.factor_info set
  //                   author=:f1<char[32],in>, " "last_update=:f2<char[32],in>
  //                   where name =:f3<char[32],in>", sql->db);
  //      sql->os << info.factor_author << info.factor_date << info.factor_name;
  //      sql->os.close();
  //    } else {
  //      //  3. insert factor info
  //      sql->os.open(1,
  //                   "insert into alpha.dbo.factor_info values(:f1<char[32]>,
  //                   "
  //                   ":f2<char[32]>, "
  //                   ":f3<char[32]>)",
  //                   sql->db);
  //      sql->os << info.factor_name << info.factor_author << info.factor_date;
  //      sql->os.close();
  //    }

  //    return 0;
  //  } catch (otl_exception& e) {
  //    // intercept OTL exceptions
  ////    std::ostringstream os;
  ////    os << e.msg << "\t" << e.stm_text << "\t" << e.sqlstate << "\t"
  ////       << e.var_info << std::endl;
  ////    sql->err_msg += os.str();
  ////    printf("sql err %s\n", sql->err_msg.c_str());
  //    return 1;
  //  }
}

int factor_result::store_result(const std::string& name, const std::string code,
                                const std::vector<lmapi_result_data>& dataset) {
  sql_internal* sql;
  std::ostringstream os;
  std::string value;
  int exists = 0;

  sql = reinterpret_cast<sql_internal*>(pdata);
  return 0;

  //  try {
  //    //  1. check factor result exists
  //    sql->os.close();
  //    sql->os.open(1,
  //                 "select top 1 name from alpha.dbo.factor_result where "
  //                 "name=:f1<char[32],in> and code=:f2<char[32],in>",
  //                 sql->db);
  //    sql->os << name << code;
  //    exists = sql->os.get_prefetched_row_count();
  //    // sql->os.flush();
  //    sql->os.close();

  //    //  2. delete existing result
  //    if (exists > 0) {
  //      sql->os.open(64,
  //                   "delete from alpha.dbo.factor_result where "
  //                   "name=:f1<char[32],in> and code=:f2<char[32],in>",
  //                   sql->db);
  //      sql->os << name << code;
  //      // sql->os.flush();
  //      sql->os.close();
  //    }

  //    //  3. insert factor result
  //    sql->os.open(256,  //"insert into alpha.dbo.factor_result
  //                       // values('aa', 'bb', 1, 2, 1.1) ",
  //                 " insert into alpha.dbo.factor_result "
  //                 "values(:name<char[32],in>,"
  //                 ":code<char[32],in>,"
  //                 ":f3<int,in>,"
  //                 ":f4<int,in>,"
  //                 ":f5<double,in>)",
  //                 sql->db);
  //    for (size_t i = 0; i < dataset.size(); ++i) {
  //      const lmapi_result_data& ds = dataset[i];
  //      sql->os << name << code << ds.date << ds.time << ds.value;
  //      //      std::ostringstream ss;
  //      //      ss << "insert into alpha.dbo.factor_result values (";
  //      //      ss << "'" << name << "','" << code << "'," << ds.date << ","
  //      <<
  //      //      ds.time
  //      //         << "," << std::setprecision(15) << ds.value << ")";
  //      // printf("%s\n", ss.str().c_str());

  //      // otl_cursor::direct_exec(sql->db, ss.str().c_str());
  //      // sql->os.open(1, ss.str().c_str(), sql->db);
  //      // sql->os.close();
  //    }

  //    printf("  store %s result %lu\n", code.c_str(), dataset.size());
  //    sql->os.close();

  //    return 0;
  //  } catch (otl_exception& e) {
  //    // intercept OTL exceptions
  ////    std::ostringstream os;
  ////    os << e.msg << "\t" << e.stm_text << "\t" << e.sqlstate << "\t"
  ////       << e.var_info << std::endl;
  ////    sql->err_msg += os.str();

  ////    printf("sql err %s\n", sql->err_msg.c_str());
  //    return 1;
  //  }
}

void factor_result::store(
    const std::vector<std::string>& stocks,
    const std::vector<std::vector<lmapi_result_data> >& results) {
  lmapi_result_info info;

  printf("1. store factor info\n");
  // 1. store info
  memset(&info, 0, sizeof(info));
  snprintf(info.factor_author, sizeof(info.factor_author), "lyalpha");
  snprintf(info.factor_name, sizeof(info.factor_name), "%s", factor.c_str());
  // ctime_r(0, info.factor_date);
  snprintf(info.factor_date, sizeof(info.factor_date), "2019-01-11");
  // printf("call store_factor\n");
  store_factor(info);

  printf("2. store factor result\n");
  // 2. store result
  for (size_t i = 0; i < stocks.size(); ++i) {
    const std::string& code = stocks[i];
    const std::vector<lmapi_result_data>& result = results[i];

    store_result(factor, code, result);
  }
}

}  // namespace lmapi
