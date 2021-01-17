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

    if (_barometer.set_odr(10.0) == LPS22HB_ERROR)
    {
        return false;
    }

    if (_barometer.enable_fifo() == LPS22HB_ERROR)
    {
        return false;
    }

    if (_barometer.set_fifo_mode(1) == LPS22HB_ERROR) // Stream mode
    {
        return false;
    }

    if (_barometer.enable_fifo_full_interrupt() == LPS22HB_ERROR)
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

bool Barometer::update()
{
    LPS22HB_FifoStatus_st status;
    _barometer.get_fifo_status(&status);

    // if the interrupt has been triggered, read the data
    if (_bar_data_ready)
    {
        read_buffered_data();
        return true;
    }

    return false;
}

void Barometer::get_pressure_buffer(float *pressure_data, uint8_t num_elements)
{
    if (num_elements > FIFO_LENGTH)
    {
        LOG_WARNING("Can only return up to %u elements", FIFO_LENGTH)
        return;
    }

    for (int i = 0; i < num_elements; i++)
    {
        pressure_data[i] = _lps22hbData[i].pressure;
    }

    _unread_pressure_data = false;
}

void Barometer::get_temperature_buffer(float *temp_data, uint8_t num_elements)
{
    if (num_elements > FIFO_LENGTH)
    {
        LOG_WARNING("Can only return up to %u elements", FIFO_LENGTH);
        return;
    }

    for (int i = 0; i < num_elements; i++)
    {
        temp_data[i] = _lps22hbData[i].temperature;
    }

    _unread_temperature_data = false;
}

bool Barometer::read_buffered_data()
{
    if (_barometer.get_fifo(_lps22hbData) == LPS22HB_ERROR)
    {
        LOG_WARNING("%s", "Unable to read barometer data");
        return false;
    }

    _unread_pressure_data = true;
    _unread_temperature_data = true;
    _bar_data_ready = false;
    return true;
}

bool Barometer::read_buffered_pressure()
{
    if (_barometer.get_pressure_fifo(_pressure_buffer) == LPS22HB_ERROR)
    {
        LOG_WARNING("%s", "Unable to read pressure data from fifo");
        return false;
    }

    _unread_pressure_data = true;
    _bar_data_ready = true;
    return true;
}

void Barometer::bar_data_ready()
{
    _bar_data_ready = true;
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