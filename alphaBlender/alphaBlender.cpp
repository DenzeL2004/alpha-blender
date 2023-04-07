#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <assert.h>
#include <unistd.h>
#include <immintrin.h>

#include "../src/log_info/log_errors.h"
#include "../src/generals_func/generals.h"
#include "../src/draw/draw.h"

#include "alphaBlender.h"

#include "configBMP.h"

//==============================================================================

static void CombineImage(const char *back_buf, const char *front_buf, char *result_buf);

static char *LoadImage(const int fdin, const off_t offset);

static void PrintFPS(sf::RenderWindow *window, sf::Clock *fps_time, size_t *frame_cnt);

static void DisplayResult(sf::RenderWindow *window, const char *result_buf);


static int GetParamBMP (const int fdin, uint32_t *width, uint32_t *hight, off_t *data_offset);

//==============================================================================

int AlphaBlending(const char *back_img_name, const char *front_img_name)
{
    assert(back_img_name != nullptr && "back_img_name is nullptr");
    assert(front_img_name != nullptr && "front_img_name is nullptr");

    Image_info back_img = {};
    if (ImagInfoCtor(&back_img, back_img_name))
        return PROCESS_ERROR(ALPHA_BLENDING_ERR, "Load background picture from file failed\n");

    // char *front_buf = LoadImage(front_img_name, 0x36);
    // if (CheckNullptr(front_buf))
    //     return PROCESS_ERROR(ALPHA_BLENDING_ERR, "Load frontend picture from file failed\n");

    // char *result_buf = CreateAlignedBuffer(32, Window_hight * Window_width * 4);
    // if (CheckNullptr(result_buf))
    //     return PROCESS_ERROR(ALPHA_BLENDING_ERR, "Load frontend picture from file failed\n");

    // sf::RenderWindow window(sf::VideoMode(Window_width, Window_hight), "Mandelbrot");

    // size_t frame_cnt = 0;
    // sf::Clock fps_time;

    // int flag_draw_img = 1;

    // while (window.isOpen())
    // {
    //     sf::Event event;
    //     while (window.pollEvent(event))
    //     {
    //         if (event.type == sf::Event::Closed)
    //             window.close();
    //     }

    //     if (sf::Keyboard::isKeyPressed(sf::Keyboard::P))
    //     {
    //         flag_draw_img ^= 1;
    //         usleep(Delay);
    //     }

    //     if (flag_draw_img)
    //     {
    //         CombineImage(back_buf, front_buf, result_buf);
    //         DisplayResult(&window, result_buf);
    //     }
    //     else
    //     {
    //         window.clear();
    //         window.display();
    //     }

    //     PrintFPS(&window, &fps_time, &frame_cnt);
    // }

    // free(back_buf);
    // free(front_buf);
    // free(result_buf);

    return 0;
}

//===============================================================================

static void CombineImage(const char *back_buf, const char *front_buf, char *result_buf)
{
    assert(back_buf != nullptr && "back_buf is nullptr");
    assert(front_buf != nullptr && "front_buf is nullptr");

    assert(result_buf != nullptr && "result_buf is nullptr");

    memcpy(result_buf, back_buf, Window_hight * Window_width * 4);

    RGB_Quad back_pixel = {}, front_pixel = {};

    for (uint32_t yi = 0; yi < 126; yi++)
    {
        for (uint32_t xi = 0; xi < 235; xi++)
        {
            size_t back_it  = (yi * Window_width + xi) * 4;
            size_t front_it = (yi * 235 + xi) * 4;

            memcpy(&back_pixel,  back_buf  + back_it, 4);
            memcpy(&front_pixel, front_buf + front_it, 4);

            uint8_t alpha = front_pixel.rgbAlpha;

            front_pixel.rgbAlpha = (uint8_t) ((alpha * front_pixel.rgbAlpha + (255 - alpha) * back_pixel.rgbAlpha) >> 8);
            front_pixel.rgbGreen = (uint8_t) ((alpha * front_pixel.rgbGreen + (255 - alpha) * back_pixel.rgbGreen) >> 8);
            front_pixel.rgbBlue  = (uint8_t) ((alpha * front_pixel.rgbBlue  + (255 - alpha) * back_pixel.rgbBlue ) >> 8);
            front_pixel.rgbRed   = (uint8_t) ((alpha * front_pixel.rgbRed   + (255 - alpha) * back_pixel.rgbRed  ) >> 8);

            memcpy(result_buf + back_it, &front_pixel, 4);
        }
    }

    return;
}

//===============================================================================

