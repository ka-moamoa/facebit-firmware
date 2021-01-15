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
        return false;
    }

    _barometer.reset();
    _barometer.sw_reset();

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

    if (_barometer.set_fifo_mode(2) == LPS22HB_ERROR) // Stream mode
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

    float odr = 255;
    _barometer.get_odr(&odr);
    LOG_INFO("odr = %0.2f\r\n", odr);

    uint8_t mode = 255;
    _barometer.get_fifo_mode(&mode);
    LOG_INFO("fifo mode = %u\r\n", mode);

    _int_pin.rise(callback(this, &Barometer::bar_data_ready));

    LOG_DEBUG("%s", "Barometer initialized successfully");
    _initialized = true;
    return true;
}

bool Barometer::update()
{
    // if the interrupt has been triggered, read the data
    if (_bar_data_ready)
    {
        read_buffered_pressure();
        return true;
    }

    uint8_t status = 0;
    _barometer.get_fifo_status(&status);
    // LOG_DEBUG("Fifo status = %u", status);

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

