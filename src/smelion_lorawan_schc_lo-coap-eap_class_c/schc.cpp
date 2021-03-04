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

/**
 * \file
 * \brief Implementation of the schc.h functions.
 *
 * This file implements the SCHC compression/decompression (SCHC C/D)
 * actions as defined in draft-ietf-lpwan-ipv6-static-context-hc-10. The
 * most important functions in this file are schc_compress() and
 * schc_decompress().
 *
 * \note Only a subset of operations of the SCHC C/D are implemented,
 * not all of them.
 *
 * \note The SCHC Fragmentation/Reassembly (SCHC F/R) is not
 * implemented.
 *
 * \note The rule Field Length is not used at all in this
 * implementation, we hardcoded everything in the struct
 * field_description. Solving this requires thinking.
 *
 * To understand better the structure of a schc_packet:
 *
 * \verbatim
 * +--- ... --+------- ... -------+------------------+~~~~~~~
 * |  Rule ID |Compression Residue| packet payload   | padding
 * +--- ... --+------- ... -------+------------------+~~~~~~~
 *                                                    (optional)
 * <----- compressed header ------>
 *
 * Figure 6: from the draft-ietf-lpwan-ipv6-static-context-hc-10
 *
 * \endverbatim
 *
 * \related schc_compress
 */

/**********************************************************************/
/***        Include files                                           ***/
/**********************************************************************/

#include <Arduino.h> // Used only for Serial. for Debug PRINTs
#include <CircularBuffer.h> //Used for main lorawan_msg_tx_cb circular buffers

/**********************************************************************/
/***        Local Include files                                     ***/
/**********************************************************************/

#include "schc.h"
#include "ipv6_layer.h"
#include "context.h"
#include "utils.h"

/**********************************************************************/
/***        Macro Definitions                                       ***/
/**********************************************************************/

// #define DEBUG

#ifdef DEBUG

	#warning "DEBUG MODE ACTIVATED!"

	#define PRINT(...) Serial.print(__VA_ARGS__)
	#define PRINTLN(...) Serial.println(__VA_ARGS__)
	#define PRINT_ARRAY(add, len) \
	do { \
		int debug_i_; \
		for (debug_i_ = 0 ; debug_i_ < (len) ; debug_i_++) { \
			if (debug_i_ % 10 == 0) \
				Serial.println(); \
			if (debug_i_ % 50 == 0) \
				Serial.println(); \
			Serial.print((((uint8_t*)(add))[debug_i_]), HEX); \
			Serial.print(" "); \
		} \
		Serial.println(); \
	} while(0)

#else /* DEBUG */

	#define PRINT(...)
	#define PRINTLN(...)
	#define PRINT_ARRAY(add, len)

#endif /* DEBUG */


/**********************************************************************/
/***        Type Definitions                                        ***/
/**********************************************************************/

typedef struct __attribute__((packed)) schc_fragment_s {
	uint8_t rule_id;
	uint8_t fcn;
	uint8_t payload[SCHC_FRG_TILE_SIZE];
} schc_fragment;


/**********************************************************************/
/***        Forward Declarations                                    ***/
/**********************************************************************/


/*
 * Extract in fv the data of the compressed packet used the SCHC decompression
 * procedure as detailed in draft-ietf-lpwan-ipv6-static-context-hc-10.
 * @return 0 if successfull, non-zero if there was an error.
 */
static int schc_decompress(uint8_t *schc_packet, size_t schc_packet_len);

/**********************************************************************/
/***        Constants                                               ***/
/**********************************************************************/

/**********************************************************************/
/***        Global Variables                                        ***/
/**********************************************************************/


extern CircularBuffer<uint8_t,2048> lorawan_msg_tx_cb;     // LoRaWAN messages transmission circular buffer
extern CircularBuffer<uint8_t,2048> lorawan_msg_rx_cb;     // LoRaWAN messages received circular buffer


/**********************************************************************/
/***        Static Variables                                        ***/
/**********************************************************************/

// SCHC Fragmentation/Reassembly {

static uint8_t schc_reassemble_buf[SIZE_MTU_IPV6] = {0};
static size_t  schc_reassemble_offset = {0};

// }

static uint16_t compute_checksum = 0;
static uint16_t compute_length = 0;
/**********************************************************************/
/***        Static Functions                                        ***/
/**********************************************************************/



static int schc_fragmentate(const uint8_t *schc_packet, size_t schc_packet_len)
{

	/*
	 * If the packet len is equal or less than the max size of a
	 * L2 packet, we send the packet as is, without fragmentation
	 */
	if (schc_packet_len <= MAX_SCHC_PKT_LEN) {


		/* Insert lorawan TX packet in the message circular buffer */
		/* This message will be treated asyncroniously by the main state machine */
		PRINT_ARRAY(schc_packet, schc_packet_len);

		lorawan_msg_tx_cb.push(schc_packet_len);
		int j;
		for (j = 0 ; j < schc_packet_len; j++) {
			lorawan_msg_tx_cb.push(schc_packet[j]);
		}

		return 0;

	}

	/*
	 * We start the fragmentation procedure.
	 */



	int nfrag = schc_packet_len / SCHC_FRG_TILE_SIZE + (schc_packet_len % SCHC_FRG_TILE_SIZE != 0);

	schc_fragment fragments[nfrag];

	memset(fragments, 0, sizeof(fragments));

	const uint8_t *offset = schc_packet;
	size_t bytes_left = schc_packet_len;

	uint8_t fr_buff_len = 0;
	uint8_t fr_buff[255] = {0};

	for (int i = 0 ; i < nfrag ; i++) {

		fragments[i].rule_id = 0x80; /* TODO hardcoded, make this configurable */
		fragments[i].fcn = 0; // Not the last fragment

		size_t frg_siz = MIN(sizeof(fragments[i].payload), bytes_left);

		memcpy(fragments[i].payload, offset, frg_siz);

		// We send the LoRaWAN packet {


		if (i + 1 < nfrag) {
			// This is not the last SCHC_Fragment
			memcpy(fr_buff, &fragments[i], MIN(sizeof(fragments[i]), sizeof(fr_buff)));
			fr_buff_len = frg_siz + (sizeof(fragments[i]) - sizeof(fragments[i].payload));
		} else {
			// This is the Last Fragment
			// uint16_t checksum (uint16_t *addr, int len);

			uint16_t mic = crc16(schc_packet, schc_packet_len);

			mic = htons(mic);

			fr_buff[0] = 0x80; // TODO hardcoded, make this configurable.
			fr_buff[1] = 0xFF; // Last Fragment, FCN == all 1s

			memcpy(&fr_buff[2], &mic, sizeof(mic));
			memcpy(&fr_buff[4], fragments[i].payload, MIN(sizeof(fr_buff), frg_siz));
			fr_buff_len = frg_siz + sizeof(mic) + (sizeof(fragments[i]) - sizeof(fragments[i].payload));

			
		}



		/* Insert lorawan TX packet in the message circular buffer */
		/* This message will be treated asyncroniously by the main state machine */
		// {
		PRINT_ARRAY(fr_buff, fr_buff_len);

		lorawan_msg_tx_cb.push(fr_buff_len);
		int j;
		for (j = 0 ; j < fr_buff_len; j++) {
			lorawan_msg_tx_cb.push(fr_buff[j]);
		}


		// } We insert the LoRaWAN message in the Tx buffer

		offset += frg_siz;
		bytes_left -= frg_siz;
	}


	return 0;


}


