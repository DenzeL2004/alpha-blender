#include "draw.h"

#include "../generals_func/generals.h"
#include "../log_info/log_errors.h"

//===========================================================================================

void DisplayImage (sf::RenderWindow *window, const sf::Image *img)
{
    assert (window != nullptr && "window is nullptr");

    sf::Texture texture;
    texture.loadFromImage(*img);

    sf::Sprite sprite;
    sprite.setTexture(texture);

    (*window).draw(sprite);
    (*window).display();

    return;
}

//===========================================================================================
 
float GetFPS (sf::Clock *fps_time, size_t *frame_cnt)
{
    assert (fps_time    != nullptr && "fps_time is nullptr");
    assert (frame_cnt   != nullptr && "frame_cnt is nullptr");

    *frame_cnt += 1;

    float cur_time = (*fps_time).getElapsedTime().asSeconds();
    float fps = (float)(*frame_cnt) / (cur_time);

    if (cur_time > 1.f || *frame_cnt == SIZE_MAX)
    {
        (*fps_time).restart();
        *frame_cnt = 0;
    }

    return fps;
}

//===========================================================================================