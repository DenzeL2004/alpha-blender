#ifndef _DRAW_H_
#define _DRAW_H_

#include <SFML/Graphics.hpp>

const size_t Buffer_size = 100;

//==================================================================================================

void DrawImage (sf::RenderWindow *window, const sf::Image *img);

float GetFPS (sf::RenderWindow *window, sf::Clock *fps_time, size_t *frame_cnt);

//==================================================================================================

#endif