/**
 * \brief Appends the compression residue to the SCHC packet, using the
 * CA action defined in rule_row.
 *
 * \note That this function might not append any bytes to the
 * compression residue. Such thing is possible depending on the rule.
 *
 * @param [in] rule_row The rule row to check the Compression Action
 * (CA) to do to the ipv6_packet target value. Must not be NULL.
 *
 * @param [in] ipv6_packet The original ipv6_packet from wihch we
 * extract the information in case we need to copy some information to
 * the compression residue.
 *
 * @param [in,out] schc_packet The target SCHC compressed packet result of
 * aplying the CA. The function appends as many bytes as it needs
 * depending on the rule_row->CDA. Must be not-null. If the function
 * returns error, the contents of the schc_packet are undefined.
 *
 * @param [in,out] offset The position in schc_packet where the
 * function must append bytes to the Compression Residue. Once the
 * function returns, it will point to the next available byte in the
 * schc_packet. The idea is to call this function many times in
 * sequence, every call will append new bytes to the schc_packet, and
 * after the last call, it points where the application payload should
 * be.
 *
 * @param [in,out] current_coap_option_num In the structure of
 * field_values we defined a configurable number of supported CoAP
 * Options. We can specify n CoAP Option rules in the context, so this
 * variable serves to keep track of what values we must read/write.
 *
 * @return Zero if success, non-zero if error.
 *
 * \note Only implements the following CA:
 * - COMPUTE_LENGTH
 * - COMPUTE_CHECKSUM
 * - NOT_SENT
 *   TODO implement the rest.
 *
 * We just need a quick implementation to start testing with a real
 * scenario, so we don't bother with all the complex compression
 * actions, like the MSB or Map Matching. We need an ad-hoc solution for
 * our tests.
 *
 */
