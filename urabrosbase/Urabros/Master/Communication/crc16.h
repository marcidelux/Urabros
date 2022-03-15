/**
  * @file     crc16.h
  * @author   Marton.Lorinczi
  * @date     Sep 2, 2020
  *
  * @brief    This file wasnt writen by me. I just copied the parts I needed on this project.
  *           I used this repo: https://github.com/lammertb/libcrc
  *           Alos this link is usefull too: https://www.lammertbies.nl/comm/info/crc-calculation 
  */

#ifndef CRC16_H_INCLUDED
#define CRC16_H_INCLUDED

#include <stddef.h>
#include <stdint.h>

/**
 * #define CRC_POLY_xxxx
 *
 * The constants of the form CRC_POLY_xxxx define the polynomials for some well
 * known CRC calculations.
 */
#define		CRC_POLY_16         0xA001

/**
 * #define CRC_START_xxxx
 *
 * The constants of the form CRC_START_xxxx define the values that are used for
 * initialization of a CRC value for common used calculation methods.
 */
#define		CRC_START_MODBUS	    0xFFFF

/**
  * @brief  Function for calculating CRC16 MODBUS value of the given uint8_t data array.
  * @param   input_str - Pointer to the data array
  * @param   num_bytes - Size of the data array
  * @return calculated Crc16 modbus value
*/
uint16_t crc_modbus(const unsigned char *input_str, size_t num_bytes);

#endif // CRC16_H_INCLUDED
