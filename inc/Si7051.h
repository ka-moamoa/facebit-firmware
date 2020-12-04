/*

Arduino Library for Silicon Labs Si7051 �0.1�C (max) Digital Temperature Sensor
Written by AA for ClosedCube
---

The MIT License (MIT)

Copyright (c) 2016 ClosedCube Limited

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/


#ifndef _CLOSEDCUBE_SI7051_h
#define _CLOSEDCUBE_SI7051_h

#include "mbed.h"

#define SI7051_ADDRESS (0x40 << 1)

typedef union {
	uint8_t rawData;
	struct {
		uint8_t resolution0 : 1; // bitfields
		uint8_t reserve1 : 4;
		uint8_t vdds : 1; // vdds = 1 if and only if VDD between 1.8V and 1.9V
		uint8_t reserved2 : 1;
		uint8_t resolution7 : 1;
	};
} SI7051_Register;


class Si7051 {
public:
	Si7051(I2C *i2c, char address = SI7051_ADDRESS);

	void initialize();
	void setResolution(uint8_t resolution);

	void reset();

	uint8_t readFirmwareVersion();

	float readTemperature();
private:
	uint8_t _address;
	I2C *_i2c;

	const char MEASURE_HOLD = 0xE3;
	const char MEASURE_NOHOLD = 0xF3;
	const char RESET = 0xFE;
	const char WRITE_UR = 0xE6;
	const char READ_UR = 0xE7;
	const char READ_ID1[2] = {0xFA, 0x0F};
	const char READ_ID2[2] = {0xFC, 0xC9};
	const char READ_FW_REV[2] = {0x84, 0xB8};
	
	const char WRITE = 0x00;
	const char READ = 0x01;
	
	const uint8_t MEASUREMENT_TIMEOUT_MS = 10; 
};

#endif
