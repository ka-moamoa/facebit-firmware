/**
 ******************************************************************************
 * @file    LPS22HBSensor.cpp
 * @author  CLab
 * @version V1.0.0
 * @date    5 August 2016
 * @brief   Implementation of an LPS22HB Pressure sensor.
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


/* Includes ------------------------------------------------------------------*/


#include "LPS22HBSensor.h"


/* Class Implementation ------------------------------------------------------*/

LPS22HBSensor::LPS22HBSensor(SPI *spi, PinName cs_pin, PinName int_pin, SPI_type_t spi_type)  : _dev_spi(spi), _cs_pin(cs_pin), _int_pin(int_pin), _spi_type(spi_type)
{
    assert (spi);
    if (cs_pin == NC) 
    {
        printf ("ERROR LPS22HBSensor CS MUST NOT BE NC\n\r");       
        _dev_spi = NULL;
        _dev_i2c=NULL;
        return;
    }       

    _dev_i2c=NULL;    
}

/** Constructor
 * @param i2c object of an helper class which handles the I2C peripheral
 * @param address the address of the component's instance
 */
LPS22HBSensor::LPS22HBSensor(DevI2C *i2c, uint8_t address, PinName int_pin) : 
                            _dev_i2c(i2c), _address(address), _cs_pin(NC), _int_pin(int_pin)
{
    assert (i2c);
    _dev_spi = NULL;
    LPS22HB_Set_I2C ((void *)this, LPS22HB_ENABLE);         
};

/**
 * @brief     Initializing the component.
 * @param[in] init pointer to device specific initalization structure.
 * @retval    "0" in case of success, an error code otherwise.
 */
int LPS22HBSensor::init(void *init)
{
  _cs_pin = 1;    

  LPS22HB_Set_I2C ((void *)this, LPS22HB_DISABLE);

  LPS22HB_Init((void *)this);

  // LPS22HB_Set_I2C((void *)this, LPS22HB_DISABLE);

  _is_enabled = 0;
  _last_odr = 25.0f;
  
  return 0;
}


/**
 * @brief  Enable LPS22HB
 * @retval 0 in case of success, an error code otherwise
 */
int LPS22HBSensor::enable(void)
{
  /* Check if the component is already enabled */
  if ( _is_enabled == 1 )
  {
    return 0;
  }
  
  if(Set_ODR_When_Enabled(_last_odr) == 1)
  {
    return 1;
  }
  
  _is_enabled = 1;

  return 0;
}

/**
 * @brief  Disable LPS22HB
 * @retval 0 in case of success, an error code otherwise
 */
int LPS22HBSensor::disable(void)
{
  /* Check if the component is already disabled */
  if ( _is_enabled == 0 )
  {
    return 0;
  }
  
  /* Power down the device */
  if ( LPS22HB_Set_Odr( (void *)this, LPS22HB_ODR_ONE_SHOT ) == LPS22HB_ERROR )
  {
    return 1;
  }
  
  _is_enabled = 0;

  return 0;
}

/**
 * @brief  Read ID address of LPS22HB
 * @param  id the pointer where the ID of the device is stored
 * @retval 0 in case of success, an error code otherwise
 */
int LPS22HBSensor::read_id(uint8_t *id)
{
  if(!id)
  { 
    return 1;
  }
 
  /* Read WHO AM I register */
  if ( LPS22HB_Get_DeviceID( (void *)this, id ) == LPS22HB_ERROR )
  {
    return 1;
  }

  return 0;
}

/**
 * @brief  Reboot memory content of LPS22HB
 * @param  None
 * @retval 0 in case of success, an error code otherwise
 */
int LPS22HBSensor::reset(void)
{
  /* Read WHO AM I register */
  if ( LPS22HB_MemoryBoot((void *)this) == LPS22HB_ERROR )
  {
    return 1;
  }

  return 0;
}

/**
 * @brief  Read LPS22HB output register, and calculate the pressure in mbar
 * @param  pfData the pressure value in mbar
 * @retval 0 in case of success, an error code otherwise
 */
int LPS22HBSensor::get_pressure(float* pfData)
{
  int32_t int32data = 0;

  /* Read data from LPS22HB. */
  if ( LPS22HB_Get_Pressure( (void *)this, &int32data ) == LPS22HB_ERROR )
  {
    return 1;
  }

  *pfData = ( float )int32data / 100.0f;

  return 0;
}

/**
 * @brief  Read LPS22HB output register, and calculate the temperature
 * @param  pfData the temperature value
 * @retval 0 in case of success, an error code otherwise
 */
int LPS22HBSensor::get_temperature(float *pfData)
{
  int16_t int16data = 0;

  /* Read data from LPS22HB. */
  if ( LPS22HB_Get_Temperature( (void *)this, &int16data ) == LPS22HB_ERROR )
  {
    return 1;
  }

  *pfData = ( float )int16data / 10.0f;

  return 0;
}

