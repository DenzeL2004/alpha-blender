#ifndef _LOG_ERRORS_H_
#define _LOG_ERRORS_H_

#include <stdio.h>

#include "log_def.h"

enum Log_errors
{
    OPEN_FILE_LOG_ERR  = -1, 
    CLOSE_FILE_LOG_ERR = -2    
};

#define USE_LOG


#ifdef USE_LOG
        
    #define Log_report(...)                            \
        Log_report_ (LOG_ARGS, __VA_ARGS__)


#else

    #define Log_report(...)           

#endif


#define Err_report(...)                                             \
        Err_report_ (LOG_ARGS, __VA_ARGS__)

#define PROCESS_ERROR(error, ...)                                   \
        Process_error_ (#error, error, LOG_ARGS, __VA_ARGS__)                              


int OpenLogsFile ();

int CloseLogsFile ();

FILE *Get_log_file_ptr ();


int Log_report_ (LOG_PARAMETS, const char *format, ...);


int Err_report_ (LOG_PARAMETS, const char *format, ...);

int Process_error_ (const char* error_name, const int error_val, 
                    LOG_PARAMETS, const char *format, ...);


#endif // #endif _LOG_ERRORS_H_