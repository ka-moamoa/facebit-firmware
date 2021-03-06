#include "Barometer.hpp"
#include "Utilites.h"

using namespace std::chrono;

Barometer::Barometer(SPI *spi, PinName cs_pin, PinName int_pin) :
_barometer(spi, cs_pin),
_int_pin(int_pin, PullNone)
{
    _logger = Logger::get_instance();
}

Barometer::~Barometer()
{
}

bool Barometer::initialize()
{
    if (_initialized)
    {
        _logger->log(TRACE_WARNING, "%s", "Barometer has already been initialized");
        return false;
    }

    _bus_control = BusControl::get_instance();

    if (_bus_control->get_spi_power() == false)
    {
        _logger->log(TRACE_WARNING, "%s", "SPI bus not powered, cannot initialize");
        return false;
    }

    if (_barometer.init(NULL) == LPS22HB_ERROR)
    {
        return false;
    }

    if (_barometer.set_odr(_frequency) == LPS22HB_ERROR)
    {
        return false;
    }

    if (_barometer.enable_fifo() == LPS22HB_ERROR)
    {
        return false;
    }

    if (_barometer.set_fifo_mode(3) == LPS22HB_ERROR) // Stream-to-fifo mode
    {
        return false;
    }

    if (_barometer.enable() == LPS22HB_ERROR)
    {
        return false;
    }

    _int_pin.rise(callback(this, &Barometer::bar_data_ready));

    _logger->log(TRACE_TRACE, "%s", "Barometer initialized successfully");
    _t_barometer.start();
    _initialized = true;
    return true;
}

bool Barometer::set_frequency(uint8_t frequency)
{
    if (_barometer.set_odr((float)frequency - 0.1) == LPS22HB_ERROR) // - 0.1 because they try to compare floats in the driver 
    {
        return false;
    }

    _frequency = frequency;
    return true;
}

bool Barometer::set_fifo_full_interrupt(bool enable)
{
    if (_barometer.fifo_full_interrupt(enable) == LPS22HB_ERROR)
    {
        return false;
    }
    
    return true;
}

bool Barometer::update(bool force)
{
    // if the interrupt has been triggered, read the data
    if (_bar_data_ready || force)
    {
        LPS22HB_FifoStatus_st fifo_status;
        _barometer.get_fifo_status(&fifo_status);
        if (!fifo_status.FIFO_FULL && !force)
        {
            _logger->log(TRACE_DEBUG, "%s", "FIFO not full, but interrupt triggered");
            return false;
        }

        LPS22HB_InterruptDiffStatus_st interrupt_source;
        _barometer.get_interrupt_status(&interrupt_source);
        if (interrupt_source.PH)
        {
            _high_pressure_event_flag = true;
        }

        if (!read_buffered_data())
        {
            return false;
        }
        
        return true;
    }

    return false;
}

bool Barometer::enable_pressure_threshold(bool enable, bool high_pressure, bool low_pressure)
{
    if (_barometer.differential_interrupt(enable, high_pressure, low_pressure) == LPS22HB_ERROR)
    {
        return false;
    }

    return true;
}

bool Barometer::set_pressure_threshold(int16_t hPa)
{
    if (_barometer.set_interrupt_pressure(hPa) == LPS22HB_ERROR)
    {
        return false;
    }

    return true;
}

void Barometer::bar_data_ready()
{
    _drdy_timestamp = duration_cast<milliseconds>(_t_barometer.elapsed_time()).count();
    uint64_t delta_timestamp = _drdy_timestamp - _last_timestamp;
    _last_timestamp = _drdy_timestamp;
    
    float measurement_frequency = 1000 * (float)BAROMETER_FIFO_SIZE / ((float)delta_timestamp);
    _measurement_frequencyx100 = Utilities::round(measurement_frequency * 100);

    _bar_data_ready = true;
}

uint64_t Barometer::get_delta_timestamp(bool broadcast)
{
    uint64_t delta_t = _drdy_timestamp - _last_broadcast_timestamp;
    if (broadcast)
    {
        _last_broadcast_timestamp = _drdy_timestamp;
    }
    return delta_t;
}

float Barometer::convert_to_hpa(uint16_t raw_data)
{
  return ((float)raw_data + 80000.0) / 100.0;
}

bool Barometer::read_buffered_data()
{
    if (_barometer.get_fifo(_pressure_buffer, _temperature_buffer) == LPS22HB_ERROR)
    {
        _logger->log(TRACE_WARNING, "%s", "Unable to read barometer data");
        return false;
    }

    if (_pressure_buffer.size() > _max_buffer_size)
    {
        _pressure_buffer.resize(_max_buffer_size);
    }

    if (_temperature_buffer.size() > _max_buffer_size)
    {
        _temperature_buffer.resize(_max_buffer_size);
    }

    _bar_data_ready = false;
    return true;
}

void Barometer::set_max_buffer_size(uint16_t size)
{
    if (size > MAX_ALLOWABLE_SIZE) size = MAX_ALLOWABLE_SIZE;

    _max_buffer_size = size;
}

/*DINOSAURS
   *                               *     _
        /\     *            ___.       /  `)
    *  //\\    /\          ///\\      / /
      ///\\\  //\\/\      ////\\\    / /     /\
     ////\\\\///\\/\\.-~~-.///\\\\  / /     //\\
    /////\\\\///\\/         `\\\\\\/ /     ///\\
   //////\\\\// /            `\\\\/ /     ////\\
  ///////\\\\\//               `~` /\    /////\\
 ////////\\\\\/      ,_____,   ,-~ \\\__//////\\\
 ////////\\\\/  /~|  |/////|  |\\\\\\\\@//jro/\\
 //<           / /|__|/////|__|///////~|~/////\\
 ~~~     ~~   ` ~   ..   ~  ~    .     ~` `   '.
 ~ _  -  -~.    .'   .`  ~ .,    '.    ~~ .  '*/

    // if (status.FIFO_EMPTY != _last_fifo_empty && status.FIFO_EMPTY)
    // {
    //     _logger->log(TRACE_DEBUG, "Fifo Empty! = %u", status.FIFO_LEVEL);
    //     _last_fifo_empty = status.FIFO_EMPTY;
    // }
    // if (status.FIFO_FULL != _last_fifo_full && status.FIFO_FULL)
    // {
    //     _logger->log(TRACE_DEBUG, "FIFO full! = %u", status.FIFO_LEVEL);
    //     _last_fifo_full = status.FIFO_FULL;
    // }
    // if (status.FIFO_OVR != _last_fifo_ovr && status.FIFO_OVR)
    // {
    //     _logger->log(TRACE_DEBUG, "FIFO overfull! = %u", status.FIFO_LEVEL);
    //     _last_fifo_ovr = status.FIFO_OVR;
    // }
    // if (status.FIFO_FTH != _last_fifo_fth && status.FIFO_FTH)
    // {
    //     _logger->log(TRACE_DEBUG, "FIFO FTH! = %u", status.FIFO_LEVEL)
    //     _last_fifo_fth = status.FIFO_FTH;
    // }
    // if (status.FIFO_LEVEL != _last_fifo_level)
    // {
    //     _logger->log(TRACE_DEBUG, "FIFO level = %u", status.FIFO_LEVEL);
    //     _last_fifo_level = status.FIFO_LEVEL;
    // }

            // uint8_t mode = 255;
        // _barometer.get_fifo_mode(&mode);
        // _logger->log(TRACE_INFO, "fifo mode = %u", mode);

        // uint8_t enabled = 255;
        // _barometer.get_fifo_enabled(&enabled);
        // _logger->log(TRACE_INFO, "fifo enabled = %u", enabled);