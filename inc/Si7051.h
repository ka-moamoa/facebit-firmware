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
#include <vector>

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
	
	void setFrequency(uint8_t frequency_Hz) { _measurement_frequency_hz = frequency_Hz; };
	uint32_t getFrequencyx100();

	void reset();

	uint8_t readFirmwareVersion();

	float readTemperature();
	void update();
	bool getBufferFull() { return _tempx100_array.size() >= MAX_BUFFER_SIZE; };
	void clearBuffer() { _tempx100_array.clear(); };
	uint8_t getBufferSize() { return _tempx100_array.size(); };
	uint16_t* getBuffer() { return _tempx100_array.data(); };
	uint64_t getDeltaTimestamp(bool broadcast);
private:
	uint8_t _address;
	I2C *_i2c;
	std::vector<uint16_t> _tempx100_array;
	uint8_t _measurement_frequency_hz = 20; // Hz
	LowPowerTimer _frequency_timer;
	LowPowerTimer _timer;
	uint64_t _relative_measurement_timestamp = 0;
	uint64_t _last_measurement_timestamp = 0;
	uint64_t _last_broadcast_timestamp = 0;

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
	
	const uint8_t MEASUREMENT_TIMEOUT_MS = 20; 

	const uint8_t MAX_BUFFER_SIZE = 100;
};

#endif
