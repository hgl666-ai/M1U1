#include "bsp_uart.h"

// 实例化两个 FIFO 缓冲区
UART_FIFO_t u4_fifo = {0};
UART_FIFO_t u7_fifo = {0};

// 中断接收的单字节接水漏斗
uint8_t rx_data_u4 = 0;
uint8_t rx_data_u7 = 0;

/**
  * @brief 串口清空函数 (带原子操作保护)
  */
void UART_ClearBuffer(UART_FIFO_t *fifo) {
    __disable_irq(); // 严谨性体现 2：关总中断。防止在清空一半时，突然来数据打断
    fifo->head = 0;
    fifo->tail = 0;
    __enable_irq();  // 开总中断
}

/**
  * @brief 初始化函数：清空缓存并开启首次接收中断
  */
void BSP_UART_Init(void) {
    UART_ClearBuffer(&u4_fifo);
    UART_ClearBuffer(&u7_fifo);
    
    // 开启 IT 中断接收
    HAL_UART_Receive_IT(&huart2, &rx_data_u4, 1);
    HAL_UART_Receive_IT(&huart1, &rx_data_u7, 1);
}

/**
  * @brief 阻塞式发送单字节
  */
void UART_SendByte(UART_HandleTypeDef *huart, uint8_t data) {
    HAL_UART_Transmit(huart, &data, 1, 100);
}

/**
  * @brief 阻塞式发送数组
  */
void UART_SendArray(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t len) {
    HAL_UART_Transmit(huart, pData, len, 1000);
}

/**
  * @brief 获取当前缓冲区里还有多少个字节没读
  */
uint16_t UART_GetUnreadLen(UART_FIFO_t *fifo) {
    if (fifo->head >= fifo->tail) {
        return fifo->head - fifo->tail;
    } else {
        return UART_BUF_SIZE - fifo->tail + fifo->head;
    }
}

/**
  * @brief 从缓冲区读取一个字节
  * @retval 1:读取成功  0:缓冲区为空
  */
uint8_t UART_ReadByte(UART_FIFO_t *fifo, uint8_t *pData) {
    if (fifo->head == fifo->tail) {
        return 0; // 追尾了，说明没新数据
    }
    *pData = fifo->buffer[fifo->tail];
    fifo->tail = (fifo->tail + 1) % UART_BUF_SIZE; // 环形步进
    return 1;
}

// =========================================================================
// 以下是 HAL 库底层中断回调函数，由硬件自动调用，绝对不要在 main 里调它们！
// =========================================================================

/**
  * @brief 正常接收完成回调函数
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) { // 收到 U4 的数据
        uint16_t next_head = (u4_fifo.head + 1) % UART_BUF_SIZE;
        
        // 严谨性体现 3：防爆仓机制。如果下一步就追上 tail 了，说明缓存满了，直接丢弃新数据
        if (next_head != u4_fifo.tail) { 
            u4_fifo.buffer[u4_fifo.head] = rx_data_u4;
            u4_fifo.head = next_head;
        }
        // 必须再次开启接收！
        HAL_UART_Receive_IT(&huart2, &rx_data_u4, 1);
        
    } else if (huart->Instance == USART1) { // 收到 U7 的数据
        uint16_t next_head = (u7_fifo.head + 1) % UART_BUF_SIZE;
        
        if (next_head != u7_fifo.tail) {
            u7_fifo.buffer[u7_fifo.head] = rx_data_u7;
            u7_fifo.head = next_head;
        }
        // 必须再次开启接收！
        HAL_UART_Receive_IT(&huart1, &rx_data_u7, 1);
    }
}

/**
  * @brief 错误中断回调函数 (严谨性体现 4：满血复活机制)
  * 当发生溢出(ORE)、帧错误(FE)时，HAL 库会关闭接收并调这个函数。
  * 如果不处理，串口将永远假死！
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
    // 无论是哪种错误，直接简单粗暴地重新挂载接收中断漏斗，满血复活！
    if (huart->Instance == USART2) {
        HAL_UART_Receive_IT(&huart2, &rx_data_u4, 1);
    } else if (huart->Instance == USART1) {
        HAL_UART_Receive_IT(&huart1, &rx_data_u7, 1);
    }
}
