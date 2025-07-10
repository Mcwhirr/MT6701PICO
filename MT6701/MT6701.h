#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/uart.h"
#include "hardware/dma.h"

// --- 配置 ---
// 使用 SPI0 实例
#define SPI_PORT spi0

// 定义引脚 (根据您的硬件连接修改)
#define PIN_MISO 16 // SPI0 RX
#define PIN_SCK  18 // SPI0 SCK
#define PIN_CS   17 // Chip Select, 使用普通 GPIO

// MT6701 数据位数 (根据您的传感器型号和配置)
#define MT6701_DATA_BITS 14
#define MT6701_BUFFER_SIZE 3

class MT6701
{
private:
    /* data */
    MT6701(/* args */);
    int dataLength = 3; // Length of data to read
    volatile uint32_t raw_value;
    // SPI configuration
    uint8_t readBuffer[16]; // Buffer for reading data
    ~MT6701();
    
    const uint8_t MT6701_READ_CMD = 0x00; // 读取命令
    int dataChannelTX; // DMA channel for SPI data transfer
    int dataChannelRX; // DMA channel for SPI data transfer
    
public:
    //uint8_t rxBuf[MT6701_BUFFER_SIZE] __attribute__((aligned(MT6701_BUFFER_SIZE * 2)));
    uint8_t rxBuf[MT6701_BUFFER_SIZE]; // 接收数据缓冲区

    static MT6701& getInstance() {
        static MT6701 instance; // 确保被销毁
        return instance;
    }

    // 禁止拷贝构造和赋值操作符
    MT6701(const MT6701&) = delete;
    MT6701& operator=(const MT6701&) = delete;

    // SPI initialisation. This example will use SPI at 1MHz.
    void initSSI();
    // Read data from the sensor
    uint32_t readData();
    // Chip select methods
    void csSelect();
    // Deselect the chip
    void csDeselect();

    //获取原始数据值
    // 返回值范围为 0 至 2^14-1
    uint32_t getRawValue();
    //获取累加的角度制角度
    float getAccumulateAngle();
    //获取绝对角度制角度
    float getAbsoluteAngle();
    //获取累加的弧度制角度
    float getAccumulateRadian();
    //获取绝对弧度制角度
    float getAbsoluteRadian();
    //获取当前磁场状态
    uint8_t getMagneticFieldStatus();

    // 获取当前磁场状态
    uint8_t DMARequest();

    // 设置 DMA 请求
    bool DMA_READ();

    // 处理原始数据
    uint32_t DMAProcessData();
};




