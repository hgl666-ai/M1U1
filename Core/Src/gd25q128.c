#include "gd25q128.h"

// 宏定义片选操作，使代码更易读
#define CS_ENABLE()  HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET)
#define CS_DISABLE() HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET)

/**
  * @brief SPI底层收发一个字节
  */
uint8_t SPI1_SwapByte(uint8_t txData) {
    uint8_t rxData = 0;
    HAL_SPI_TransmitReceive(&hspi1, &txData, &rxData, 1, 1000);
    return rxData;
}

/**
  * @brief 读取芯片ID (正常返回 0xC817)
  */
uint16_t GD25Q_ReadID(void) {
    uint16_t temp = 0;
    CS_ENABLE();
    SPI1_SwapByte(GD25Q_DeviceID_Cmd);
    SPI1_SwapByte(0x00);
    SPI1_SwapByte(0x00);
    SPI1_SwapByte(0x00);
    temp = SPI1_SwapByte(0xFF) << 8;
    temp |= SPI1_SwapByte(0xFF);
    CS_DISABLE();
    return temp;
}

/**
  * @brief 写使能 (每次擦除和写入前必须调用)
  */
void GD25Q_WriteEnable(void) {
    CS_ENABLE();
    SPI1_SwapByte(GD25Q_WriteEnable_Cmd);
    CS_DISABLE();
}

/**
  * @brief 等待空闲 (查询状态寄存器的 WIP 位)
  */
void GD25Q_WaitForWriteEnd(void) {
    uint8_t status = 0;
    CS_ENABLE();
    SPI1_SwapByte(GD25Q_ReadStatusReg1_Cmd);
    do {
        status = SPI1_SwapByte(0xFF);
    } while ((status & 0x01) == 0x01); // 等待 WIP(Write In Progress) 标志位清零
    CS_DISABLE();
}

/**
  * @brief 擦除指定扇区 (4KB 为一个扇区)
  * @param SectorAddr: 扇区起始地址
  */
void GD25Q_EraseSector(uint32_t SectorAddr) {
    GD25Q_WriteEnable();
    GD25Q_WaitForWriteEnd();

    CS_ENABLE();
    SPI1_SwapByte(GD25Q_SectorErase_Cmd);
    SPI1_SwapByte((uint8_t)((SectorAddr) >> 16));
    SPI1_SwapByte((uint8_t)((SectorAddr) >> 8));
    SPI1_SwapByte((uint8_t)SectorAddr);
    CS_DISABLE();

    GD25Q_WaitForWriteEnd(); // 等待物理擦除动作完成
}

/**
  * @brief 按页写入数据 (最大256字节，写入前须确保已擦除)
  * @param pBuffer: 数据源指针
  * @param WriteAddr: 写入起始地址
  * @param NumByteToWrite: 写入字节数 (不能超过一页即256，且不能跨页)
  */
void GD25Q_WritePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite) {
    GD25Q_WriteEnable();
    GD25Q_WaitForWriteEnd();

    CS_ENABLE();
    SPI1_SwapByte(GD25Q_PageProgram_Cmd);
    SPI1_SwapByte((uint8_t)((WriteAddr) >> 16));
    SPI1_SwapByte((uint8_t)((WriteAddr) >> 8));
    SPI1_SwapByte((uint8_t)WriteAddr);

    for (uint16_t i = 0; i < NumByteToWrite; i++) {
        SPI1_SwapByte(pBuffer[i]);
    }
    CS_DISABLE();

    GD25Q_WaitForWriteEnd(); 
}

/**
  * @brief 连续读取任意长度数据
  * @param pBuffer: 存放读取数据的指针
  * @param ReadAddr: 读取起始地址
  * @param NumByteToRead: 读取字节数
  */
void GD25Q_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead) {
    CS_ENABLE();
    SPI1_SwapByte(GD25Q_ReadData_Cmd);
    SPI1_SwapByte((uint8_t)((ReadAddr) >> 16));
    SPI1_SwapByte((uint8_t)((ReadAddr) >> 8));
    SPI1_SwapByte((uint8_t)ReadAddr);

    for (uint16_t i = 0; i < NumByteToRead; i++) {
        pBuffer[i] = SPI1_SwapByte(0xFF);
    }
    CS_DISABLE();
}
