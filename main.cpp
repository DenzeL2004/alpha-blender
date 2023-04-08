#include <stdio.h>
#include <stdlib.h>

#include "src/log_info/log_errors.h"
#include "src/generals_func/generals.h"

#include "alphaBlender/alphaBlender.h"

int main (int argc, char *argv[])
{  
    #ifdef USE_LOG
        if (OpenLogsFile()) 
            return OPEN_FILE_LOG_ERR;
    #endif

    if (argc != 6)
        return PROCESS_ERROR(EXIT_FAILURE, "incorrect number of parameters\n");

    char *back_img_name   = argv[1];
    char *front_img_name  = argv[2];
    char *result_img_name = argv[3];

    uint32_t coord_x = atoi(argv[4]);
    uint32_t coord_y = atoi(argv[5]);

    AlphaBlending(back_img_name, front_img_name, result_img_name, coord_x, coord_y);

    #ifdef USE_LOG
        if (CloseLogsFile ())
            return OPEN_FILE_LOG_ERR;
    #endif

    return EXIT_SUCCESS;
}