/** Copyright 2018 He Hao<hehaoslj@sina.com> */

#include <string>
#include <vector>

enum lm_data_type {
    TICK_TYPE,
    K_30SEC_TYPE,
    K_1MIN_TYPE,
    K_5MIN_TYPE,
    K_1DAY_TYPE

};

struct lm_config_file{
    std::string factor_name;
    std::vector<std::string> instrument_array;
    std::string start_date;
    std::string end_date;
    lm_data_type data_type;
    int data_length;
    int data_step;

};
