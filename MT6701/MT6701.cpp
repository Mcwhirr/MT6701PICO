#include "MT6701.h"
#include "MT6701.pio.h"

MT6701::MT6701(/* args */)
{
    // SPI initialisation. This example will use SPI at 1MHz.

}

void MT6701::initSSI()
{
    spi_init(SPI_PORT, 4*1000*1000);
    spi_set_format(SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    
    gpio_init(PIN_CS); // 初始化手动控制的 CS 引脚
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1); // 默认设为高电平 (不选中)


}

void MT6701::PIOinit()
{
    pio = pio0;
    //sm = 0;
    dmaChannel = 0;
    // &PIOTest_program 是 PIO 程序的汇编代码地址。
    // 使用 pio_add_program 函数将程序（汇编器转换后的二进制代码）加载到 PIO。
    // 该函数会返回程序的起始位置，我们将其保存在 offset 变量中。
    offset = pio_add_program(pio, &ssi_CPOL0_CPHA1_WITH_CS_program);
    sm = pio_claim_unused_sm(pio,true);

    //pio选择，状态机，数据引脚，时钟引脚，时钟分频
    ssi_CPOL0_CPHA1_WITH_CS_Init(pio,sm,offset, PIN_MISO, PIN_SCK, 20.f);


}

uint32_t MT6701::readData()
{
    uint8_t rx_data[3]; // 读取3个字节 (24位) 来容纳24位数据
    // 1. 将 CS 拉低，开始通信
    csSelect();
    // 2. 使用 SPI 读取数据
    // 我们发送0x00作为虚拟数据来产生时钟，同时接收传感器数据
    spi_read_blocking(SPI_PORT, 0x00, rx_data, 3);
    // 3. 将 CS 拉高，结束通信
    csDeselect();
    // 4. 将接收到的3个字节合并成一个32位整数
    // MT6701 的数据是 MSB first (高位在前)
    raw_value = (rx_data[0] << 16) | (rx_data[1] << 8) | rx_data[2];
    
    return raw_value;
}

void MT6701::csSelect()
{
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 0);  // Active low
    asm volatile("nop \n nop \n nop");
}

void MT6701::csDeselect()
{
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 1);
    asm volatile("nop \n nop \n nop");
}

uint32_t MT6701::getRawValue()
{
    raw_value >>= (24 - MT6701_DATA_BITS);
    return raw_value;
}

float MT6701::getAccumulateAngle()
{
    uint32_t raw_data = readData();
    raw_data >>= (24 - MT6701_DATA_BITS);

    // 计算最大原始值 (2^14)
    const uint32_t max_raw_value = (1 << MT6701_DATA_BITS);
    // 将原始值映射到 0-360 度
    float angle = (float)raw_data / (float)max_raw_value * 360.0f;

    return angle;
}

float MT6701::getAbsoluteAngle()
{
    return 0.0f;
}

float MT6701::getAccumulateRadian()
{
    return 0.0f;
}

float MT6701::getAbsoluteRadian()
{
    return 0.0f;
}

uint8_t MT6701::getMagneticFieldStatus()
{
    uint8_t status = (raw_value << 14) >> 6 ; // 假设状态位在最低两位

    return status;
}

uint8_t MT6701::DMARequest()
{
    // 触发 DMA 请求以读取数据
    // 这里可以根据具体的 DMA 配置来实现
    dma_hw->ints0 = 1u << dataChannelRX; // 清除 DMA 中断标志
    dma_channel_start(dataChannelRX); // 启动 DMA 通道
    return 0;
}

bool MT6701::DMA_READ()
{
    dataChannelRX = dma_claim_unused_channel(true); // 初始化 DMA 通道 (如果需要)
    dataChannelTX = dma_claim_unused_channel(true); 

    dma_channel_config SPI_DMA_RX_CONFIG = dma_channel_get_default_config(dataChannelRX); // 配置 DMA 通道 ，后面那个大的结构体就是具体配置详情
    dma_channel_config SPI_DMA_TX_CONFIG = dma_channel_get_default_config(dataChannelTX);

    //接受数据通道
    channel_config_set_transfer_data_size(&SPI_DMA_RX_CONFIG, DMA_SIZE_8); // 设置数据大小为 32 位
    channel_config_set_dreq(&SPI_DMA_RX_CONFIG, spi_get_dreq(SPI_PORT, false)); // DREQ 来自 SPI RX  true: 表示请求的是 发送 (TX) 方向的 DREQ 信号。若为 false 则表示接收 (RX) 方向
    channel_config_set_read_increment(&SPI_DMA_RX_CONFIG, false);   // 设置读取地址递增
    channel_config_set_write_increment(&SPI_DMA_RX_CONFIG, true); // 向缓冲区的写入地址递增

    // 设置 DMA 通道的配置
    dma_channel_configure(
        dataChannelRX, 
        &SPI_DMA_RX_CONFIG,
        rxBuf,                       // 目标地址: 接收数据的缓冲区
        &spi_get_hw(SPI_PORT)->dr,   // 源地址: SPI 数据寄存器
        3,                           // 传输数据量 (3 个字节)
        false                        // 不立即启动
    );

    channel_config_set_transfer_data_size(&SPI_DMA_TX_CONFIG, DMA_SIZE_8); // 设置数据大小为 8 位
    channel_config_set_read_increment(&SPI_DMA_TX_CONFIG, false);  // 设置读取地址不递增
    channel_config_set_write_increment(&SPI_DMA_TX_CONFIG, true); // 设置写入地址递增
    //channel_config_set_chain_to(&SPI_DMA_TX_CONFIG, dataChannelRX); // 设置链式传输到接收通道，单工才会需要这种东西

    dma_channel_configure(
        dataChannelTX, 
        &SPI_DMA_TX_CONFIG,
        &spi_get_hw(SPI_PORT)->dr,   // 目标地址: SPI 数据寄存器
        &MT6701_READ_CMD,            // 源地址: 发送数据的缓冲区
        3,                           // 传输数据量 (3 个字节)
        false                        // 不立即启动
    );

    csSelect(); // 选择芯片，开始通信
    // 启动DMA传输 (同时启动TX和RX以实现全双工)
    dma_start_channel_mask((1u << dataChannelTX) | (1u << dataChannelRX));

    // 等待DMA传输完成
    dma_channel_wait_for_finish_blocking(dataChannelTX); // 等待发送通道完成
    dma_channel_wait_for_finish_blocking(dataChannelRX); // 等待接收通道完成

    csDeselect(); // 取消选择芯片，结束通信
    /*
    dma_channel_set_irq0_enabled(dataChannelRX, true); // 告诉DMA在通道完成一个块时触发IRQ线0
    irq_set_exclusive_handler(DMA_IRQ_0, [this]() {
        // 处理 SPI 中断
        // 这里可以添加处理逻辑，例如读取数据或处理状态
        DMARequest(); // 调用 DMARequest 来处理数据
    });
    irq_set_enabled(DMA_IRQ_0, true); // 启用 DMA 中断
    */

    // 手动拉高DMA通道
    dma_channel_unclaim(dataChannelTX);
    dma_channel_unclaim(dataChannelRX);

    return true; // 返回 true 表示 DMA 请求已设置
}

uint32_t MT6701::DMAProcessData()
{
    raw_value = (rxBuf[0] << 16) | (rxBuf[1] << 8) | rxBuf[2];
    
    return raw_value;
}

uint32_t MT6701::readWithPIO()
{
    raw_value = ssi_CPOL0_CPHA1_WITH_CS_Read(pio,sm);
    return raw_value;
}

MT6701::~MT6701()
{

}