static int do_compression_action(const struct field_description *rule_row,
    const struct field_values *ipv6_packet, uint8_t schc_packet[SIZE_MTU_IPV6], size_t *offset, int *current_coap_option_num)
{
	if (rule_row == NULL || ipv6_packet == NULL) {
		return -1;
	}

	if (rule_row->CDA == COMPUTE_LENGTH || rule_row->CDA == NOT_SENT ||
	    rule_row->CDA == COMPUTE_CHECKSUM) {

      if (rule_row->fieldid == COAP_OPTION_VALUE) {
        /*
         * Even if we are not incluying anything on the ressidue for This CoAP
         * Option, we must increment the current_coap_option_num value
         * Because we assume that the uncompressed version of the CoAP
         * header has a CoAP Option here.
         */
        (*current_coap_option_num)++;
      }

		return 0;
	}

	size_t n = 0;
	uint32_t udp_flow_label;
	uint16_t ipv6_payload_length;
	uint16_t udp_dev_port;

	uint16_t udp_app_port;
	uint16_t udp_length;
	uint16_t udp_checksum;

   PRINT("do_compression_action, field_id: "); PRINTLN(rule_row->fieldid);

	if (rule_row->CDA == VALUE_SENT) {

		switch (rule_row->fieldid) {
			case IPV6_VERSION:
				n = 1;
				memcpy(&schc_packet[*offset], &ipv6_packet->ipv6_version, n);
				*offset += n;
				return 0;
			case IPV6_TRAFFIC_CLASS:
				n = 1;
				memcpy(&schc_packet[*offset], &ipv6_packet->ipv6_traffic_class, n);
				*offset += n;
				return 0;
			case IPV6_FLOW_LABEL:
				n = 3;
				udp_flow_label = htonl(ipv6_packet->ipv6_flow_label);
				udp_flow_label = udp_flow_label << 8;
				memcpy(&schc_packet[*offset], &udp_flow_label, n);
				*offset += n;
				return 0;
			case IPV6_PAYLOAD_LENGTH:
				n = 2;
				ipv6_payload_length = htons(ipv6_packet->ipv6_payload_length);
				memcpy(&schc_packet[*offset], &ipv6_payload_length, n);
				*offset += n;
				return 0;
			case IPV6_NEXT_HEADER:
				n = 1;
				memcpy(&schc_packet[*offset], &ipv6_packet->ipv6_next_header, n);
				*offset += n;
				return 0;
			case IPV6_HOP_LIMIT:
				n = 1;
				memcpy(&schc_packet[*offset], &ipv6_packet->ipv6_hop_limit, n);
				*offset += n;
				return 0;
			case IPV6_DEV_PREFIX:
				n = 8;
				memcpy(&schc_packet[*offset], &ipv6_packet->ipv6_dev_prefix, n);
				*offset += n;
				return 0;
			case IPV6_DEVIID:
				n = 8;
				memcpy(&schc_packet[*offset], &ipv6_packet->ipv6_dev_iid, n);
				*offset += n;
				return 0;
			case IPV6_APP_PREFIX:
				n = 8;
				memcpy(&schc_packet[*offset], &ipv6_packet->ipv6_app_prefix, n);
				*offset += n;
				return 0;
			case IPV6_APPIID:
				n = 8;
				memcpy(&schc_packet[*offset], &ipv6_packet->ipv6_app_iid, n);
				*offset += n;
				return 0;
			case UDP_DEVPORT:
				n = 2;
				udp_dev_port = htons(ipv6_packet->udp_dev_port);
				memcpy(&schc_packet[*offset], &udp_dev_port, n);
				*offset += n;
				return 0;
			case UDP_APPPORT:
				n = 2;
				udp_app_port = htons(ipv6_packet->udp_app_port);
				memcpy(&schc_packet[*offset], &udp_app_port, n);
				*offset += n;
				return 0;
			case UDP_LENGTH:
				n = 2;
				udp_length = htons(ipv6_packet->udp_length);
				memcpy(&schc_packet[*offset], &udp_length, n);
				*offset += n;
				return 0;
			case UDP_CHECKSUM:
				n = 2;
				udp_checksum = htons(ipv6_packet->udp_checksum);
				memcpy(&schc_packet[*offset], &udp_checksum, n);
				*offset += n;
				return 0;
			// CoAP
			case COAP_VERSION:
				PRINTLN("CA - CoAP - version");
				n = 1;
				memcpy(&schc_packet[*offset], &ipv6_packet->coap_version, n);
				*offset += n;
				return 0;
			case COAP_TYPE:
				PRINTLN("CA - CoAP - type");
				n = 1;
				memcpy(&schc_packet[*offset], &ipv6_packet->coap_type, n);
				*offset += n;
				return 0;
			case COAP_TKL:
				PRINTLN("CA - CoAP - tkl");
				n = 1;
				memcpy(&schc_packet[*offset], &ipv6_packet->coap_tkl, n);
				*offset += n;
				return 0;
			case COAP_CODE:
				PRINTLN("CA - CoAP - code");
				n = 1;
				memcpy(&schc_packet[*offset], &ipv6_packet->coap_code, n);
				*offset += n;
				return 0;
			case COAP_MESSAGE_ID:
				PRINTLN("CA - CoAP - msgid");
				n = 2;
				memcpy(&schc_packet[*offset], &ipv6_packet->coap_message_id, n);
				*offset += n;
				return 0;
			case COAP_TOKEN:
				PRINTLN("CA - CoAP - token");
				n = ipv6_packet->coap_tkl;
				memcpy(&schc_packet[*offset], &ipv6_packet->coap_token, n);
				*offset += n;
				return 0;
			case COAP_OPTION_DELTA:
				PRINTLN("CA - CoAP - delta");
				n = 1;
				memcpy(&schc_packet[*offset], &ipv6_packet->coap_option[*current_coap_option_num].delta, n);
				*offset += n;
				return 0;
			case COAP_OPTION_LENGTH:
				PRINTLN("CA - CoAP - length");
				n = 1;
				memcpy(&schc_packet[*offset], &ipv6_packet->coap_option[*current_coap_option_num].length, n);
				*offset += n;
				return 0;
			case COAP_OPTION_VALUE:
				PRINTLN("CA - CoAP - value");
				n = ipv6_packet->coap_option[*current_coap_option_num].length;
				memcpy(&schc_packet[*offset], &ipv6_packet->coap_option[*current_coap_option_num].value, n);
				*offset += n;
            (*current_coap_option_num) ++;
				return 0;
			default:
				break;
		}
	}

	return -1;
}



/**
 * \brief Checks the corresponding fv value on the fd->fieldid, using
 * the fd->MO against fd->tv.
 *
 * @param [in] rule_row The rule row in the context to check the MO and
 * the TV. Must be non-zero, or returns 0.
 *
 * @param [in] ipv6_packet The received IPv6 packet to check for matching. Must
 * be non-zero, or returns 0.
 *
 * @param [in, out] current_coap_option_num We manage a variable number
 * of CoAP Options in our structure, but this function has no way of
 * knowing what CoAP option it should be checking. In order to give it
 * the required index to read the correct CoAP Option values, we use
 * this parameter.
 *
 * @return 1 if the field matches with the MO. 0 if the field does not
 * match.
 *
 * The idea of this function is to be called in a loop from
 * schc_compress() for any rows in the SCHC rule and check individually
 * if it's a match or not.
 *
 * I don't really like the way it works, the switch (fd->fieldid) is a
 * little dirty, but right now it does the job.
 *
 * \note Does not distinguis between an error an a non-match. In both
 * cases, 0 is returned.
 *
 * TODO It must distinguis between an error an a non-match.
 */
