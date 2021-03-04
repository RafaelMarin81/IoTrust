/*
 * Copyright (c) 2018, Department of Information and Communication Engineering.
 * University of Murcia. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
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
 * Author(s):
 *            Jorge Gallego Madrid <jorge.gallego1@um.es>
 *            Jesús Sánchez Gómez  <jesus.sanchez4@um.es>
 */

#ifndef SCHC_H
#define SCHC_H

using namespace std;

/**
 * \file
 * 
 * \brief SCHC C/D related functions and struct definition.
 *
 * All the definitions in this file are directly from the
 * draft-ietf-lpwan-ipv6-static-context-hc-10.
 */

/**********************************************************************/
/***        Include files                                           ***/
/**********************************************************************/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <cstring>

/**********************************************************************/
/***        Local Include files                                     ***/
/**********************************************************************/

#include "lorawan.h"
#include "utils.h"
#include "ipv6_layer.h"

/**********************************************************************/
/***        Macro Definitions                                       ***/
/**********************************************************************/

/**
 * This is taken from the LoRaWAN Specification v1.0 Table 17.
 *
 * No matter what, no SCHC packet length should ever be longer than
 * this.
 *
 * Not really a generic solution, since every LPWAN has a different PTU,
 * but for now it servers as an ad-hoc solution for testings.
 *
 * \note See table 17 of the LoRaWAN specification 1R0.pdf for other
 * values.
 */
#define MAX_LORAWAN_PKT_LEN 242

/**
 * The maximum length allowed by schc_fragmentate() to decide if the
 * SCHC packet should be fragmentated or not. All packets longer than
 * this get fragmentated.
 */
#define MAX_SCHC_PKT_LEN 40

/*
 * This is the maximum number of CoAP header Option fields supported
 * by this implementation. If your project requires a larger number,
 * increase this define. Note that this will also significantly increase
 * the static memory consumption of the project.
 */
#define SCHC_MAX_COAP_OPT 4


// Fragmentation/Reassembly {

/**
 * This is the SCHC Fragment Tile Size
 */
#define SCHC_FRG_TILE_SIZE 40 /* TODO this value is set for testing, change to something else */

// } Fragmentation/Reassembly
//
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))


#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

#define SCHC_FRG_RULEID 0x80



/**********************************************************************/
/***        Types Definitions                                       ***/
/**********************************************************************/

// SCHC draft 10, section 6.4
enum MO {
	EQUALS,
	IGNORE,
	MATCH_MAPPING,
	MSB
};

// SCHC draft 10, section 9
enum fieldid {
	IPV6_VERSION,
	IPV6_TRAFFIC_CLASS,
	IPV6_FLOW_LABEL,
	IPV6_PAYLOAD_LENGTH,
	IPV6_NEXT_HEADER,
	IPV6_HOP_LIMIT,
	IPV6_DEV_PREFIX,
	IPV6_DEVIID,
	IPV6_APP_PREFIX,
	IPV6_APPIID,

	UDP_DEVPORT,
	UDP_APPPORT,
	UDP_LENGTH,
	UDP_CHECKSUM,

	COAP_VERSION,
	COAP_TYPE,
	COAP_TKL,
	COAP_CODE,
	COAP_MESSAGE_ID,
	COAP_TOKEN,
	COAP_OPTION_DELTA,
	COAP_OPTION_LENGTH,
	COAP_OPTION_VALUE,

   END_OF_RULE,
};

// SCHC draft 10, section 6.1
enum direction {
	UPLINK,
	DOWNLINK,
	BI
};

// SCHC draft 10, section 6.5
enum CDA {
	NOT_SENT,
	VALUE_SENT,
	MAPPING_SENT,
	LSB,
	COMPUTE_LENGTH,
	COMPUTE_CHECKSUM,
	DEVIID,
	APPIID
};


struct field_description {
	enum fieldid fieldid;
	size_t field_length; /** Length in bits */
	int field_position;
	enum direction direction;
	char *tv;
	enum MO MO;
	enum CDA CDA;
};

struct field_values {
	uint8_t ipv6_version;
	uint8_t ipv6_traffic_class;
	uint32_t ipv6_flow_label;
	size_t ipv6_payload_length;
	uint8_t ipv6_next_header;
	uint8_t ipv6_hop_limit;
	uint8_t ipv6_dev_prefix[8];
	uint8_t ipv6_dev_iid[8];
	uint8_t ipv6_app_prefix[8];
	uint8_t ipv6_app_iid[8];

	uint16_t udp_dev_port;
	uint16_t udp_app_port;
	size_t udp_length;
	uint16_t udp_checksum;
	
	// TODO: consider what to do with this field
	// IPv6/UDP packet (i.e. non CoAP).
	//uint8_t payload[SIZE_MTU_IPV6]; 

	// CoAP header
	uint8_t coap_version;
	uint8_t coap_type;
	uint8_t coap_tkl;
	uint8_t coap_code;
	uint8_t coap_message_id[2];
	// CoAP 
	uint8_t coap_token[16];
	// CoAP option
   uint8_t coap_option_num; // Number of following CoAP Options stored in the struct
   struct {
     uint16_t delta;
     uint16_t length;
     uint8_t value[16];
   } coap_option[SCHC_MAX_COAP_OPT];

	// CoAP payload
	uint8_t coap_payload_length;
	uint8_t coap_payload[SIZE_MTU_IPV6];
};


/**********************************************************************/
/***        Forward Declarations                                    ***/
/**********************************************************************/

int string_to_bin(uint8_t dst[8], const char *src);

/**
 * \brief Applies the SCHC compression procedure as detailed in
 * draft-ietf-lpwan-ipv6-static-context-hc-10 and, in case of success,
 * sends the SCHC Packet or derived SCHC Fragments though the downlink.
 *
 * \note In case of failure or not matching any SCHC Rule, the packet is
 * silently discarded and is not sent through the downlink.
 *
 * @param [in] ipv6_pcaket The original IPv6 packet received from
 * the ipv6 interface. It will be used to apply a SCHC rule and generate
 * a compressed IPv6 Packet.
 *
 * @return 0 if successfull, non-zero if there was an error.
 */
int schc_compress(const struct field_values ipv6_packet);

/*
 * TODO comment
 */
int schc_reassemble(uint8_t *lorawan_payload, uint8_t lorawan_payload_len);

/**********************************************************************/
/***        Constants                                               ***/
/**********************************************************************/

/**********************************************************************/
/***        Global Variables                                        ***/
/**********************************************************************/

/**********************************************************************/
/***        AUX Functions                                           ***/
/**********************************************************************/

/**********************************************************************/
/***        MAIN routines                                           ***/
/**********************************************************************/

/**********************************************************************/
/***        END OF FILE                                             ***/
/**********************************************************************/


#endif /* SCHC_H */

// vim:tw=72
