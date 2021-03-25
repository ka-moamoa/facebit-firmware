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
#include "Logger.h"
#include "Utilites.h"

Si7051::Si7051(I2C *i2c, char address)
{
	_logger = Logger::get_instance();
	_i2c = i2c;
	_address = address;
}

void Si7051::initialize() {
	setResolution(12);

	_frequency_timer.reset();
	_frequency_timer.start();

	_timer.reset();
	_timer.start();
}

void Si7051::reset()
{
	_i2c->start();
	bool ack = _i2c->write(_address | WRITE);
	if (!ack) { _logger->log(TRACE_WARNING, "nack reset address stage"); };
	
	ack = _i2c->write(RESET);
	if (!ack) { _logger->log(TRACE_WARNING, "nack reset cmd stage"); };

	_i2c->stop();

	_timer.reset();
	_timer.start();
}

uint8_t Si7051::readFirmwareVersion()
{
	_i2c->start();

	bool ack = _i2c->write(_address | WRITE); // write command
	if (!ack) { _logger->log(TRACE_WARNING, "nack fw address stage"); };
	
	ack = _i2c->write(READ_FW_REV[0]);
	if (!ack) { _logger->log(TRACE_WARNING, "nack fw cmd1 stage"); };

	ack = _i2c->write(READ_FW_REV[1]);
	if (!ack) { _logger->log(TRACE_WARNING, "nack fw cmd2 stage"); };

	_i2c->start();
	_i2c->write(_address | READ);

	uint8_t fw_rev = _i2c->read(false);

	_i2c->stop();

	return fw_rev;
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
	if (!ack) { _logger->log(TRACE_WARNING, "nack set res address stage"); };

	ack = _i2c->write(WRITE_UR);
	if (!ack) { _logger->log(TRACE_WARNING, "nack set res cmd stage"); };

	ack = _i2c->write(reg.rawData);
	_i2c->read(true); // this is to "ack" the write

	_i2c->stop();
}

float Si7051::readTemperature() 
{
	_i2c->start();

	bool ack = _i2c->write(_address | WRITE);
	if (!ack) { _logger->log(TRACE_WARNING, "nack address stage"); };

	ack = _i2c->write(MEASURE_NOHOLD);
	if (!ack) { _logger->log(TRACE_WARNING, "nack measure cmd stage"); };

	LowPowerTimer timeout;
	timeout.start();
	ack = false;

	// TODO: this isn't working right now. The device should nack until a measurement is ready, but it seems to ack before it's ready and we're getting erroneous results. 
	// the workaround is to sleep_for > 4 ms or so (probably 10 is safer), to just give it time to finish the measurement.
	while (ack == false && std::chrono::duration_cast<std::chrono::milliseconds>(timeout.elapsed_time()).count() < MEASUREMENT_TIMEOUT_MS) // the device will nack read requests until the measurement is ready
	{
		ThisThread::sleep_for(10ms);
		_i2c->start();
		ack = _i2c->write(_address | READ);
		if (!ack) { _logger->log(TRACE_DEBUG, "nack from temp sensor. waiting... %lli ms", std::chrono::duration_cast<std::chrono::milliseconds>(timeout.elapsed_time()).count()); }
	}
	timeout.stop();
	if (ack == false)
	{
		_logger->log(TRACE_WARNING, "Temp sensor timeout without response");
		return -999.999;
	}
	
	
	uint8_t msb = _i2c->read(true); // ack the byte
	uint8_t lsb = _i2c->read(false); // don't ack (don't get checksum)

	_i2c->stop();
 
	uint16_t val = msb << 8 | lsb;

	return (175.72*val) / 65536 - 46.85;
}

bool Si7051::update()
{
	float measurement_period = 1.0 / (float)_measurement_frequency_hz;

	if (_frequency_timer.read() >= measurement_period)
	{
		float tempVal = readTemperature();
		uint16_t tempValx100 = (uint16_t)Utilities::round(tempVal * 100.0);
		_tempx100_array.push_back(tempValx100);

		if (_tempx100_array.size() > MAX_BUFFER_SIZE)
		{
			_tempx100_array.erase(_tempx100_array.begin());
		}

		_relative_measurement_timestamp = _frequency_timer.read_ms();
		_last_measurement_timestamp = _timer.read_ms();
		_frequency_timer.reset();
		return true;
	}
	
	return false;
}

uint64_t Si7051::getDeltaTimestamp(bool broadcast)
{
	uint64_t delta_t = _last_measurement_timestamp - _last_broadcast_timestamp;
	if (broadcast)
	{
		float actual_frequency = 1000.0 * (float)getBufferSize() / (float) delta_t;
		_actual_frequencyx100 = Utilities::round(actual_frequency * 100.0);
		_last_broadcast_timestamp = _last_measurement_timestamp;
	}

	return delta_t;
}

uint32_t Si7051::getFrequencyx100()
{
	return _actual_frequencyx100;
}

