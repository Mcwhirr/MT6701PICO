#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "MT6701/MT6701.h"

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

/**
 * @brief 从 MT6701 读取原始角度数据
 * * @return uint32_t 返回原始数据值 (0 至 2^18-1)
 */

int main() {
    // 初始化标准 IO (用于 printf)
    stdio_init_all();

    gpio_put(18, 1); // CSn高电平
    gpio_put(17, 0); // SCK低电平


    
    // 等待几秒，方便打开串口监视器
    sleep_ms(2000);
    printf("MT6701 Angle Sensor Reader\n");

    MT6701& sensor = MT6701::getInstance(); // 获取单例实例
    //sensor.initSSI(); // 初始化 SP
    //sensor.initPIO();  //PIO初始化
    sensor.initSSI();

    // 主循环
    while (1) {
        /*
        float angle = sensor.getAccumulateAngle(); // 获取累加角度
        uint8_t magnetic_status = sensor.getMagneticFieldStatus(); // 获取磁场状态
        // 打印角度和磁场状态
        uint32_t getRaw = sensor.getRawValue(); // 获取原始数据值
        printf("Raw: %-7lu | Angle: %.2f degrees | MagneticStatus: %d\n", getRaw, angle, magnetic_status);
        */
        /*
        //PIO版本
        volatile int32_t raw_value0 = sensor.readWithPIO(); // 获取原始数据值
        int32_t getRaw = raw_value0 >> (24 - MT6701_DATA_BITS);
        printf("Raw: %-7lu\n", getRaw);
        */
        
        /*
        //DMA版本
        sensor.DMA_READ(); // 触发 DMA 读取
        int32_t raw_value = sensor.DMAProcessData(); // 处理 DMA 数据
        int32_t getRaw = raw_value >> (24 - MT6701_DATA_BITS);
        // 计算最大原始值 (2^14)
        const uint32_t max_raw_value = (1 << MT6701_DATA_BITS);
        // 将原始值映射到 0-360 度
        float angle = (float)getRaw / (float)max_raw_value * 360.0f;
        // 假设状态位在最低两位
        uint8_t status = (getRaw << 14) >> 6 ; 
        printf("Raw: %-5lu | Angle: %.2f degrees | MagneticStatus: %d\n", getRaw, angle, status);
        */
        
        //DMA&PIO超强版本
        sensor.ssiRead();
        sensor.processData();
        volatile float raw_value1 = sensor.getAccumulateAngle();
        printf("Raw: %.2f\n", raw_value1);

    }
    
    return 0;
}