static int check_matching(struct field_description rule_row, struct field_values ipv6_packet, int *current_coap_option_num)
{


	if (rule_row.MO == IGNORE) {
		PRINTLN("check_matching ignored");

      if (rule_row.fieldid ==  COAP_OPTION_VALUE) { // We need to keep track how many CoAP Option we pass
        (*current_coap_option_num)++;
      }

		return 1;
	}
 
  if (rule_row.MO == EQUALS) {
    PRINTLN("check_matching entering");

    PRINT("fieldid: ");
    PRINTLN(rule_row.fieldid);

	 uint8_t tv[8] = {0};
    switch (rule_row.fieldid) {

      case IPV6_VERSION:
        return (atoi(rule_row.tv) == ipv6_packet.ipv6_version);
      case IPV6_TRAFFIC_CLASS:
        return (atoi(rule_row.tv) == ipv6_packet.ipv6_traffic_class);
      case IPV6_FLOW_LABEL:
        return (atoi(rule_row.tv) == ipv6_packet.ipv6_flow_label);
      case IPV6_PAYLOAD_LENGTH:
        return (atoi(rule_row.tv) == ipv6_packet.ipv6_payload_length);
      case IPV6_NEXT_HEADER:
        return (atoi(rule_row.tv) == ipv6_packet.ipv6_next_header);
      case IPV6_HOP_LIMIT:
        return (atoi(rule_row.tv) == ipv6_packet.ipv6_hop_limit);
      case IPV6_DEV_PREFIX:
        string_to_bin(tv, rule_row.tv);
        // If the field is not equal, we must return error.
        if (memcmp((void*)&ipv6_packet.ipv6_dev_prefix, (void*)tv, 8) != 0) {
          return 0;
        }
        return 1;
      case IPV6_DEVIID:
        string_to_bin(tv, rule_row.tv);
        // If the field is not equal, we must return error.
        if (memcmp(ipv6_packet.ipv6_dev_iid, tv, 8) != 0) {
          return 0;
        }
        return 1;
      case IPV6_APP_PREFIX:
        string_to_bin(tv, rule_row.tv);
        // If the field is not equal, we must return error.
         
        if (memcmp(ipv6_packet.ipv6_app_prefix, tv, 8) != 0) {
          return 0;
        }
        return 1;
      case IPV6_APPIID:
        string_to_bin(tv, rule_row.tv);
        // If the field is not equal, we must return error.
        if (memcmp(ipv6_packet.ipv6_app_iid, tv, 8) != 0) {
          return 0;
        }
        return 1;
      case UDP_DEVPORT:
        return (atoi(rule_row.tv) == ipv6_packet.udp_dev_port);
      case UDP_APPPORT:
        return (atoi(rule_row.tv) == ipv6_packet.udp_app_port);
      case UDP_LENGTH:
        return (atoi(rule_row.tv) == ipv6_packet.udp_length);
      case UDP_CHECKSUM:
        return (atoi(rule_row.tv) == ipv6_packet.udp_checksum);
      // CoAP
      case COAP_VERSION:
        return (atoi(rule_row.tv) == ipv6_packet.coap_version);
      case COAP_TYPE:
        return (atoi(rule_row.tv) == ipv6_packet.coap_type);
      case COAP_TKL:
        return (atoi(rule_row.tv) == ipv6_packet.coap_tkl);
      case COAP_CODE:
        return (atoi(rule_row.tv) == ipv6_packet.coap_code);
      case COAP_MESSAGE_ID:
        string_to_bin(tv, rule_row.tv);
        
         // If the field is not equal, we must return error.
         
        if (memcmp(ipv6_packet.coap_message_id, tv, 2) != 0) {
          return 0;
        }
        return 1;
      case COAP_TOKEN:
				{
        string_to_bin(tv, rule_row.tv);

        uint8_t tkl = tkl;

        
         // If the field is not equal, we must return error.
         
        if (memcmp(ipv6_packet.coap_token, tv, tkl) != 0) {
          return 0;
        }
        return 1;
				}
      case COAP_OPTION_DELTA:
			return (atoi(rule_row.tv) == ipv6_packet.coap_option[*current_coap_option_num].delta);
      case COAP_OPTION_LENGTH:
			return (atoi(rule_row.tv) == ipv6_packet.coap_option[*current_coap_option_num].length);
      case COAP_OPTION_VALUE:
        string_to_bin(tv, rule_row.tv);

        
        // If the field is not equal, we must return error.
         
			if (memcmp(ipv6_packet.coap_option[*current_coap_option_num].value, rule_row.tv, ipv6_packet.coap_option[*current_coap_option_num].length) != 0) {
				PRINTLN("TV NOT EQUALS");
				(*current_coap_option_num) ++;
				return 0;
			}
			 (*current_coap_option_num) ++;
			 return 1;
		default:
			 break;
    }
  }

  PRINTLN("check_matching exit");

  return 0;
}


uint16_t compute_udp_checksum (struct field_values *fv) {

	struct ip6_hdr * ip6 = (struct ip6_hdr*)malloc(sizeof(struct ip6_hdr));

	// Combine version, traffic class and flow label into the flow field
	ip6->ip6_ctlun.ip6_un1.ip6_un1_flow = htonl((fv->ipv6_version << 7*4) + (fv->ipv6_traffic_class << 6*4) + fv->ipv6_flow_label);
	ip6->ip6_ctlun.ip6_un1.ip6_un1_plen = htons(fv->ipv6_payload_length);
	ip6->ip6_ctlun.ip6_un1.ip6_un1_nxt = fv->ipv6_next_header;
	ip6->ip6_ctlun.ip6_un1.ip6_un1_hlim = fv->ipv6_hop_limit;

	// Copy addresses
	for (int i = 0; i < 8; i++){
		ip6->ip6_src.s6_addr[i] = fv->ipv6_dev_prefix[i];
	}
	for (int i = 8; i < 16; i++){
		ip6->ip6_src.s6_addr[i] = fv->ipv6_dev_iid[i-8];
	}
	for (int i = 0; i < 8; i++){
		ip6->ip6_dst.s6_addr[i] = fv->ipv6_app_prefix[i];
	}
	for (int i = 8; i < 16; i++){
		ip6->ip6_dst.s6_addr[i] = fv->ipv6_app_iid[i-8];
	}

	// Prepare udp header
	struct udphdr * udp = (struct udphdr*)malloc(sizeof(struct udphdr));
	udp->uh_sport = htons(fv->udp_dev_port);
	udp->uh_dport = htons(fv->udp_app_port);
	udp->uh_ulen = htons(fv->udp_length);
	udp->uh_sum = 0;

	size_t payload_len = fv->udp_length - SIZE_UDP;
	// Calculate udp checksum
	// First, we prepare the buffer with the UDP/IPv6 packet
	uint8_t packet[SIZE_IPV6 + SIZE_UDP + payload_len];
	memcpy(packet, ip6, SIZE_IPV6);
	memcpy(packet + SIZE_IPV6, udp, SIZE_UDP);

	// Prepare manually the coap header
	uint8_t * coap_offset = packet + SIZE_IPV6 + SIZE_UDP;
	if (fv->coap_type == 0)
		memcpy(coap_offset, "\x42", 1);
	else if (fv->coap_type == 2)
		memcpy(coap_offset, "\x62", 1);
	coap_offset += 1;
	memcpy(coap_offset, &fv->coap_code, 1);
	coap_offset += 1;
	memcpy(coap_offset, &fv->coap_message_id, 2);
	coap_offset += 2;
	if (fv->coap_tkl > 0) {
		memcpy(coap_offset, fv->coap_token, fv->coap_tkl);
		coap_offset += fv->coap_tkl;
	}

   int i;

   for (i = 0 ; i < fv->coap_option_num ; i++) {
     if (fv->coap_option[i].length > 0) {
        uint8_t option = 0;
        option = option | fv->coap_option[i].length;
        option = option | (fv->coap_option[i].delta << 4);
        memcpy(coap_offset, &option, 1);
        coap_offset += 1;
        memcpy(coap_offset, &fv->coap_option[i].value, 1);
        coap_offset += 1;
     }

     if (i >= SCHC_MAX_COAP_OPT) {
       break;
     }
   }
	if (fv->coap_payload_length > 0) {
		memcpy(coap_offset, "\xFF", 1);
		coap_offset += 1;
		memcpy(coap_offset, fv->coap_payload, fv->coap_payload_length);
		coap_offset += fv->coap_payload_length;
	}


	uint16_t sum = (uint16_t) checksum_udp_ipv6(packet, SIZE_IPV6 + SIZE_UDP + 9 + fv->coap_payload_length);

	free(ip6);
	return sum;

}



