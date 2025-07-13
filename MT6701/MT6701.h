#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/uart.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "MT6701.pio.h"

// --- 配置 ---
// 使用 SPI0 实例
#define SPI_PORT spi0

// 定义引脚 (根据您的硬件连接修改)
#define PIN_MISO 6 // SPI0 RX
#define PIN_SCK  7 // SPI0 SCK
#define PIN_CS   8 // Chip Select, 使用普通 GPIO

// MT6701 数据位数 (根据您的传感器型号和配置)
#define MT6701_DATA_BITS 14
#define MT6701_BUFFER_SIZE 3

class MT6701
{
private:
    MT6701();
    
    //Data
    volatile uint32_t rawValue; //原始数据
    volatile uint16_t angleRawData; //原始角度数据
    volatile uint8_t magneticStatus; //磁性状态
    volatile uint8_t crcCode; //CRC校验数据
    volatile float angleValue; //角度数据
    volatile float radValue; //弧度数据

    //DMA SPI Read without PIO
    int dataLength = 3; // Length of data to read
    uint8_t readBuffer[16]; // Buffer for reading data
    const uint8_t MT6701_READ_CMD = 0x00; // Dummy读取
    //uint8_t rxBuf[MT6701_BUFFER_SIZE] __attribute__((aligned(MT6701_BUFFER_SIZE * 2)));
    uint8_t rxBuf[MT6701_BUFFER_SIZE]; // 接收数据缓冲区

    int dataChannelTX; // DMA channel for SPI data transfer
    int dataChannelRX; // DMA channel for SPI data transfer
    
    //PIO Read
    PIO pio;
    uint32_t sm; //PIO状态机
    uint32_t pioDMAChannel; //PIO DMA通道
    uint32_t offset; //PIO状态机内存位置
    

    ~MT6701();

public:
    static MT6701& getInstance() {
        static MT6701 instance; // 确保被销毁
        return instance;
    }

    // 禁止拷贝构造和赋值操作符
    MT6701(const MT6701&) = delete;
    MT6701& operator=(const MT6701&) = delete;

    // 三种初始化
    void initSSI();
    void initPIO();
    void initDMA();

    void processData(); // 处理原始数据
    uint32_t getRawValue(); //获取原始数据值 返回值范围为 0 至 2^14-1
    float getAccumulateAngle(); //获取累加的角度制角度
    float getAbsoluteAngle();  //获取绝对角度制角度
    float getAccumulateRadian(); //获取累加的弧度制角度
    float getAbsoluteRadian(); //获取绝对弧度制角度
    uint8_t getMagneticFieldStatus(); //获取当前磁场状态

    //--------------------------只使用DMA不使用PIO----------------------------------------------
    uint32_t readDMAData(); // Read data from the sensor only DMA
    void csSelect(); // Chip select methods
    void csDeselect(); // Deselect the chip
    uint8_t DMARequest(); // 获取当前磁场状态
    bool DMA_READ(); // 设置 DMA 请求
    uint32_t makeRawDMAValue(); //获取DMA读取的RawValue

    //--------------------------只使用PIO不使用DMA----------------------------------------------
    uint32_t readWithPIO(); //PIO读取
    

    //----------------------------同时使用DMA与PIO极致加速---------------------------------------
    uint32_t readPIOViaDMA(); // 使用PIO与DMA读取数据
    void transportStart(); //PIO启动
    void ssiRead(); //读取SSI

};




