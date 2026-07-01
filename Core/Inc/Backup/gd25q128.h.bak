#ifndef __GD25Q128_H
#define __GD25Q128_H

#include "main.h"

// 声明外部由 CubeMX 生成的 SPI1 句柄
extern SPI_HandleTypeDef hspi1; 

// GD25Q128 常用指令集 (与 W25Q 系列完全兼容)
#define GD25Q_WriteEnable_Cmd      0x06  // 写使能
#define GD25Q_ReadStatusReg1_Cmd   0x05  // 读状态寄存器1
#define GD25Q_ReadData_Cmd         0x03  // 读数据
#define GD25Q_PageProgram_Cmd      0x02  // 页编程 (写入)
#define GD25Q_SectorErase_Cmd      0x20  // 扇区擦除 (4KB)
#define GD25Q_DeviceID_Cmd         0x90  // 读制造商/设备ID

// 基础 SPI 接口
uint8_t SPI1_SwapByte(uint8_t txData);
uint16_t GD25Q_ReadID(void);

// 核心读写擦除接口
void GD25Q_WriteEnable(void);
void GD25Q_WaitForWriteEnd(void);
void GD25Q_EraseSector(uint32_t SectorAddr);
void GD25Q_WritePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void GD25Q_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);

#endif