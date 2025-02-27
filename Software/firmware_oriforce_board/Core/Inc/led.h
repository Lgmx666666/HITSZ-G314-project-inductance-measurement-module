//
// Created by LiangGuang on 2023/5/30.
//

#ifndef ISENSOR_LED_H
#define ISENSOR_LED_H

#include "main.h"

typedef enum{
    BLACK = 0,
    RED,
    BLUE,
    GREEN,
    WHITE
}led_color_e;

void led_status(led_color_e COLOR);

#endif //ISENSOR_LED_H
