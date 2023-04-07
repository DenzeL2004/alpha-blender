#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <assert.h>
#include <unistd.h>
#include <immintrin.h>

#include "../src/log_info/log_errors.h"
#include "../src/generals_func/generals.h"
#include "../src/draw/draw.h"

#include "alphaBlender.h"

//==============================================================================
static void CombineImage    (const Image_info *back_img, const Image_info *front_img, 
                             const Image_info *result_img,
                             const uint32_t x_start, const uint32_t y_start);

static char *LoadImage      (const char *file_name, 
                             uint32_t *width, uint32_t *hight);

static void PrintFPS        (sf::RenderWindow *window, sf::Clock *fps_time, size_t *frame_cnt);

static void DrawImage       (const Image_info *result_img, sf::Image *img);

static void SaveImage       (const char *result_file_name, const sf::Image *result_img);


//==============================================================================

int AlphaBlending(const char *back_img_name, const char *front_img_name, const char *result_img_name, 
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

    size_t frame_cnt = 0;
    sf::Clock fps_time;

    int flag_draw_img = 1;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::P))
        {
            flag_draw_img ^= 1;
            usleep(Delay);
        }

        CombineImage(&back_img, &front_img, &result_img, x_start, y_start);

        if (flag_draw_img)
        {
            DrawImage(&result_img, &img);
            DisplayImage(&window, &img);
        }
        else
        {
            window.clear();
            window.display();
        }

        PrintFPS(&window, &fps_time, &frame_cnt);
    }

    SaveImage(result_img_name, &img);


    ImagInfoDtor(&back_img);
    ImagInfoDtor(&front_img);
    ImagInfoDtor(&result_img);

    return 0;
}

//===============================================================================

static void CombineImage(const Image_info *back_img, const Image_info *front_img, 
                                                     const Image_info *result_img,
                         const uint32_t x_start, const uint32_t y_start)
{
    assert(back_img != nullptr && "back_img is nullptr");
    assert(front_img != nullptr && "front_img is nullptr");

    assert(result_img != nullptr && "result_img is nullptr");

    //memcpy(result_img->pixel_data, back_img->pixel_data, back_img->hight * back_img->width * DWORD);

    RGB_Quad back_pixel = {}, front_pixel = {};

    for (uint32_t yi = 0; yi < front_img->hight; yi++)
    {
        for (uint32_t xi = 0; xi < front_img->width; xi++)
        {
            size_t back_it  = ((yi + y_start) * back_img->width  + (xi + x_start)) * DWORD;
            size_t front_it = (yi * front_img->width + xi) * DWORD;

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

    return;
}

//===============================================================================

static void DrawImage(const Image_info *result_img, sf::Image *img)
{
    assert(result_img != nullptr && "front_buf is nullptr");
    assert(img        != nullptr && "img is nullptr");

    size_t it = 0;

    for (uint32_t yi = 0; yi < result_img->hight; yi++)
    {
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


    img_str->pixel_data = LoadImage(file_name, &img_str->width, &img_str->hight);
    if (CheckNullptr(img_str->pixel_data))
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

static char *LoadImage( const char *file_name, 
                        uint32_t *width, uint32_t *hight)
{
    assert(file_name != nullptr && "file name is nullptr");

    sf::Image img = {};
    img.loadFromFile(file_name);

    sf::Vector2u img_size = img.getSize();

    *width = img_size.x;
    *hight = img_size.y;

    uint32_t size = (*width) * (*hight) * DWORD;

    char *buffer = (char*)CreateAlignedBuffer(32, size);
    if (CheckNullptr(buffer))
    {
        PROCESS_ERROR(ERR_FILE_OPEN, "allocate memory to file \'%s\' failed\n", file_name);
        return nullptr;
    }

    memcpy(buffer, img.getPixelsPtr(), size);

    return buffer;
}


//===============================================================================

static void SaveImage(const char *result_file_name, const sf::Image *result_img)
{
    assert(result_file_name != nullptr && "file name is nullptr");
    assert(result_img != nullptr && "result_img is nullptr");

    open(result_file_name, O_WRONLY);
    (*result_img).saveToFile(result_file_name);

    return;
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