/**
 * @brief  Read LPS22HB output data rate
 * @param  odr the pointer to the output data rate
 * @retval 0 in case of success, an error code otherwise
 */
int LPS22HBSensor::get_odr(float* odr)
{
  LPS22HB_Odr_et odr_low_level;

  if ( LPS22HB_Get_Odr( (void *)this, &odr_low_level ) == LPS22HB_ERROR )
  {
    return 1;
  }

  switch( odr_low_level )
  {
    case LPS22HB_ODR_ONE_SHOT:
      *odr = 0.0f;
      break;
    case LPS22HB_ODR_1HZ:
      *odr = 1.0f;
      break;
    case LPS22HB_ODR_10HZ:
      *odr = 10.0f;
      break;
    case LPS22HB_ODR_25HZ:
      *odr = 25.0f;
      break;
    case LPS22HB_ODR_50HZ:
      *odr = 50.0f;
      break;
    case LPS22HB_ODR_75HZ:
      *odr = 75.0f;
      break;
    default:
      *odr = -1.0f;
      return 1;
  }

  return 0;
}

/**
 * @brief  Set ODR
 * @param  odr the output data rate to be set
 * @retval 0 in case of success, an error code otherwise
 */
int LPS22HBSensor::set_odr(float odr)
{
  if(_is_enabled == 1)
  {
    if(Set_ODR_When_Enabled(odr) == 1)
    {
      return 1;
    }
  }
  else
  {
    if(Set_ODR_When_Disabled(odr) == 1)
    {
      return 1;
    }
  }

  return 0;
}


/**
 * @brief Set the LPS22HB sensor output data rate when enabled
 * @param odr the functional output data rate to be set
 * @retval 0 in case of success
 * @retval 1 in case of failure
 */
int LPS22HBSensor::Set_ODR_When_Enabled( float odr )
{
  LPS22HB_Odr_et new_odr;

  new_odr = ( odr <=  1.0f ) ? LPS22HB_ODR_1HZ
          : ( odr <= 10.0f ) ? LPS22HB_ODR_10HZ
          : ( odr <= 25.0f ) ? LPS22HB_ODR_25HZ
          : ( odr <= 50.0f ) ? LPS22HB_ODR_50HZ
          :                    LPS22HB_ODR_75HZ;

  if ( LPS22HB_Set_Odr( (void *)this, new_odr ) == LPS22HB_ERROR )
  {
    return 1;
  }

  if ( get_odr( &_last_odr ) == 1 )
  {
    return 1;
  }

  return 0;
}

/**
 * @brief Set the LPS22HB sensor output data rate when disabled
 * @param odr the functional output data rate to be set
 * @retval 0 in case of success
 * @retval 1 in case of failure
 */
int LPS22HBSensor::Set_ODR_When_Disabled( float odr )
{
  _last_odr = ( odr <=  1.0f ) ? 1.0f
           : ( odr <= 10.0f ) ? 10.0f
           : ( odr <= 25.0f ) ? 25.0f
           : ( odr <= 50.0f ) ? 50.0f
           :                    75.0f;

  return 0;
}


/**
 * @brief Read the data from register
 * @param reg register address
 * @param data register data
 * @retval 0 in case of success
 * @retval 1 in case of failure
 */
int LPS22HBSensor::read_reg( uint8_t reg, uint8_t *data )
{

  if ( LPS22HB_read_reg( (void *)this, reg, 1, data ) == LPS22HB_ERROR )
  {
    return 1;
  }

  return 0;
}

/**
 * @brief Write the data to register
 * @param reg register address
 * @param data register data
 * @retval 0 in case of success
 * @retval 1 in case of failure
 */
int LPS22HBSensor::write_reg( uint8_t reg, uint8_t data )
{

  if ( LPS22HB_write_reg( (void *)this, reg, 1, &data ) == LPS22HB_ERROR )
  {
    return 1;
  }

  return 0;
}

/**
 * @brief  Software reset LPS22HB
 * @param  None
 * @retval 0 in case of success, an error code otherwise
 */
int LPS22HBSensor::sw_reset(void)
{
  /* Read WHO AM I register */
  if ( LPS22HB_SwReset((void *)this) == LPS22HB_ERROR )
  {
    return 1;
  }

  return 0;
}

/**
 * @brief  Enable LPS22HB FIFO
 * @retval 0 in case of success, an error code otherwise
 */
int LPS22HBSensor::enable_fifo(void)
{
  if(LPS22HB_Set_FifoModeUse( (void *)this, LPS22HB_ENABLE) == LPS22HB_ERROR)
  {
    return 1;
  }
  return 0;
}

