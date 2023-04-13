#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <assert.h>
#include <unistd.h>
#include <immintrin.h>

#include "../src/log_info/log_errors.h"
#include "../src/generals_func/generals.h"
#include "../src/draw/draw.h"

#include "../config.h"

#include "alphaBlender.h"

//==============================================================================
static float    CombineImage    (const Image_info *back_img, const Image_info *front_img, 
                                 const Image_info *result_img,
                                 uint32_t x_start, uint32_t y_start);

static int      LoadImage       (const char *file_name, Image_info *img_str);

static void     DrawImage       (const Image_info *result_img, sf::Image *img);

static void     SaveImage       (const char *result_file_name, const sf::Image *result_img);


//==============================================================================

int AlphaBlending(  const char *back_img_name, const char *front_img_name, 
                    const char *result_img_name, 
                    const uint32_t x_start, const uint32_t y_start)
{
    assert(back_img_name   != nullptr && "back_img_name is nullptr");
    assert(front_img_name  != nullptr && "front_img_name is nullptr");
    assert(result_img_name != nullptr && "result_img_name is nullptr");

    Image_info back_img = {};
    if (ImagInfoCtor(&back_img, back_img_name))
        return PROCESS_ERROR(ALPHA_BLENDING_ERR, "Load background picture from file failed\n");

    Image_info front_img = {};
    if (ImagInfoCtor(&front_img, front_img_name))
        return PROCESS_ERROR(ALPHA_BLENDING_ERR, "Load front picture from file failed\n");

    Image_info result_img = {};
    if (ImagInfoCtor(&result_img, back_img_name))
        return PROCESS_ERROR(ALPHA_BLENDING_ERR, "Load background picture from file failed\n");

    sf::RenderWindow window(sf::VideoMode(back_img.width, back_img.hight), "Blending");

    sf::Image img = {};
    img.create(result_img.width, result_img.hight, sf::Color::Black);


    float time = CombineImage(&back_img, &front_img, &result_img, x_start, y_start);
    printf("all passed time: %.3f s\n", time);

    DrawImage(&result_img, &img);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        DisplayImage(&window, &img);
    }

    SaveImage(result_img_name, &img);

    ImagInfoDtor(&back_img);
    ImagInfoDtor(&front_img);
    ImagInfoDtor(&result_img);

    return 0;
}

//===============================================================================

