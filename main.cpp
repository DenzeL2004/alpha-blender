#include <stdio.h>
#include <stdlib.h>

#include "src/log_info/log_errors.h"
#include "src/generals_func/generals.h"

#include "alphaBlender/alphaBlender.h"

int main ()
{  
    #ifdef USE_LOG
        if (OpenLogsFile()) 
            return OPEN_FILE_LOG_ERR;
    #endif

    AlphaBlending("src/img/Table.bmp", "src/img/AskhatCat.bmp", "src/img/result.bmp", 300, 200);

    #ifdef USE_LOG
        if (CloseLogsFile ())
            return OPEN_FILE_LOG_ERR;
    #endif

    return EXIT_SUCCESS;
}