void schc_DA_not_sent(struct field_values *fv, enum fieldid fieldid, char *tv)
{

	// Variables to convert hex string to byte array
	char tmp[8];
	uint8_t *pos = (uint8_t*)tv;

	switch(fieldid) {
		case IPV6_VERSION:
			fv->ipv6_version = atoi(tv);
			break;
		case IPV6_TRAFFIC_CLASS:
			fv->ipv6_traffic_class = atoi(tv);
			break;
		case IPV6_FLOW_LABEL:
			fv->ipv6_flow_label = atoi(tv);
			break;
		case IPV6_NEXT_HEADER:
			fv->ipv6_next_header = atoi(tv);
			break;
		case IPV6_HOP_LIMIT:
			fv->ipv6_hop_limit = atoi(tv);
			break;
		case IPV6_DEV_PREFIX:
			// Copy ipv6 address
			// for (int i = 0; i < 8; i++) {
			// 	sscanf((const char*)pos, "%2hhx", &tmp[i]);
			// 	pos+=2;
			// }	
			// memcpy(fv->ipv6_dev_prefix, tmp, 8);


         string_to_bin(fv->ipv6_dev_prefix, tv);
			break;
		case IPV6_DEVIID:
			// Copy ipv6 address
			// for (int i = 0; i < 8; i++) {
			// 	sscanf((const char*)pos, "%2hhx", &tmp[i]);
			// 	pos+=2;
			// }
			// memcpy(fv->ipv6_dev_iid, tmp, 8);

         string_to_bin(fv->ipv6_dev_iid, tv);
			break;
		case IPV6_APP_PREFIX:
			// Copy ipv6 address
			// for (int i = 0; i < 8; i++) {
			// 	sscanf((const char*)pos, "%2hhx", &tmp[i]);
			// 	pos+=2;
			// }
			// memcpy(fv->ipv6_app_prefix, tmp, 8);

         string_to_bin(fv->ipv6_app_prefix, tv);
			break;
		case IPV6_APPIID:
			// Copy ipv6 address
			// for (int i = 0; i < 8; i++) {
			// 	sscanf((const char*)pos, "%2hhx", &tmp[i]);
			// 	pos+=2;
			// }
			// memcpy(fv->ipv6_app_iid, tmp, 8);

         string_to_bin(fv->ipv6_app_iid, tv);
			break;
		case UDP_DEVPORT:
			fv->udp_dev_port = atoi(tv); 
			break;
		case UDP_APPPORT:
			fv->udp_app_port = atoi(tv); 
			break;
		//CoAP
		case COAP_VERSION:
			fv->coap_version = atoi(tv);
			break;
		case COAP_TYPE:
			fv->coap_type = atoi(tv);
			break;
		case COAP_TKL:
			fv->coap_tkl = atoi(tv);
			break;
		case COAP_CODE:
			fv->coap_code = atoi(tv);
			break;
		case COAP_MESSAGE_ID:
			for (int i = 0; i < 2; i++) {
				sscanf((const char*)pos, "%2hhx", &tmp[i]);
				pos+=2;
			}
			memcpy(fv->coap_message_id, tmp, 2);
			break;
		case COAP_TOKEN:
			for (int i = 0; i < 2; i++) {
				sscanf((const char*)pos, "%2hhx", &tmp[i]);
				pos+=2;
			}
			memcpy(fv->coap_token, tmp, 2);
			break;
		case COAP_OPTION_DELTA:
			fv->coap_option[fv->coap_option_num].delta = atoi(tv);
			break;
		case COAP_OPTION_LENGTH:
			fv->coap_option[fv->coap_option_num].length = atoi(tv);
			break;
		case COAP_OPTION_VALUE:
			/*for (int i = 0; i < fv->coap_option_length; i++) {
				sscanf(pos, "%2hhx", &tmp[i]);
				pos+=2;
			}*/
			memcpy(fv->coap_option[fv->coap_option_num].value, tv, fv->coap_option[fv->coap_option_num].length);
         fv->coap_option_num ++;
			break;		
		default:
			break;
	}

}