static float CombineImage(const Image_info *back_img, const Image_info *front_img, 
                                                     const Image_info *result_img,
                         uint32_t x_start, uint32_t y_start)
{
    assert(back_img != nullptr && "back_img is nullptr");
    assert(front_img != nullptr && "front_img is nullptr");

    assert(result_img != nullptr && "result_img is nullptr");
   
    sf::Clock time;

    for (size_t count = 0; count < Accuracy; count++)
    {
        #ifdef OPTIMIZE

        x_start = (x_start / 32) * 32;

        const uint8_t Zero_mask = 255u;
        const uint8_t Max_aplha = 255u;

        size_t back_it  = 0;
        size_t front_it = 0;

        for (uint32_t yi = 0; yi < front_img->hight; yi++)
        {
            back_it  = ((yi + y_start) * back_img->patch_width + x_start) * DWORD;
            front_it = (yi * front_img->patch_width) * DWORD;

            for (uint32_t xi = 0; xi < front_img->patch_width; xi += 8, back_it += 32, front_it += 32)
            {
                //load current eight pixel
                __m256i front = _mm256_load_si256((__m256i*) (front_img->pixel_data + front_it));
                __m256i back  = _mm256_load_si256((__m256i*) (back_img->pixel_data  + back_it));

                //separate high and low bytes
                __m256i back_l = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(back, 0));
                __m256i back_h = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(back, 1));

                __m256i front_l = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(front, 0));;
                __m256i front_h = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(front, 1));


                //Get alpha for each pixel
                __m256i move_mask = _mm256_set_epi8(Zero_mask, 14, Zero_mask, 14, Zero_mask, 14, Zero_mask, 14,  
                                                    Zero_mask,  6, Zero_mask,  6, Zero_mask,  6, Zero_mask,  6,
                                                    Zero_mask, 14, Zero_mask, 14, Zero_mask, 14, Zero_mask, 14,  
                                                    Zero_mask,  6, Zero_mask,  6, Zero_mask,  6, Zero_mask,  6);

                __m256i alpha_l = _mm256_shuffle_epi8(front_l, move_mask);
                __m256i alpha_h = _mm256_shuffle_epi8(front_h, move_mask);

                //front * alpha
                front_l = _mm256_mullo_epi16(front_l, alpha_l);
                front_h = _mm256_mullo_epi16(front_h, alpha_h);

                //back * (255 - alpha)
                back_l = _mm256_mullo_epi16(back_l, _mm256_subs_epu16(_mm256_set1_epi16(Max_aplha), alpha_l));
                back_h = _mm256_mullo_epi16(back_h, _mm256_subs_epu16(_mm256_set1_epi16(Max_aplha), alpha_h));

                //back + front
                __m256i sum_l = _mm256_add_epi16(front_l, back_l);
                __m256i sum_h = _mm256_add_epi16(front_h, back_h);


                move_mask = _mm256_set_epi8(15,   13,   11,    9,    7,    5,    3,    1, 
                                            Zero_mask, Zero_mask, Zero_mask, Zero_mask, Zero_mask, Zero_mask, Zero_mask, Zero_mask,
                                            Zero_mask, Zero_mask, Zero_mask, Zero_mask, Zero_mask, Zero_mask, Zero_mask, Zero_mask,   
                                            15,   13,   11,    9,    7,    5,    3,    1);


                //index: | 31 | 30 | 29 | 28 | 27 | 26 | 25 | 24 | 23 | 22 | 21 | 20 | 19 | 18 | 17 | 16 |
                //value: | a3 | r3 | g3 | b3 | a2 | r2 | g2 | b2 | 00 | 00 | 00 | 00 | 00 | 00 | 00 | 00 |

                //index: | 15 | 14 | 13 | 12 | 11 | 10 | 09 | 08 | 07 | 06 | 05 | 04 | 03 | 02 | 01 | 00 |
                //value: | 00 | 00 | 00 | 00 | 00 | 00 | 00 | 00 | a1 | r1 | g1 | b1 | a0 | r0 | g0 | b0 |

                sum_l = _mm256_shuffle_epi8(sum_l, move_mask);
                sum_h = _mm256_shuffle_epi8(sum_h, move_mask);


                //index: | 15 | 14 | 13 | 12 | 11 | 10 | 09 | 08 | 07 | 06 | 05 | 04 | 03 | 02 | 01 | 00 |
                //
                //value: | a3 | r3 | g3 | b3 | a2 | r2 | g2 | b2 | 00 | 00 | 00 | 00 | 00 | 00 | 00 | 00 |
                //      +        
                //value: | 00 | 00 | 00 | 00 | 00 | 00 | 00 | 00 | a1 | r1 | g1 | b1 | a0 | r0 | g0 | b0 |
                
                __m256i colors = _mm256_set_m128i(  _mm_add_epi8(_mm256_extracti128_si256(sum_h, 0), _mm256_extracti128_si256(sum_h, 1)),
                                                    _mm_add_epi8(_mm256_extracti128_si256(sum_l, 0), _mm256_extracti128_si256(sum_l, 1)));
                
    
                _mm256_store_si256((__m256i*)(result_img->pixel_data + back_it), colors);
            }
        }

        #else

        RGB_Quad back_pixel = {}, front_pixel = {};

        size_t back_it  = 0;
        size_t front_it = 0;

        for (uint32_t yi = 0; yi < front_img->hight; yi++)
        {
            back_it  = ((yi + y_start) * back_img->patch_width + (x_start)) * DWORD;
            front_it = (yi * front_img->patch_width) * DWORD;
            
            for (uint32_t xi = 0; xi < front_img->patch_width; xi++, back_it += DWORD,front_it += DWORD)
            {
                memcpy(&back_pixel,  back_img->pixel_data  + back_it,  DWORD);
                memcpy(&front_pixel, front_img->pixel_data + front_it, DWORD);

                uint8_t alpha = front_pixel.rgbAlpha;

                front_pixel.rgbAlpha = (uint8_t) ((alpha * front_pixel.rgbAlpha + (255 - alpha) * back_pixel.rgbAlpha) >> 8);
                front_pixel.rgbGreen = (uint8_t) ((alpha * front_pixel.rgbGreen + (255 - alpha) * back_pixel.rgbGreen) >> 8);
                front_pixel.rgbBlue  = (uint8_t) ((alpha * front_pixel.rgbBlue  + (255 - alpha) * back_pixel.rgbBlue ) >> 8);
                front_pixel.rgbRed   = (uint8_t) ((alpha * front_pixel.rgbRed   + (255 - alpha) * back_pixel.rgbRed  ) >> 8);

                memcpy(result_img->pixel_data + back_it, &front_pixel, DWORD);
            }
        }

        #endif
    }

    return time.getElapsedTime().asSeconds();
}

