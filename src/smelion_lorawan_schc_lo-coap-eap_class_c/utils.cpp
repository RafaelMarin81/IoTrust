
/**********************************************************************/
/***        Include files                                           ***/
/**********************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/**********************************************************************/
/***        Local Include files                                     ***/
/**********************************************************************/

#include "utils.h"

/**********************************************************************/
/***        Macro Definitions                                       ***/
/**********************************************************************/

/**********************************************************************/
/***        Type Definitions                                        ***/
/**********************************************************************/

/**********************************************************************/
/***        Forward Declarations                                    ***/
/**********************************************************************/

/**********************************************************************/
/***        Constants                                               ***/
/**********************************************************************/

/**********************************************************************/
/***        Global Variables                                        ***/
/**********************************************************************/

/**********************************************************************/
/***        Static Variables                                        ***/
/**********************************************************************/

/**********************************************************************/
/***        Static Functions                                        ***/
/**********************************************************************/

/**********************************************************************/
/***        Public Functions                                        ***/
/**********************************************************************/


/**
 * Computes the CRC-16 bit one-complement. This is needed as a subroutine
 * for computing the UDP Checksum.
 *
 * @param [in] data The source array of bytes to compute the checksum.
 *
 * @param [in] len The length of data. It can be an odd number too.
 *
 * @return The computed CRC-16 in one's complement.
 *
 * \note Dependencies #include <stdint.h>
 * \note Dependencies #include <arpa/inet.h>
 *
 * \note Reference: https://barrgroup.com/Embedded-Systems/How-To/Additive-Checksums
 */
uint16_t crc16(const uint8_t *data, size_t len)
{
	register uint32_t sum = 0;
	const uint16_t *p = (const uint16_t *)data;
	uint16_t upper_half = 0;
	uint16_t lower_half = 0;

	if (len == 0) {
		return 0xFFFF;
	}


	for (; len > 1 ; len -= 2) {
		sum = sum + *p;
		p++; // Pointer moves to the next uint16_t address.
	}

	/*
	 * The length is an odd number, we must add also the last byte of the array.
	 */
	if (len != 0) {
		sum = sum + *(uint8_t*)p;
	}

	/*
	 * We fold the 32 bits into 16 bits until we don't have any bits in the upper
	 * half of uint32_t sum.
	 */
	upper_half = sum >> 16;
	lower_half = sum & 0xFFFF;
	do {

		sum = upper_half + lower_half;

		upper_half = sum >> 16;
		lower_half = sum & 0xFFFF;

	} while (upper_half != 0);

	sum = htons(sum);

	return (uint16_t) ~sum;

}

/**********************************************************************/
/***        main() setup() loop()                                   ***/
/**********************************************************************/

/**********************************************************************/
/***        END OF FILE                                             ***/
/**********************************************************************/

/*
 * Deprecated or testing functions start here.
 */
