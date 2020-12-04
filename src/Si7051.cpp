/*

Arduino Library for Silicon Labs Si7051 ±0.1°C (max) Digital Temperature Sensor
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

#include "Si7051.h"
#include "LowPowerTimer.h"
#include "I2C.h"
#include "logger.h"

Si7051::Si7051(I2C *i2c, char address)
{
	_i2c = i2c;
	_address = address;
}

void Si7051::initialize() {
	setResolution(14);
}

void Si7051::reset()
{
	_i2c->start();
	bool ack = _i2c->write(_address | WRITE);
	if (!ack) { LOG_WARNING("%s", "nack reset address stage") };
	
	ack = _i2c->write(RESET);
	if (!ack) { LOG_WARNING("%s", "nack reset cmd stage") };

	_i2c->stop();
}

uint8_t Si7051::readFirmwareVersion()
{
	// bool ack = _i2c->beginTransmission(_address | WRITE); // write command
	
	// _i2c->write(0x84);
	// _i2c->write(0xB8);
	// _i2c->endTransmission();

	// _i2c->requestFrom(_address, (uint8_t)1);

	// return _i2c->read();
}

void Si7051::setResolution(uint8_t resolution)
{
	SI7051_Register reg;

	switch (resolution)
	{
		case 11:
			reg.resolution7 = 1;
			reg.resolution0 = 1;
			break;
		case 12: 
			reg.resolution7 = 0;
			reg.resolution0 = 1;
			break;	
		case 13:
			reg.resolution7 = 1;
			reg.resolution0 = 0;
			break;
	}
	
	_i2c->start();
	bool ack = _i2c->write(_address | WRITE);
	if (!ack) { LOG_WARNING("%s", "nack set res address stage") };

	ack = _i2c->write(WRITE_UR);
	if (!ack) { LOG_WARNING("%s", "nack set res cmd stage") };

	ack = _i2c->write(reg.rawData);
	if (!ack) { LOG_WARNING("%s", "nack set res write stage") };

	_i2c->stop();
}

float Si7051::readTemperature() 
{
	_i2c->start();
	bool ack = _i2c->write(_address | WRITE);
	if (!ack) { LOG_WARNING("%s", "nack address stage") };

	ack = _i2c->write(MEASURE_NOHOLD);
	if (!ack) { LOG_WARNING("%s", "nack measure cmd stage") };

	LowPowerTimer timeout;
	timeout.start();
	ack = false;
	while (ack == false && timeout.read_ms() < MEASUREMENT_TIMEOUT_MS) // the device will nack read requests until the measurement is ready
	{
		ThisThread::sleep_for(1ms);
		_i2c->start();
		ack = _i2c->write(_address | READ);
	}
	timeout.stop();
	
	uint8_t msb = _i2c->read(true); // ack the byte
	uint8_t lsb = _i2c->read(false); // don't ack (don't get checksum)

	_i2c->stop();
 
	uint16_t val = msb << 8 | lsb;

	return (175.72*val) / 65536 - 46.85;
}

