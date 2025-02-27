/** @file
 * @brief CRC-8 Functions
 * @defgroup ldc1000evm ldc1000evm
 * @{
 * @ingroup usb
 *
 * @brief <b>CRC-8 Functions</b> @n@n
 * This file contains functions to implement CRC-8.
 * CRC-8 with initial value = 0, poly = x^8 + x^2 + x^1 + x^0.
 * Lookup table methodology.
 *
 * @author
 * Charles Cheung (chuck@ti.com)
 */

#ifndef CRC8_H_
#define CRC8_H_

#include <stdint.h>

/** Calculate CRC-8 from a buffer
Calculates CRC-8 from a buffer
@param dataPtr pointer to buffer
@param size size of buffer
@return CRC
*/
uint8_t calcCRC8(uint8_t * dataPtr, uint8_t size);

#endif /* CRC8_H_ */

/** @} */
