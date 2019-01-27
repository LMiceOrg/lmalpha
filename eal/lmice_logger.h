#ifndef LMICE_LOGGER_H
#define LMICE_LOGGER_H


enum lmice_logger_type_e
{
    lmice_logger_text =     0,

    lmice_logger_none =     1<<0,
    lmice_logger_info =     1<<1,
    lmice_logger_debug =    1<<2,
    lmice_logger_warning =  1<<3,
    lmice_logger_error =    1<<4,
    lmice_logger_critical = 1<<5,

    lmice_logger_bson =     1<<6
};
typedef enum lmice_logger_type_e lmice_logger_type_t;

/** supported formats like-as c-printf
*/
void lmice_log(lmice_logger_type_t log_type, const char* format, ...);

#endif /** LMICE_LOGGER_H */