void schc_DA_value_sent(struct field_values *fv, enum fieldid fieldid, uint8_t *schc_packet, uint8_t *offset)
{

	switch(fieldid) {
		case IPV6_VERSION:
			memcpy(&fv->ipv6_version, &schc_packet[*offset], 1);
			*offset+=1;
			break;
		case IPV6_TRAFFIC_CLASS:
			memcpy(&fv->ipv6_traffic_class, &schc_packet[*offset], 1);
			*offset+=1;
			break;
		case IPV6_FLOW_LABEL:
			memcpy(&fv->ipv6_flow_label, &schc_packet[*offset], 3);
			fv->ipv6_flow_label = fv->ipv6_flow_label << 8;
			fv->ipv6_flow_label = ntohl(fv->ipv6_flow_label);
			*offset+=3;
			break;
		case IPV6_PAYLOAD_LENGTH:
			memcpy(&fv->ipv6_payload_length, &schc_packet[*offset], 2);
			fv->ipv6_payload_length = ntohs(fv->ipv6_payload_length);
			*offset+=2;
			break;
		case IPV6_NEXT_HEADER:
			memcpy(&fv->ipv6_next_header, &schc_packet[*offset], 1);
			*offset+=1;
			break;
		case IPV6_HOP_LIMIT:
			memcpy(&fv->ipv6_hop_limit, &schc_packet[*offset], 1);
			*offset+=1;
			break;
		case IPV6_DEV_PREFIX:
			memcpy(fv->ipv6_dev_prefix, &schc_packet[*offset], 8);
			*offset+=8;
			break;
		case IPV6_DEVIID:
			memcpy(fv->ipv6_dev_iid, &schc_packet[*offset], 8);
			*offset+=8;
			break;
		case IPV6_APP_PREFIX:
			memcpy(fv->ipv6_app_prefix, &schc_packet[*offset], 8);
			*offset+=8;
			break;
		case IPV6_APPIID:
			memcpy(fv->ipv6_app_iid, &schc_packet[*offset], 8);
			*offset+=8;
			break;
		case UDP_DEVPORT:
			memcpy(&fv->udp_dev_port, &schc_packet[*offset], 2);
			fv->udp_dev_port = ntohs(fv->udp_dev_port);
			*offset+=2;
			break;
		case UDP_APPPORT:
			memcpy(&fv->udp_app_port, &schc_packet[*offset], 2);
			fv->udp_app_port = ntohs(fv->udp_app_port);
			*offset+=2;
			break;
		case UDP_LENGTH:
			memcpy(&fv->udp_length, &schc_packet[*offset], 2);
			fv->udp_length = ntohs(fv->udp_length);
			*offset+=2;
			break;
		case UDP_CHECKSUM:
			memcpy(&fv->udp_checksum, &schc_packet[*offset], 2);
			fv->udp_checksum = ntohs(fv->udp_checksum);
			*offset+=2;
			break;
		// CoAP
		case COAP_VERSION:
			memcpy(&fv->coap_version, &schc_packet[*offset], 1);
			*offset+=1;
			break;
		case COAP_TYPE:
			memcpy(&fv->coap_type, &schc_packet[*offset], 1);
			*offset+=1;
			break;
		case COAP_TKL:
			memcpy(&fv->coap_tkl, &schc_packet[*offset], 1);
			*offset+=1;
			break;
		case COAP_CODE:
			memcpy(&fv->coap_code, &schc_packet[*offset], 1);
			*offset+=1;
			break;
		case COAP_MESSAGE_ID:
			memcpy(&fv->coap_message_id, &schc_packet[*offset], 2);
			*offset+=2;
			break;
		case COAP_TOKEN:
			memcpy(&fv->coap_token, &schc_packet[*offset], fv->coap_tkl);
			*offset+=fv->coap_tkl;
			break;
		case COAP_OPTION_DELTA:
			memcpy(&fv->coap_option[fv->coap_option_num].delta, &schc_packet[*offset], 1);
			*offset+=1;
			break;
		case COAP_OPTION_LENGTH:
			memcpy(&fv->coap_option[fv->coap_option_num].length, &schc_packet[*offset], 1);
			*offset+=1;
			break;
		case COAP_OPTION_VALUE:
			memcpy(&fv->coap_option[fv->coap_option_num].value, &schc_packet[*offset], fv->coap_option[fv->coap_option_num].length);
			*offset+=fv->coap_option[fv->coap_option_num].length;
         fv->coap_option_num ++;
			break;
		default:
			break;
	}

}

/*
 * TODO
 */
