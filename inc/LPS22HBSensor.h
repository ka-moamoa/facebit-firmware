/**
 ******************************************************************************
 * @file    LPS22HBSensor.h
 * @author  CLab
 * @version V1.0.0
 * @date    5 August 2016
 * @brief   Abstract Class of an LPS22HB Pressure sensor.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */


/* Prevent recursive inclusion -----------------------------------------------*/

#ifndef __LPS22HBSensor_H__
#define __LPS22HBSensor_H__


/* Includes ------------------------------------------------------------------*/

#include "DevI2C.h"
#include "LPS22HB_driver.h"
#include "PressureSensor.h"
#include "TempSensor.h"
#include <assert.h>


#define FIFO_LENGTH (uint8_t)32
/* Data Types -------------------------------------------------------------*/
/** @defgroup LPS22HB_Data_Types
* @{
*/

/**
* @brief  LPS22HB output data structure.
*/
typedef struct
{
  float temperature;
  float pressure;
}LPS22HB_Data_st;

/**
* @}
*/

/* Class Declaration ---------------------------------------------------------*/

/**
 * Abstract class of an LPS22HB Pressure sensor.
 */
class LPS22HBSensor : public PressureSensor, public TempSensor
{
  public:
  
    enum SPI_type_t {SPI3W, SPI4W};  
    LPS22HBSensor(SPI *spi, PinName cs_pin, PinName int_pin=NC, SPI_type_t spi_type=SPI4W);     
    LPS22HBSensor(DevI2C *i2c, uint8_t address=LPS22HB_ADDRESS_HIGH, PinName int_pin=NC);
    virtual int init(void *init);
    virtual int read_id(uint8_t *id);
    virtual int get_pressure(float *pfData);
    virtual int get_temperature(float *pfData);
    int enable(void);
    int disable(void);
    int reset(void);
    int get_odr(float *odr);
    int set_odr(float odr);
    int read_reg(uint8_t reg, uint8_t *data);
    int write_reg(uint8_t reg, uint8_t data);
    int sw_reset(void);
    int enable_fifo(void);
    int get_fifo_enabled(uint8_t *enabled);
    int fifo_full_interrupt(bool enable);
    int set_interrupt_level(uint8_t intr);
    int enable_fifo_watermark(void);
    int set_fifo_watermark(uint8_t level);
    int get_fifo_watermark(uint8_t *level);
    int enable_fifo_watermark_interrupt(void);
    int set_fifo_mode(uint8_t mode);
    int get_fifo_mode(uint8_t *mode);
    int get_fifo_status(LPS22HB_FifoStatus_st *status);
    int get_fifo(LPS22HB_Data_st *data);
    int get_pressure_fifo(float *pfData);
    int get_temperature_fifo(float *pfData);
    int differential_interrupt(bool enable, bool high_pressure, bool low_pressure);
    int set_interrupt_pressure(int16_t hPa);
    int get_interrupt_status(LPS22HB_InterruptDiffStatus_st *int_source);
    /**
     * @brief Utility function to read data.
     * @param  pBuffer: pointer to data to be read.
     * @param  RegisterAddr: specifies internal address register to be read.
     * @param  NumByteToRead: number of bytes to be read.
     * @retval 0 if ok, an error code otherwise.
     */
    uint8_t io_read(uint8_t* pBuffer, uint8_t RegisterAddr, uint16_t NumByteToRead)
    {
        if (_dev_spi) {
        /* Write Reg Address */
            _dev_spi->lock();
            _cs_pin = 0;           
            if (_spi_type == SPI4W) {            
                _dev_spi->write(RegisterAddr | 0x80);
                for (int i=0; i<NumByteToRead; i++) {
                    *(pBuffer+i) = _dev_spi->write(0x00);
                }
            } else if (_spi_type == SPI3W){
                /* Write RD Reg Address with RD bit*/
                uint8_t TxByte = RegisterAddr | 0x80;    
                _dev_spi->write((char *)&TxByte, 1, (char *)pBuffer, (int) NumByteToRead);
            }            
            _cs_pin = 1;
            _dev_spi->unlock(); 
            return 0;
        }                       
        if (_dev_i2c) return (uint8_t) _dev_i2c->i2c_read(pBuffer, _address, RegisterAddr, NumByteToRead);
        return 1;
    }
    
    /**
     * @brief Utility function to write data.
     * @param  pBuffer: pointer to data to be written.
     * @param  RegisterAddr: specifies internal address register to be written.
     * @param  NumByteToWrite: number of bytes to write.
     * @retval 0 if ok, an error code otherwise.
     */
    uint8_t io_write(uint8_t* pBuffer, uint8_t RegisterAddr, uint16_t NumByteToWrite)
    {
        if (_dev_spi) { 
            _dev_spi->lock();
            _cs_pin = 0;
            _dev_spi->write(RegisterAddr);                    
            _dev_spi->write((char *)pBuffer, (int) NumByteToWrite, NULL, 0);                     
            _cs_pin = 1;                    
            _dev_spi->unlock();
            return 0;                    
        }        
        if (_dev_i2c) return (uint8_t) _dev_i2c->i2c_write(pBuffer, _address, RegisterAddr, NumByteToWrite);    
        return 1;
    }

  private:
    int Set_ODR_When_Enabled(float odr);
    int Set_ODR_When_Disabled(float odr);

    /* Helper classes. */
    DevI2C *_dev_i2c;
    SPI    *_dev_spi;     
    
    /* Configuration */
    uint8_t _address;
    DigitalOut  _cs_pin; 
    InterruptIn _int_pin;    
    SPI_type_t _spi_type;    
    
    uint8_t _is_enabled;
    float _last_odr;
};

#ifdef __cplusplus
 extern "C" {
#endif
uint8_t LPS22HB_io_write( void *handle, uint8_t WriteAddr, uint8_t *pBuffer, uint16_t nBytesToWrite );
uint8_t LPS22HB_io_read( void *handle, uint8_t ReadAddr, uint8_t *pBuffer, uint16_t nBytesToRead );
#ifdef __cplusplus
  }
#endif

#endif
