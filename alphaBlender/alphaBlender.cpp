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

static void CombineImage(const sf::Image *back_img, const sf::Image *front_img, sf::Image *img);

static char *LoadImage(const char *file_name, const off_t offset);

static int   FreeImage(const char *file_name, char *virtual_buf);

//==============================================================================

int AlphaBlending(const char *back_img_name, const char *front_img_name)
{
    assert(back_img_name  != nullptr && "back_img_name is nullptr");
    assert(front_img_name != nullptr && "front_img_name is nullptr");

    sf::Image back_img;
    if (!back_img.loadFromFile(back_img_name))
        return PROCESS_ERROR(ALPHA_BLENDING_ERR, "Load background picture from file failed\n");

    sf::Image front_img = {};
    if (!front_img.loadFromFile(front_img_name))
        return PROCESS_ERROR(ALPHA_BLENDING_ERR, "Load front picture from file failed\n");

    sf::RenderWindow window(sf::VideoMode(Window_width, Window_hight), "Mandelbrot");

    sf::Image img = {};
    img.create(Window_width, Window_hight, sf::Color::Black);

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
        
        if (flag_draw_img)
        {
            CombineImage(&back_img, &front_img, &img);
            DrawImage(&window, &img);
        }
        else
        {
            window.clear();
            window.display();
        }

        float cur_fps = GetFPS(&window, &fps_time, &frame_cnt);

        char buf[Buffer_size] = {0};
        sprintf(buf, "%.2f FPS", cur_fps * Accuracy);
        window.setTitle(buf);
    }
    
    return 0;
}

//===============================================================================

static void CombineImage(const sf::Image *back_img, const sf::Image *front_img, sf::Image *img)
{
    assert(back_img  != nullptr && "back_buf is nullptr");
    assert(front_img != nullptr && "front_buf is nullptr");
    
    assert(img != nullptr && "img is nullptr");

    sf::Color back_pixel = {}, front_pixel = {};

    for (uint32_t yi = 0; yi < Window_hight; yi++)
    {
        for (uint32_t xi = 0; xi < Window_width; xi++)
        {
            back_pixel  = back_img->getPixel(xi, yi);
            front_pixel = front_img->getPixel(xi, yi);
            

            uint8_t brightness = front_pixel.a;

            front_pixel.r = (brightness * front_pixel.r + (255 - brightness) * back_pixel.r) >> 8;
            front_pixel.g = (brightness * front_pixel.g + (255 - brightness) * back_pixel.g) >> 8;
            front_pixel.b = (brightness * front_pixel.b + (255 - brightness) * back_pixel.b) >> 8;
            front_pixel.a = (brightness * front_pixel.a + (255 - brightness) * back_pixel.a) >> 8;

            img->setPixel(xi, yi, front_pixel);
        }    
    }

    return;
} 

//===============================================================================

static char *LoadImage(const char *file_name, const off_t offset)
{
    assert(file_name != nullptr && "file_name is nullptr");

    int fdin = OpenFileDescriptor(file_name, O_RDWR);
    if (fdin <= 0)
    {
        PROCESS_ERROR(ERR_FILE_OPEN, "open file \'%s\' discriptor = %d failed\n", 
                                                file_name,         fdin);
        return nullptr;
    }  

    // struct stat file_info = {};
    // fstat (fdin, &file_info);

    // printf ("%llu\n", file_info.st_size);

    // char *virtual_buf = (char*) calloc(file_info.st_size, sizeof(char));
    // assert(virtual_buf != nullptr);

    // size_t cnt_read = pread(fdin, virtual_buf, file_info.st_size, offset);
    // printf("%llu\n", cnt_read);

    char *virtual_buf =  CreateVirtualBuf(fdin, PROT_READ, offset);
    if (CheckNullptr(virtual_buf))
    {
        PROCESS_ERROR(ERR_CREATE_VIRTUAL_BUF, "create virtual buffer failed. discriptor = %d\n", fdin);
        return nullptr;
    }

    // printf ("|%p|\n", virtual_buf);

    if (CloseFileDescriptor(fdin))
    {
        PROCESS_ERROR(ERR_FILE_CLOSE, "close file \'%s\' discriptor = %d failed\n", 
                                                  file_name,         fdin);
        return nullptr;
    }

    return virtual_buf;
}

//===============================================================================

static int FreeImage(const char *file_name, char *virtual_buf)
{
    assert(file_name   != nullptr && "file_name is nullptr");
    assert(virtual_buf != nullptr && "virtual_buf is nullptr");

    int fdin = OpenFileDescriptor(file_name, O_RDWR);
    if (fdin <= 0)
    {
        return PROCESS_ERROR(ERR_FILE_OPEN, "open file \'%s\' discriptor = %d failed\n", 
                                                        file_name,         fdin);
    }  

    if (FreeVirtualBuf(fdin, virtual_buf))
        return PROCESS_ERROR(ERR_FREE_VIRTUAL_BUF, "free virtual buffer failed\n");

    if (CloseFileDescriptor(fdin))
    {
        return PROCESS_ERROR(ERR_FILE_CLOSE, "close file \'%s\' discriptor = %d failed\n", 
                                                        file_name,         fdin);
    }

    return 0;
}

//===============================================================================