static int schc_decompress(uint8_t *schc_packet, size_t schc_packet_len)
{

	PRINT("schc_decompress -> decompressing\n");
	PRINT("schc_packet: ");
	PRINT_ARRAY(schc_packet, schc_packet_len);



	// TODO if malloc() returns == 0 , print error and/or exit the execution of the project.
	struct field_values fv_local;
	struct field_values *fv = &fv_local;

	memset(fv, 0, sizeof(*fv));


	// Extract rule_id used for compression
	uint8_t rule_id = (uint8_t) schc_packet[0];

	/*
	* If the rule does not exists in context.c, we silently drop the
	* packet. {
	*/
	int nrules = sizeof(rules) / sizeof(rules[0]);
	int field_description_size = sizeof(rules[0])/sizeof(struct field_description);

	if (rule_id >= nrules) {
		return -1;
   }

	/*
	* } If the rule does not exists in context.c, we silently drop the
	* packet.
	*/

	PRINT("schc_decompress -> using rule_id: ");
	PRINTLN(rule_id);
	
	// Offset to go through the compression residue
	uint8_t offset = 1;

	// Iterate over the rule
	for (int i = 0; i < field_description_size; i++){


		// Extract rule data
		enum fieldid fieldid = rules[rule_id][i].fieldid;
		size_t field_length = rules[rule_id][i].field_length;
		int field_position = rules[rule_id][i].field_position;
		enum direction direction = rules[rule_id][i].direction;
		enum MO MO = rules[rule_id][i].MO;
		enum CDA CDA = rules[rule_id][i].CDA;

      /*
       * If we find the END_OF_RULE mark, we break the loop here.
       */
      if (fieldid == END_OF_RULE) {
        break;
      }

		// Check action
		switch (CDA) {
			case NOT_SENT:
				schc_DA_not_sent(fv, fieldid, rules[rule_id][i].tv);
				break;
			case VALUE_SENT:
				schc_DA_value_sent(fv, fieldid, schc_packet, &offset);
				break;
			case MAPPING_SENT:
				break;
			case LSB:
				break;
			// If length or checksum is needed to be computed, we must do it at the end, as we
			// need all the field values
			case COMPUTE_LENGTH:
				compute_length = 1;
				break;
			case COMPUTE_CHECKSUM:
				compute_checksum = 1;
				break;
			case DEVIID:
				break;
			case APPIID:
				break;
			default:
				break;
		}
	}



	size_t coap_payload_length = 0;
	size_t udp_payload_length = 0;

	// Now we can compute the length of the payload
	if (compute_length)	{
		// The length is the schc packet length minus the rule id and the compression residue
		coap_payload_length = schc_packet_len - offset;

      /*
       * Compute the coap header length {
       */
      size_t coap_header_len  = 0;

      coap_header_len += 4; // Fixed CoAP Length size (Ver, T, TLK, Code, MessageID
      coap_header_len += fv->coap_tkl; // Token Length

      int i;
      uint16_t last_delta = 0;
      for (i = 0; i < fv->coap_option_num ; i++) {
        // Refer to RFC7252 - Section 3.1 for this block of code

        coap_header_len += 1; // The Initial byte of each CoAP Option (4 bits for CoAP Delta Option and 4 bits for CoAP length]

        if (fv->coap_option[i].delta - last_delta <= 12) {
          // No extended CoAp Option Delta bytes -> Do nothing here
        } else if (fv->coap_option[i].delta - last_delta <= 255) {
          coap_header_len += 1; // The CoAP Option delta extended byte
        } else  {
          coap_header_len += 2; // The CoAP Option delta extended 2-bytes
        }

        if (fv->coap_option[i].length  <= 12) {
          // No extended CoAp Option Length bytes -> Do nothing here
        } else if (fv->coap_option[i].length  <= 255) {
          coap_header_len += 1; // The CoAP Option Length extended byte
        } else  {
          coap_header_len += 2; // The CoAP Option Length extended 2-bytes
        }


        coap_header_len += fv->coap_option[i].length; // The length of the CoAP Option value bytes

        last_delta = fv->coap_option[i].delta;
      }

      if (coap_payload_length > 0) {
        coap_header_len += 1; // 0xFF mark at the end of CoAP Options and beginning of CoAP Payload
      }
      /*
       * }
       */

		fv->coap_payload_length = coap_payload_length;

		udp_payload_length = coap_header_len + coap_payload_length;
		fv->ipv6_payload_length = udp_payload_length + SIZE_UDP;
		fv->udp_length = udp_payload_length  + SIZE_UDP;

		compute_length = 0;
	}

	// Add payload to the struct
	if (coap_payload_length > 0) {
		memcpy(fv->coap_payload, &schc_packet[offset], coap_payload_length);
	}

	// Now that we have the payload, we can compute the udp checksum
	if (compute_checksum) {
		uint16_t sum = compute_udp_checksum(fv);
		fv->udp_checksum = sum;

		compute_checksum = 0;
	}

	// Print the reconstructed packet

	// TODO print_udpIp6_packet(fv) assumes always downlink packets
	// (when dev_iid is the ipv6_dst_addr), if we use it to print here,
	// the values will be scrambled.

	// print_udpIp6_packet(fv);
	
	fv->ipv6_hop_limit = 255;

	PRINTLN("schc_decompress calling send_fv_outbound");

	ipv6_process_rx(fv);

	return 0;

}

/**********************************************************************/
/***        Public Functions                                        ***/
/**********************************************************************/

int string_to_bin(uint8_t dst[8], const char *src)
{

  /* WARNING: no sanitization or error-checking whatsoever */
	 int i = 0;
	 uint8_t tmp = 0;
	 for (i = 0 ; i < 8 ; i++ ) {
			switch (src[0]) {
				 case '0': tmp = 0x0; break;
				 case '1': tmp = 0x1; break;
				 case '2': tmp = 0x2; break;
				 case '3': tmp = 0x3; break;
				 case '4': tmp = 0x4; break;
				 case '5': tmp = 0x5; break;
				 case '6': tmp = 0x6; break;
				 case '7': tmp = 0x7; break;
				 case '8': tmp = 0x8; break;
				 case 'A':case 'a': tmp = 0xa; break;
				 case 'B':case 'b': tmp = 0xb; break;
				 case 'C':case 'c': tmp = 0xc; break;
				 case 'D':case 'd': tmp = 0xd; break;
				 case 'E':case 'e': tmp = 0xe; break;
				 case 'F':case 'f': tmp = 0xf; break;

			}
			tmp = tmp << 4;

			switch (src[1]) {
				 case '0': tmp |= 0x0; break;
				 case '1': tmp |= 0x1; break;
				 case '2': tmp |= 0x2; break;
				 case '3': tmp |= 0x3; break;
				 case '4': tmp |= 0x4; break;
				 case '5': tmp |= 0x5; break;
				 case '6': tmp |= 0x6; break;
				 case '7': tmp |= 0x7; break;
				 case '8': tmp |= 0x8; break;
				 case 'A':case 'a': tmp |= 0xa; break;
				 case 'B':case 'b': tmp |= 0xb; break;
				 case 'C':case 'c': tmp |= 0xc; break;
				 case 'D':case 'd': tmp |= 0xd; break;
				 case 'E':case 'e': tmp |= 0xe; break;
				 case 'F':case 'f': tmp |= 0xf; break;
			}

			dst[i] = tmp;

			src += 2;
	 }


  return 0;
}