int LPS22HBSensor::get_fifo_enabled(uint8_t *enabled)
{
  uint8_t tmp;
  if (LPS22HB_GetCtrlReg2((void *)this, &tmp) == LPS22HB_ERROR)
  {
    return 1;
  }

  *enabled = (uint8_t)((tmp & LPS22HB_FIFO_EN_MASK)>>LPS22HB_FIFO_EN_BIT);

  return 0;
}

/**
 * @brief  Enable LPS22HB full interrupt
 * @retval 0 in case of success, an error code otherwise
 */
int LPS22HBSensor::fifo_full_interrupt(bool enable)
{
  if(LPS22HB_Set_FIFO_FULL_Interrupt( (void *)this, enable ? LPS22HB_ENABLE : LPS22HB_DISABLE) == LPS22HB_ERROR)
  {
    return 1;
  }
  return 0;
}

/**
 * @brief  Set interrupt level
 * @retval 0 in case of success, an error code otherwise
 */
int LPS22HBSensor::set_interrupt_level(uint8_t intr)
{
  LPS22HB_InterruptActiveLevel_et interrupt_level = (intr == 0) ? LPS22HB_ActiveLow : LPS22HB_ActiveHigh;
  if(LPS22HB_Set_InterruptActiveLevel( (void *)this, interrupt_level) == LPS22HB_ERROR)
  {
    return 1;
  }
  return 0;
}

/**
 * @brief  Enable LPS22HB FIFO watermark level use
 * @retval 0 in case of success, an error code otherwise
 */
int LPS22HBSensor::enable_fifo_watermark(void)
{
  if(LPS22HB_Set_FifoWatermarkLevelUse( (void *)this, LPS22HB_ENABLE) == LPS22HB_ERROR)
  {
    return 1;
  }
  return 0;
}

/**
 * @brief  get FIFO mode of LPS22HB
 * @param  the pointer to the mode of FIFO
 * @retval 0 in case of success, an error code otherwise
 */
int LPS22HBSensor::get_fifo_watermark(uint8_t *level)
{
  uint8_t new_level;
  if(LPS22HB_Get_FifoWatermarkLevel( (void *)this, &new_level) == LPS22HB_ERROR)
  {
    return 1;
  }
  *level = new_level;
  return 0;
}

/**
 * @brief  Set FIFO mode of LPS22HB
 * @param the mode of FIFO to be set
 * @retval 0 in case of success, an error code otherwise
 */
int LPS22HBSensor::set_fifo_watermark(uint8_t level)
{
  if(level > 15)
    level = 15;
  if(LPS22HB_Set_FifoWatermarkLevel( (void *)this, level) == LPS22HB_ERROR)
  {
    return 1;
  }
  return 0;
}

/**
 * @brief  Enable LPS22HB FIFO watermark interrupt
 * @retval 0 in case of success, an error code otherwise
 */
int LPS22HBSensor::enable_fifo_watermark_interrupt(void)
{
  if(LPS22HB_Set_FIFO_FTH_Interrupt( (void *)this, LPS22HB_ENABLE) == LPS22HB_ERROR)
  {
    return 1;
  }
  return 0;
}

/**
 * @brief  get FIFO mode of LPS22HB
 * @param  the pointer to the mode of FIFO
 * @retval 0 in case of success, an error code otherwise
 */
int LPS22HBSensor::get_fifo_mode(uint8_t *mode)
{
  LPS22HB_FifoMode_et new_mode;
  if(LPS22HB_Get_FifoMode( (void *)this, &new_mode) == LPS22HB_ERROR)
  {
    return 1;
  }
  switch( new_mode )
  {
    case LPS22HB_FIFO_BYPASS_MODE:
      *mode = 0;
      break;
    case LPS22HB_FIFO_MODE:
      *mode = 1;
      break;
    case LPS22HB_FIFO_STREAM_MODE:
      *mode = 2;
      break;
    case LPS22HB_FIFO_TRIGGER_STREAMTOFIFO_MODE:
      *mode = 3;
      break;
    case LPS22HB_FIFO_TRIGGER_BYPASSTOSTREAM_MODE:
      *mode = 4;
      break;
    case LPS22HB_FIFO_TRIGGER_BYPASSTOFIFO_MODE:
      *mode = 5;
      break;
    default:
      *mode = -1;
      return 1;
  }
  return 0;
}

/**
 * @brief  Set FIFO mode of LPS22HB
 * @param the mode of FIFO to be set
 * @retval 0 in case of success, an error code otherwise
 */