//===============================================================================

static void DrawImage(const Image_info *result_img, sf::Image *img)
{
    assert(result_img != nullptr && "front_buf is nullptr");
    assert(img        != nullptr && "img is nullptr");

    size_t it = 0;
 
    for (uint32_t yi = 0; yi < result_img->hight; yi++)
    {
        it = yi * result_img->patch_width * DWORD;
        for (uint32_t xi = 0; xi < result_img->width; xi++)
        {
            (*img).setPixel(xi, yi, sf::Color(  result_img->pixel_data[it + 0], 
                                                result_img->pixel_data[it + 1], 
                                                result_img->pixel_data[it + 2],
                                                result_img->pixel_data[it + 3]));
            it += 4;
        }
    }

    return;
}

//===============================================================================

int ImagInfoCtor(Image_info *img_str, const char *file_name)
{
    assert (img_str   != nullptr && "img_str is nullptr");
    assert (file_name != nullptr && "img_str is nullptr");

    img_str->hight       = 0;
    img_str->width       = 0;

    img_str->patch_width = 0;


    if(LoadImage(file_name, img_str))
        return PROCESS_ERROR(IMAGE_INFO_CTOR_ERR, "Load picture from file \'%s\' failed\n", file_name);

    return 0;
}

//===============================================================================

int ImagInfoDtor(Image_info *img_str)
{
    assert (img_str   != nullptr && "img_str is nullptr");

    img_str->hight       = 0;
    img_str->width       = 0;

    free (img_str->pixel_data);

    return 0;
}

//===============================================================================

static int LoadImage(const char *file_name, Image_info *img_str)
{
    assert(file_name != nullptr && "file name is nullptr");
    assert(img_str   != nullptr && "img_str is nullptr");

    sf::Image img = {};
    img.loadFromFile(file_name);

    sf::Vector2u img_size = img.getSize();

    img_str->width = img_size.x;
    img_str->hight = img_size.y;

    img_str->patch_width = (img_str->width / 32 + 1) * 32;

    uint32_t size = img_str->patch_width * img_str->hight * DWORD;

    img_str->pixel_data = (char*) CreateAlignedBuffer(32, size);
    if (CheckNullptr(img_str->pixel_data))
        return PROCESS_ERROR(ERR_FILE_OPEN, "allocate memory to file \'%s\' failed\n", file_name);
    
    char *pixel_ptr = (char*) img.getPixelsPtr();

    size_t lowe_width = (img_str->width / 32) * 32;

    for (size_t yi = 0; yi < img_str->hight; yi++)
    {
        size_t new_offset = yi * img_str->patch_width * DWORD;

        for (size_t xi = 0; xi < lowe_width; xi += 8, pixel_ptr += 32, new_offset += 32)
        {
            __m256i data = _mm256_loadu_si256((__m256i*) (pixel_ptr));
            _mm256_storeu_si256((__m256i*) (img_str->pixel_data + new_offset), data); 
        }           

        size_t cnt = img_str->width % 32;

        memcpy(img_str->pixel_data + new_offset, pixel_ptr, cnt * DWORD);
        pixel_ptr += cnt * DWORD;
    }

    return 0;
}


//===============================================================================

static void SaveImage(const char *result_file_name, const sf::Image *result_img)
{
    assert(result_file_name != nullptr && "file name is nullptr");
    assert(result_img != nullptr && "result_img is nullptr");

    int fd = open(result_file_name, O_WRONLY);
    if (fd <= 0)
    {
        PROCESS_ERROR(SAVE_IMAGE_ERR, "save picture to file descriptor: %d failed\n", fd);
        return;
    }

    (*result_img).saveToFile(result_file_name);

    close(fd);

    return;
}

//===============================================================================