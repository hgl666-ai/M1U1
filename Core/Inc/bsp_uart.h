#ifndef __BSP_UART_H
#define __BSP_UART_H

#include "main.h"

// 引入 CubeMX 生成的串口句柄
extern UART_HandleTypeDef huart1; // U7 电机板 / 电脑调试
extern UART_HandleTypeDef huart2; // U4 待测板烧录 (9E1)

// 定义环形缓冲区大小 (必须是 2 的幂次方最安全，这里设为 256)
#define UART_BUF_SIZE 256

// 严谨性体现 1：FIFO 结构体。指针必须加 volatile，防止编译器过度优化
typedef struct {
    uint8_t buffer[UART_BUF_SIZE];
    volatile uint16_t head; // 中断写指针
    volatile uint16_t tail; // 主程序读指针
} UART_FIFO_t;

// 暴露出两个串口的 FIFO 结构体
extern UART_FIFO_t u4_fifo;
extern UART_FIFO_t u7_fifo;

// 核心函数声明
void BSP_UART_Init(void);
void UART_SendByte(UART_HandleTypeDef *huart, uint8_t data);
void UART_SendArray(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t len);
uint8_t UART_ReadByte(UART_FIFO_t *fifo, uint8_t *pData);
void UART_ClearBuffer(UART_FIFO_t *fifo);
uint16_t UART_GetUnreadLen(UART_FIFO_t *fifo);

#endif