int LPS22HBSensor::set_fifo_mode(uint8_t mode)
{
  LPS22HB_FifoMode_et new_mode;

  new_mode =  ( mode ==  0 ) ? LPS22HB_FIFO_BYPASS_MODE
            : ( mode ==  1 ) ? LPS22HB_FIFO_MODE
            : ( mode ==  2 ) ? LPS22HB_FIFO_STREAM_MODE
            : ( mode ==  3 ) ? LPS22HB_FIFO_TRIGGER_STREAMTOFIFO_MODE
            : ( mode ==  4 ) ? LPS22HB_FIFO_TRIGGER_BYPASSTOSTREAM_MODE
            :                  LPS22HB_FIFO_TRIGGER_BYPASSTOFIFO_MODE;

  if(LPS22HB_Set_FifoMode( (void *)this, new_mode) == LPS22HB_ERROR)
  {
    return 1;
  }
  return 0;
}

/**
 * @brief  get FIFO mode of LPS22HB
 * @param  the pointer to the FIFO status register
 * @retval 0 in case of success, an error code otherwise
 */
int LPS22HBSensor::get_fifo_status(LPS22HB_FifoStatus_st *status)
{
  LPS22HB_FifoStatus_st new_status;
  if(LPS22HB_Get_FifoStatus( (void *)this, &new_status) == LPS22HB_ERROR)
  {
    return 1;
  }
  *status = new_status;
  return 0;
}

/**
 * @brief  Read LPS22HB output temperature FIFO, and calculate the temperature
 * @param  pfData the temperature value array
 * @retval 0 in case of success, an error code otherwise
 */
int LPS22HBSensor::get_temperature_fifo(float *pfData)
{
  int16_t int16data = 0;
  uint8_t i = 0;

  for(i=0; i<FIFO_LENGTH; i++)
  {
    /* Read data from LPS22HB. */
    if ( LPS22HB_Get_Temperature( (void *)this, &int16data ) == LPS22HB_ERROR )
    {
      return 1;
    }
    pfData[i] = ( float )int16data / 10.0f;
  }
  return 0;
}

/**
 * @brief  Read LPS22HB output pressure FIFO, and calculate the pressure
 * @param  pfData the pressure value array
 * @retval 0 in case of success, an error code otherwise
 */
int LPS22HBSensor::get_pressure_fifo(float *pfData)
{
  int32_t int32data = 0;
  uint8_t i = 0;

  for(i=0; i<FIFO_LENGTH; i++)
  {
    /* Read data from LPS22HB. */
    if ( LPS22HB_Get_Pressure( (void *)this, &int32data ) == LPS22HB_ERROR )
    {
      return 1;
    }
    pfData[i] = ( float )int32data / 100.0f;
  }
  return 0;
}

int LPS22HBSensor::get_fifo(std::queue<uint32_t> &pressure_buffer, std::queue<uint32_t> &temperature_buffer)
{
  for (int i = 0; i < FIFO_LENGTH; i++)
  {
    int32_t pressure_data = 0;
    if (LPS22HB_Get_Pressure((void *)this, &pressure_data) == LPS22HB_ERROR)
    {
      return 1;
    }

    pressure_buffer.push(pressure_data);

    int16_t temp_data = 0;
    if (LPS22HB_Get_Temperature((void *)this, &temp_data) == LPS22HB_ERROR)
    {
      return 1;
    }

    temperature_buffer.push(temp_data);
  }
  
  return 0;
}

int LPS22HBSensor::differential_interrupt(bool enable, bool high_pressure, bool low_pressure)
{
  if (LPS22HB_Set_InterruptDifferentialGeneration((void *)this, enable ? LPS22HB_ENABLE : LPS22HB_DISABLE) == LPS22HB_ERROR)
  {
    return 1;
  }

  if (LPS22HB_Set_PHE((void *)this, high_pressure ? LPS22HB_ENABLE : LPS22HB_DISABLE) == LPS22HB_ERROR)
  {
    return 1;
  }

  if (LPS22HB_Set_PLE((void *)this, low_pressure ? LPS22HB_ENABLE : LPS22HB_DISABLE) == LPS22HB_ERROR)
  {
    return 1;
  }

  return 0;
}

int LPS22HBSensor::set_interrupt_pressure(int16_t hPa)
{
  if (LPS22HB_Set_PressureThreshold((void *)this, hPa) == LPS22HB_ERROR)
  {
    return 1;
  }

  return 0;
}

int LPS22HBSensor::get_interrupt_status(LPS22HB_InterruptDiffStatus_st *int_source)
{
  if (LPS22HB_Get_InterruptDifferentialEventStatus((void *)this, int_source))
  {
    return 1;
  }

  return 0;
}

uint8_t LPS22HB_io_write( void *handle, uint8_t WriteAddr, uint8_t *pBuffer, uint16_t nBytesToWrite )
{
  return ((LPS22HBSensor *)handle)->io_write(pBuffer, WriteAddr, nBytesToWrite);
}

uint8_t LPS22HB_io_read( void *handle, uint8_t ReadAddr, uint8_t *pBuffer, uint16_t nBytesToRead )
{
  return ((LPS22HBSensor *)handle)->io_read(pBuffer, ReadAddr, nBytesToRead);
}
