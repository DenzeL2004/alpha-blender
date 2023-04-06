#ifndef _MANDELBROT_H_
#define _MANDELBROT_H_

#include <stdint.h>
#include <stddef.h>

#include <immintrin.h>

//===============================================================================



const uint32_t Window_hight  = 600;
const uint32_t Window_width  = 800;

const size_t Accuracy = 100;

const size_t Delay    = 100000;  

const size_t BMP_offset = 54;

//===============================================================================



//===============================================================================

enum Alpha_Blende_ERR
{
    LOAD_IMG_ERR        =  -1,

    ALPHA_BLENDING_ERR  =  -2,

};    

//===============================================================================

struct RGB_Quad
{
    uint8_t rgbBlue         = 0;
    uint8_t rgbGreen        = 0;
    uint8_t rgbRed          = 0;

    uint8_t rgbBrightness   = 0;

};

//===============================================================================

int AlphaBlending(const char *back_img_name, const char *front_img_name);

//===============================================================================

#endif