static void DisplayResult(sf::RenderWindow *window, const char *result_buf)
{
    assert(result_buf != nullptr && "front_buf is nullptr");
    assert(window != nullptr && "window is nullptr");

    sf::Image img = {};
    img.create(Window_width, Window_hight, sf::Color::Green);

    size_t it = 0;

    for (uint32_t yi = 0; yi < Window_hight; yi++)
    {
        for (uint32_t xi = 0; xi < Window_width; xi++)
        {
            uint32_t x = Window_width - xi - 1;
            uint32_t y = Window_hight - yi - 1;
            img.setPixel(x, y, sf::Color(   result_buf[it + 2], 
                                            result_buf[it + 1], 
                                            result_buf[it + 0],
                                            result_buf[it + 3]));

            //printf("%x %x %x %x\n", result_buf[it + 0], result_buf[it + 1], result_buf[it + 2], result_buf[it + 3]);
            //    usleep(Delay * 10);

            it += 4;
        }
    }

    DisplayImage(window, &img);

    return;
}

//===============================================================================

int ImagInfoCtor (Image_info *img_str, const char *file_name)
{
    assert (img_str   != nullptr && "img_str is nullptr");
    assert (file_name != nullptr && "img_str is nullptr");

    int fdin = OpenFileDescriptor(file_name, O_RDWR);
    if (fdin <= 0)
        return PROCESS_ERROR(IMAGE_INFO_CTOR_ERR, "open file \'%s\' discriptor = %d failed\n", 
                                                            file_name,          fdin);
    
    img_str->hight       = 0;
    img_str->width       = 0;
    img_str->data_offset = 0;

    if (GetParamBMP(fdin, &img_str->width, &img_str->hight,  &img_str->data_offset))
        return PROCESS_ERROR(IMAGE_INFO_CTOR_ERR, "Get param from bmp file failed");


    img_str->pixel_data = LoadImage(fdin, img_str->data_offset);
    if (CheckNullptr(img_str->pixel_data))
        return PROCESS_ERROR(ALPHA_BLENDING_ERR, "Load background picture from file failed\n");


    if (CloseFileDescriptor(fdin))
        return PROCESS_ERROR(IMAGE_INFO_CTOR_ERR, "close file \'%s\' discriptor = %d failed\n",
                                                            file_name,          fdin);
    return 0;
}

//===============================================================================

static int GetParamBMP (const int fdin, 
                 uint32_t *width, uint32_t *hight, off_t *data_offset)
{
    assert(fdin >= 0 && "file descriptor isn't positive number");

    assert(width        !=  nullptr && "width is nullptr");
    assert(hight        !=  nullptr && "highr is nullptr");
    assert(data_offset  !=  nullptr && "data_offset is nullptr");

    size_t read_num = 0;

    read_num = pread(fdin, width, BYTE * 4, Offset_width);
    if (read_num != BYTE * 4)
        return PROCESS_ERROR(GET_PARAM_BMP_ERR, "read width file failed. Was readden %lu", read_num);

    read_num = pread(fdin, hight, BYTE * 4, Offset_hight);
    if (read_num != BYTE * 4)
        return PROCESS_ERROR(GET_PARAM_BMP_ERR, "read hight file failed. Was readden %lu", read_num);

    read_num = pread(fdin, data_offset, BYTE * 4, Offset_data);
    if (read_num != BYTE * 4)
        return PROCESS_ERROR(GET_PARAM_BMP_ERR, "read data offset file failed. Was readden %lu", read_num);

    return 0;
}

//===============================================================================


int ImagInfoDtor (Image_info *img_str)
{
    assert (img_str   != nullptr && "img_str is nullptr");

    
    img_str->hight       = 0;
    img_str->width       = 0;
    img_str->data_offset = 0;

    free (img_str->pixel_data);

    return 0;
}

//===============================================================================

static char *LoadImage(const int fdin, const off_t offset)
{
    assert(fdin >= 0 && "file descriptor isn't positive number");

    struct stat file_info = {};
    fstat(fdin, &file_info);

    size_t img_size = file_info.st_size - offset;

    char *buffer = (char*)CreateAlignedBuffer(32, img_size);
    if (CheckNullptr(buffer))
    {
        PROCESS_ERROR(ERR_FILE_OPEN, "allocate memory to file discriptor = %d failed\n", fdin);
        return nullptr;
    }

    size_t read_num = pread(fdin, buffer, img_size, offset);
    if (read_num != img_size)
    {
        PROCESS_ERROR(ERR_FILE_OPEN,    "read from file failed.\n",
                                        "was readen %lu, must %lu",
                                        read_num, img_size);
        return nullptr;
    }

    return buffer;
}

//===============================================================================

static void PrintFPS(sf::RenderWindow *window, sf::Clock *fps_time, size_t *frame_cnt)
{
    assert(window != nullptr && "window is nullptr");

    assert(fps_time != nullptr && "fps_time is nullptr");
    assert(frame_cnt != nullptr && "frame_cnt is nullptr");

    float cur_fps = GetFPS(fps_time, frame_cnt);

    char buf[Buffer_size] = {};
    sprintf(buf, "%.2f FPS", cur_fps * Accuracy);
    (*window).setTitle(buf);

    return;
}

//===============================================================================