/**
  ******************************************************************************
  * @file           : sgp30.h
  * @author         : Mauricio Barroso Benavides
  * @date           : Jul 18, 2023
  * @brief          : todo: write brief
  ******************************************************************************
  * @attention
  *
  * MIT License
  *
  * Copyright (c) 2023 Mauricio Barroso Benavides
  *
  * Permission is hereby granted, free of charge, to any person obtaining a copy
  * of this software and associated documentation files (the "Software"), to
  * deal in the Software without restriction, including without limitation the
  * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  * sell copies of the Software, and to permit persons to whom the Software is
  * furnished to do so, subject to the following conditions:
  *
  * The above copyright notice and this permission notice shall be included in
  * all copies or substantial portions of the Software.
  *
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  * IN THE SOFTWARE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef sgp30_H_
#define sgp30_H_

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

//#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C2 || CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32C6
#define ESP32_TARGET
//#endif

#ifdef ESP32_TARGET
#include "driver/i2c_master.h"
#else
#include "main.h"
#endif /* ESP32_TARGET */

/* Exported Macros -----------------------------------------------------------*/
#define SGP30_I2C_ADDR						0x58
#define SGP30_I2C_BUFFER_LEN_MAX			8

#define SGP30_CMD_READ_ID					0x3682 /* command: read ID register */
#define SGP30_CMD_SOFT_RESET				0x805D /* soft reset */

#define SGP30_CMD_IAQ_INIT		0x2003 /* meas. read T first, clock stretching disabled in normal mode */
#define SGP30_CMD_MEAS_IAQ		0x2008 /* meas. read T first, clock stretching enabled in normal mode */

/* Exported typedef ----------------------------------------------------------*/
typedef struct {
#ifdef ESP32_TARGET
	i2c_master_dev_handle_t handle;
#else
	uint8_t addr;
	I2C_HandleTypeDef *handle;
#endif /* ESP32_TARGET */
} sgp30_i2c_t;

typedef struct {
	sgp30_i2c_t i2c_dev;
} sgp30_t;

/* Exported variables --------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
/**
 * @brief Function to initialize a sgp30 instance
 *
 * @param me         : Pointer to a sgp30_t instance
 * @param i2c_handle : Pointer to a structure with the data to initialize the
 * 					   I2C device
 * @param dev_addr   : I2C device address
 *
 * @return ESP_OK on success
 */
int sgp30_init(sgp30_t *const me, void *i2c_handle, uint8_t dev_addr);

/**
 * @brief Function to get the device ID
 *
 * @param me : Pointer to a sgp30_t instance
 * @param id : Pointer to sgp30 ID
 *
 * @return ESP_OK on success
 */
int sgp30_get_id(sgp30_t *const me, uint16_t *id);

/**
 * @brief Function to get the temperature (°C) and humidity (%)
 *
 * @param me   : Pointer to a sgp30_t instance
 * @param temp : Pointer to floating point value, where the calculated
 *               temperature value will be stored
 * @param hum  : Pointer to floating point value, where the calculated
 *               humidity value will be stored
 *
 * @return ESP_OK on success
 */
int sgp30_get_co2_and_tvoc(sgp30_t *const me, int *co2, int *tvoc);

/**
 * @brief Function to perfrom a software reset of the device
 *
 * @param me : Pointer to a sgp30_t instance
 *
 * @return ESP_OK on success
 */
int sgp30_soft_reset(sgp30_t *const me);

#ifdef __cplusplus
}
#endif

#endif /* sgp30_H_ */

/***************************** END OF FILE ************************************/
