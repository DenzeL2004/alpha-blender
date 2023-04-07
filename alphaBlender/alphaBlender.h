#ifndef _MANDELBROT_H_
#define _MANDELBROT_H_

#include <stdint.h>
#include <stddef.h>

#include <immintrin.h>

//===============================================================================

const size_t Accuracy = 1;

const size_t Delay    = 200000;  

//===============================================================================

struct RGB_Quad
{
    uint8_t rgbBlue    = 0;
    uint8_t rgbGreen   = 0;
    uint8_t rgbRed     = 0;

    uint8_t rgbAlpha   = 0;

};

//===============================================================================

struct Image_info
{
    uint32_t hight = 0;
    uint32_t width = 0;

    char *pixel_data  = nullptr;

};


//===============================================================================

enum Alpha_Blende_ERR
{
    LOAD_IMG_ERR        =   -1,

    ALPHA_BLENDING_ERR  =   -2,

    IMAGE_INFO_CTOR_ERR =   -3,
    IMAGE_INFO_DTOR_ERR =   -4,

    GET_PARAM_BMP_ERR   =   -5,

};    

//===============================================================================

int AlphaBlending(const char *back_img_name, const char *front_img_name, const char *result_img_name,
                        const uint32_t x_start, const uint32_t y_start);

int ImagInfoCtor(Image_info *img_str, const char *file_name);


int ImagInfoDtor(Image_info *img_str);

//===============================================================================

#endif