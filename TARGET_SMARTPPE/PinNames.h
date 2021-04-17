/* 
 * Copyright (c) 2016 Nordic Semiconductor ASA
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 *   1. Redistributions of source code must retain the above copyright notice, this list 
 *      of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form, except as embedded into a Nordic Semiconductor ASA 
 *      integrated circuit in a product or a software update for such product, must reproduce 
 *      the above copyright notice, this list of conditions and the following disclaimer in 
 *      the documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of Nordic Semiconductor ASA nor the names of its contributors may be 
 *      used to endorse or promote products derived from this software without specific prior 
 *      written permission.
 *
 *   4. This software, with or without modification, must only be used with a 
 *      Nordic Semiconductor ASA integrated circuit.
 *
 *   5. Any software provided in binary or object form under this license must not be reverse 
 *      engineered, decompiled, modified and/or disassembled. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#ifndef PPE_PINNAMES_H
#define PPE_PINNAMES_H

#include "cmsis.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PIN_INPUT,
    PIN_OUTPUT
} PinDirection;

typedef enum {
    // Not connected
    NC = (int)0xFFFFFFFF,

    p0  = 0,
    p1  = 1,
    p2  = 2,
    p3  = 3,
    p4  = 4,
    p5  = 5,
    p6  = 6,
    p7  = 7,
    p8  = 8,
    p9  = 9,
    p10 = 10,
    p11 = 11,
    p12 = 12,
    p13 = 13,
    p14 = 14,
    p15 = 15,
    p16 = 16,
    p17 = 17,
    p18 = 18,
    p19 = 19,
    p20 = 20,
    p21 = 21,
    p22 = 22,
    p23 = 23,
    p24 = 24,
    p25 = 25,
    p26 = 26,
    p27 = 27,
    p28 = 28,
    p29 = 29,
    p30 = 30,
    p31 = 31,

    P0_0  = p0,
    P0_1  = p1,
    P0_2  = p2,
    P0_3  = p3,
    P0_4  = p4,
    P0_5  = p5,
    P0_6  = p6,
    P0_7  = p7,

    P0_8  = p8,
    P0_9  = p9,
    P0_10 = p10,
    P0_11 = p11,
    P0_12 = p12,
    P0_13 = p13,
    P0_14 = p14,
    P0_15 = p15,

    P0_16 = p16,
    P0_17 = p17,
    P0_18 = p18,
    P0_19 = p19,
    P0_20 = p20,
    P0_21 = p21,
    P0_22 = p22,
    P0_23 = p23,

    P0_24 = p24,
    P0_25 = p25,
    P0_26 = p26,
    P0_27 = p27,
    P0_28 = p28,
    P0_29 = p29,
    P0_30 = p30,
    P0_31 = p31,

    // mBed interface Pins
    USBTX = NC,
    USBRX = NC,
    CONSOLE_TX = NC,
    CONSOLE_RX = NC,
    STDIO_UART_TX = p18,
    STDIO_UART_RX = NC,
    STDIO_UART_CTS = NC,
    STDIO_UART_RTS = NC,
    LED1 = p24,

    // SPI
    SPI_MOSI = p16,
    SPI_MISO = p12,
    SPI_SCK  = p15,

    // I2C
    I2C_PULLUP = p23,
    I2C_SDA0 = p27,
    I2C_SCL0 = p25,

    // Magnetometer
    MAG_CS = p2,
    MAG_DRDY = p4,
    MAG_VCC = p7,
    
    // FRAM
    FRAM_CS = p13,
    FRAM_VCC = p14,
    
    // Barometer
    BAR_VCC = p9,
    BAR_CS = p17,
    BAR_DRDY = p19,
    
    // Microphone
    MIC_VCC = p20,
    SD = p6,
    WS = p8,
    I2S_SCK = p11,

    // Temperature
    TEMP_VCC = p26,

    // Accessory Header
    ACC_VCC_EN = p28,
    
    // Air Quality
    VOC_VCC = p29,
    
    // Capacitor Monitoring
    VCAP = p3,
    VCAP_ENABLE = p5,
    N_BACKUP = p30,
    
    // IMU
    IMU_VCC = p10,
    IMU_INT1 = p22,
    IMU_CS = p31,

    D0 = p11,
    D1 = p12,
    D2 = p13,
    D3 = p14,
    D4 = p15,
    D5 = p16,
    D6 = p17,
    D7 = p18,

    D8 = p19,
    D9 = p20,
    D10 = p22,
    D11 = p23,
    D12 = p24,
    D13 = p25,

    D14 = p26,
    D15 = p27,

    A0 = p3,
    A1 = p4,
    A2 = p28,
    A3 = p29,
    A4 = p30,
    A5 = p31
} PinName;

typedef enum {
    PullNone = 0,
    PullDown = 1,
    PullUp = 3,
    PullDefault = PullUp
} PinMode;

#ifdef __cplusplus
}
#endif

#endif
