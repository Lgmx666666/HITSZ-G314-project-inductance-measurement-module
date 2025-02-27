//
// Created by LiangGuang on 2023/5/30.
//

#include "../Inc/led.h"


void led_status(led_color_e COLOR){
    switch (COLOR) {
        case BLACK:
            HAL_GPIO_WritePin(DRED_GPIO_Port, DRED_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(DBLUE_GPIO_Port, DBLUE_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(DGREEN_GPIO_Port, DGREEN_Pin, GPIO_PIN_SET);
            break;
        case RED:
            HAL_GPIO_WritePin(DRED_GPIO_Port, DRED_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(DBLUE_GPIO_Port, DBLUE_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(DGREEN_GPIO_Port, DGREEN_Pin, GPIO_PIN_SET);
            break;
        case BLUE:
            HAL_GPIO_WritePin(DRED_GPIO_Port, DRED_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(DBLUE_GPIO_Port, DBLUE_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(DGREEN_GPIO_Port, DGREEN_Pin, GPIO_PIN_RESET);
            break;
        case GREEN:
            HAL_GPIO_WritePin(DRED_GPIO_Port, DRED_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(DBLUE_GPIO_Port, DBLUE_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(DGREEN_GPIO_Port, DGREEN_Pin, GPIO_PIN_SET);
            break;
        case WHITE:
            HAL_GPIO_WritePin(DRED_GPIO_Port, DRED_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(DBLUE_GPIO_Port, DBLUE_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(DGREEN_GPIO_Port, DGREEN_Pin, GPIO_PIN_RESET);
            break;
        default:
            break;
    }
}