int schc_compress(const struct field_values ipv6_packet)
{

	PRINTLN("schc_compress entering");

  uint8_t schc_packet[SIZE_MTU_IPV6] = {0};
  size_t  schc_packet_len = 0;

  /*
   * Pointer to the current location of the schc_packet.
   */
  uint8_t offset = 0;

  /*
   * We go through all the rules.
   */

  int nrules = sizeof(rules) / sizeof(rules[0]);
  int field_description_size = sizeof(rules[0])/sizeof(struct field_description); 

  for (int i = 0 ; i < nrules ; i++ ) {

    //struct field_description current_rule[22] = {0};
    struct field_description current_rule[field_description_size];
    memset(current_rule, 0, field_description_size * sizeof(struct field_description));
    memcpy(&current_rule, rules[i], sizeof(current_rule));

    /*
     * number of field_descriptors in the rule.
     */
    int rule_rows = sizeof(current_rule) / sizeof(current_rule[0]);

    int rule_matches = 1; /* Guard Condition for the next loop */


	 /* We keep track of how many CoAP Option of packet has */
	 int current_coap_option_num = 0;

    for (int j = 0 ; j < rule_rows ; j++) {

		 /*
		  * If we find the END_OF_RULE mark, we break the loop.
		  */
		 if (current_rule[j].fieldid == END_OF_RULE) {
			 break;
		 }
      
      /*
       * We check all fields to see if they match until. If some condition is not
       * met, we stop and return an error.
       */
      if (check_matching(current_rule[j], ipv6_packet, &current_coap_option_num) == 0) {
        rule_matches = 0;
        break;
      }

    }

	 if (current_coap_option_num != ipv6_packet.coap_option_num) {
		 /*
		  * We verify here if the number of CoAP Options contained within
		  * the SCHC Rule do not match the same number as the
		  * ipv6_packet. Hence, it actually does not match.
		  */
		 rule_matches = 0;
	 }

    if (rule_matches == 0) {
			PRINTLN("schc_compress - rule don't matched :(");
			schc_packet_len = 0;
			continue;
		}

    PRINTLN("schc_compress - rule matched!\n");
		/*
		 * At this point, all the MO of the rule returned success, we can
		 * start writing the schc_packet.
		 *
		 * First, we append the Rule ID to the schc_packet
		 */
		schc_packet[schc_packet_len] = i;
		schc_packet_len++;

		/*
		 * We do the compression actions.
		 */

      current_coap_option_num = 0;


    for (int j = 0 ; j < rule_rows ; j++) {

        /*
         * If we find the END_OF_RULE mark, we break the loop.
         */
			if (current_rule[j].fieldid == END_OF_RULE) {
				break;
			}

			/*
			 * We apply the compression action 
			 */
			if (do_compression_action(&current_rule[j], &ipv6_packet, schc_packet, &schc_packet_len, &current_coap_option_num) != 0) {
				return -1;
			}

	 }
	

  /*
    * At this point, Compression Ressidue is already in the packet.
    * We concatenate the app_payload to the packet.
    * Now, we have to add only de CoAP payload
    */

  size_t app_payload_len = ipv6_packet.coap_payload_length;

  uint8_t *p = schc_packet + schc_packet_len;

  memcpy(p, ipv6_packet.coap_payload, app_payload_len);

  schc_packet_len += app_payload_len;

	PRINTLN("schc_compression() result: ");
	PRINT_ARRAY(schc_packet, schc_packet_len);


		/*
		 * We have created the SCHC packet, we now go to the Fragmentation
		 * layer and the packet will be inserted in the downlink LPWAN queue by
		 * the schc_fragmentate() function as a whole SCHC packet, or as a 
		 * series of fragments.
		 *
		 * If schc_fragmentate succeeds, we return succeed. If it fails,
		 * we return fail.
		 */
	return schc_fragmentate(schc_packet, schc_packet_len);

  }

	/*
	 * No Rule in the context matched the ipv6_packet.
	 */
	return -1;
}


/**
 * TODO
 *
 * @return Zero in case of success. Non-zero in case of failure.
 */
int schc_reassemble(uint8_t *lorawan_payload, uint8_t lorawan_payload_len)
{

	PRINT("schc_reassemble: \n");
	PRINT("schc_reassemble lorawan_payload: ");
	PRINT_ARRAY(lorawan_payload, lorawan_payload_len);

	if (lorawan_payload[0] != SCHC_FRG_RULEID) {
		/*
		 * The received payload, is actually not a SCHC fragment, but a full
		 * SCHC packet.
		 */

		return schc_decompress(lorawan_payload, (size_t)lorawan_payload_len);

	}

	/*
	 * At this point, the received lorawan_payload IS an SCHC fragment.
	 * We start treating it accordingly.
	 */


	if (lorawan_payload[1] != 0xFF) {
		// The payload is not the last fragment of a packet

		memcpy(&schc_reassemble_buf[schc_reassemble_offset], &lorawan_payload[2], lorawan_payload_len-2);
		schc_reassemble_offset += lorawan_payload_len -2;

		PRINT("schc_reassemble_buf len: ");
		PRINTLN( schc_reassemble_offset);


	} else {
		// The payload is the last fragment of a packet.

		memcpy(&schc_reassemble_buf[schc_reassemble_offset], &lorawan_payload[4], lorawan_payload_len-4);
		schc_reassemble_offset += lorawan_payload_len -4;

		// We check the CRC.

		uint16_t mic = crc16(schc_reassemble_buf, schc_reassemble_offset);

		PRINT("checksum calculado: ");
		PRINT_ARRAY(&mic, sizeof(mic));

		mic = ntohs(mic);

		PRINT_ARRAY(&mic, sizeof(mic));


		if (memcmp(&lorawan_payload[2], &mic, sizeof(mic)) == 0) {
			// CRC OK
			PRINT("CRC OK\n");
			schc_decompress(schc_reassemble_buf, schc_reassemble_offset);
		}

		// We reset the counters
		schc_reassemble_offset = 0;


	}



	return 0;
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

