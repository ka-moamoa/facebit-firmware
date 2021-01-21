#include "Barometer.hpp"

Barometer::Barometer(SPI *spi, PinName cs_pin, PinName int_pin) :
_barometer(spi, cs_pin),
_int_pin(int_pin)
{
}

Barometer::~Barometer()
{
}

bool Barometer::initialize()
{
    if (_initialized)
    {
        LOG_INFO("%s", "Barometer has already been initialized");
        return false;
    }

    _bus_control = BusControl::get_instance();

    if (_bus_control->get_spi_power() == false)
    {
        LOG_WARNING("%s", "SPI bus not powered, cannot initialize");
        return false;
    }

    if (_barometer.init(NULL) == LPS22HB_ERROR)
    {
        return false;
    }

    if (_barometer.set_odr(25.0) == LPS22HB_ERROR)
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

    LOG_DEBUG("%s", "Barometer initialized successfully");
    _initialized = true;
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

bool Barometer::update()
{
    // if the interrupt has been triggered, read the data
    if (_bar_data_ready)
    {
        LPS22HB_FifoStatus_st fifo_status;
        _barometer.get_fifo_status(&fifo_status);
        if (!fifo_status.FIFO_FULL)
        {
            LOG_DEBUG("%s", "FIFO not full, but interrupt triggered");
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
    _bar_data_ready = true;
}

uint32_t Barometer::get_pressure_buffer_element()
{
    uint32_t val = _pressure_buffer.front();
    _pressure_buffer.pop();
    return val;
}

uint32_t Barometer::get_temp_buffer_element()
{
    uint32_t val = _temperature_buffer.front();
    _temperature_buffer.pop();
    return val;
}

bool Barometer::read_buffered_data()
{
    if (_barometer.get_fifo(_pressure_buffer, _temperature_buffer) == LPS22HB_ERROR)
    {
        LOG_WARNING("%s", "Unable to read barometer data");
        return false;
    }

    _bar_data_ready = false;
    return true;
}

// Purgatory


    // if (status.FIFO_EMPTY != _last_fifo_empty && status.FIFO_EMPTY)
    // {
    //     LOG_DEBUG("Fifo Empty! = %u", status.FIFO_LEVEL);
    //     _last_fifo_empty = status.FIFO_EMPTY;
    // }
    // if (status.FIFO_FULL != _last_fifo_full && status.FIFO_FULL)
    // {
    //     LOG_DEBUG("FIFO full! = %u", status.FIFO_LEVEL);
    //     _last_fifo_full = status.FIFO_FULL;
    // }
    // if (status.FIFO_OVR != _last_fifo_ovr && status.FIFO_OVR)
    // {
    //     LOG_DEBUG("FIFO overfull! = %u", status.FIFO_LEVEL);
    //     _last_fifo_ovr = status.FIFO_OVR;
    // }
    // if (status.FIFO_FTH != _last_fifo_fth && status.FIFO_FTH)
    // {
    //     LOG_DEBUG("FIFO FTH! = %u", status.FIFO_LEVEL)
    //     _last_fifo_fth = status.FIFO_FTH;
    // }
    // if (status.FIFO_LEVEL != _last_fifo_level)
    // {
    //     LOG_DEBUG("FIFO level = %u", status.FIFO_LEVEL);
    //     _last_fifo_level = status.FIFO_LEVEL;
    // }

            // uint8_t mode = 255;
        // _barometer.get_fifo_mode(&mode);
        // LOG_INFO("fifo mode = %u", mode);

        // uint8_t enabled = 255;
        // _barometer.get_fifo_enabled(&enabled);
        // LOG_INFO("fifo enabled = %u", enabled);