/*
 * sniffex.c
 *
 * Sniffer example of TCP/IP packet capture using libpcap.
 * 
 * Version 0.1.1 (2005-07-05)
 * Copyright (c) 2005 The Tcpdump Group
 *
 * This software is intended to be used as a practical example and 
 * demonstration of the libpcap library; available at:
 * http://www.tcpdump.org/
 *
 ****************************************************************************
 *
 * This software is a modification of Tim Carstens' "sniffer.c"
 * demonstration source code, released as follows:
 * 
 * sniffer.c
 * Copyright (c) 2002 Tim Carstens
 * 2002-01-07
 * Demonstration of using libpcap
 * timcarst -at- yahoo -dot- com
 * 
 * "sniffer.c" is distributed under these terms:
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. The name "Tim Carstens" may not be used to endorse or promote
 *    products derived from this software without prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * <end of "sniffer.c" terms>
 *
 * This software, "sniffex.c", is a derivative work of "sniffer.c" and is
 * covered by the following terms:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Because this is a derivative work, you must comply with the "sniffer.c"
 *    terms reproduced above.
 * 2. Redistributions of source code must retain the Tcpdump Group copyright
 *    notice at the top of this source file, this list of conditions and the
 *    following disclaimer.
 * 3. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. The names "tcpdump" or "libpcap" may not be used to endorse or promote
 *    products derived from this software without prior written permission.
 *
 * THERE IS ABSOLUTELY NO WARRANTY FOR THIS PROGRAM.
 * BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY
 * FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN
 * OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES
 * PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED
 * OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS
 * TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE
 * PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,
 * REPAIR OR CORRECTION.
 * 
 * IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING
 * WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR
 * REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,
 * INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING
 * OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED
 * TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY
 * YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER
 * PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 * <end of "sniffex.c" terms>
 *
 ****************************************************************************
 *
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

/**********************************************************************/
/***        Global Includes       E.g. <foo.h>                      ***/
/**********************************************************************/

#include <Arduino.h> // Used only for Serial. for Debug PRINTs

/**********************************************************************/
/***        Local Includes        E.g. "bar.h"                      ***/
/**********************************************************************/

#include "schc.h"
#include "utils.h"
#include "ipv6_layer.h"

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

/**********************************************************************/
/***        Forward Declarations                                    ***/
/**********************************************************************/

/**********************************************************************/
/***        Constants                                               ***/
/**********************************************************************/

/**********************************************************************/
/***        Global Variables                                        ***/
/**********************************************************************/

// We store here the received ipv6 packet
uint8_t  ipv6_rx_buff[SIZE_MTU_IPV6] = {0};
uint16_t ipv6_rx_buff_len            =  0 ;
int      ipv6_rx_packet_ready        =  0 ; // 0 == no packet received, 1 == packet received

/**********************************************************************/
/***        Static Variables                                        ***/
/**********************************************************************/

/**********************************************************************/
/***        Static Functions                                        ***/
/**********************************************************************/

/**********************************************************************/
/***        Global Functions                                        ***/
/**********************************************************************/


// TODO: Imprime paquetes downlink
void print_udpIp6_packet(const struct field_values *udpIp6_packet) {


	PRINTLN("UDP/IPv6--------------------------------------------------------");

	PRINT("	ipv6_version: 0x");       PRINTLN(udpIp6_packet->ipv6_version, HEX);
	PRINT("	ipv6_traffic_class: 0x"); PRINTLN(udpIp6_packet->ipv6_traffic_class, HEX);
	PRINT("	ipv6_flow_label: 0x");    PRINTLN(udpIp6_packet->ipv6_flow_label, HEX);
	PRINT("	ipv6_payload_length: 0x");PRINTLN(udpIp6_packet->ipv6_payload_length, HEX);
	PRINT("	ipv6_next_header: 0x");   PRINTLN(udpIp6_packet->ipv6_next_header, HEX);
	PRINT("	ipv6_hop_limit: 0x"); PRINTLN(udpIp6_packet->ipv6_hop_limit, HEX);
	PRINTLN("	ipv6_app: ");
   PRINT_ARRAY(udpIp6_packet->ipv6_app_prefix, 8);
   PRINT_ARRAY(udpIp6_packet->ipv6_app_iid, 8);
	PRINTLN("	ipv6_dev: ");
   PRINT_ARRAY(udpIp6_packet->ipv6_dev_prefix, 8);
   PRINT_ARRAY(udpIp6_packet->ipv6_dev_iid, 8);

	PRINT("	udp_appport: 0x"); PRINTLN(udpIp6_packet->udp_app_port, HEX);
	PRINT("	udp_devport: 0x"); PRINTLN(udpIp6_packet->udp_dev_port, HEX);
	PRINT("	udp_length: 0x"); PRINTLN(udpIp6_packet->udp_length, HEX);
	PRINT("	udp_checksum: 0x");PRINTLN(udpIp6_packet->udp_checksum, HEX);


	PRINT("	coap_version: 0x"); PRINTLN( udpIp6_packet->coap_version, HEX);
	PRINT("	coap_type: 0x"); PRINTLN( udpIp6_packet->coap_type, HEX);
	PRINT("	coap_tkl: 0x"); PRINTLN( udpIp6_packet->coap_tkl, HEX);
	PRINT("	coap_code: 0x"); PRINTLN( udpIp6_packet->coap_code, HEX);
	PRINT("	coap_message_id: ");
	PRINT_ARRAY(udpIp6_packet->coap_message_id, 2);	
	PRINT("	coap_token: ");
	PRINT_ARRAY(udpIp6_packet->coap_token, udpIp6_packet->coap_tkl);

	PRINT("	coap_option_num: "); PRINTLN(udpIp6_packet->coap_option_num);
   int i;
   for (i = 0 ; i < udpIp6_packet->coap_option_num; i++) {
     PRINT("	coap_option: delta "); PRINTLN(udpIp6_packet->coap_option[i].delta);
     PRINT("	coap_option: length "); PRINTLN(udpIp6_packet->coap_option[i].length);
     PRINT("	coap_option: value ");

     PRINT_ARRAY(udpIp6_packet->coap_option[i].value, udpIp6_packet->coap_option[i].length);

     if (i >= SCHC_MAX_COAP_OPT) {
       break;
     }
   }

   if (udpIp6_packet->coap_payload_length > 0) {
     PRINT("	coap_payload: "); PRINTLN( udpIp6_packet->coap_payload_length);
     PRINT_ARRAY(udpIp6_packet->coap_payload, udpIp6_packet->coap_payload_length);
   }

	PRINTLN("----------------------------------------------------------------");

}

/*
 * We prepare a buffer to be passed to crc16(). The buffer must include the IPv6/UDP
 * Pseudo Header and also the UDP header + UDP payload.
 *
 * The procedure is explained in:
 *
 * https://stackoverflow.com/questions/30858973/udp-checksum-calculation-for-ipv6-packet
 */
uint16_t checksum_udp_ipv6(uint8_t *buffer, int length_buffer) {

	PRINTLN("checksum_udp_ipv6 -> preparing pseudo-header");

	uint8_t aux[SIZE_MTU_IPV6] = {0};
	size_t aux_offset = 0;
	int i = 0;

    /*
     * Pseudo Header.
     */
    {

		 /* IPv6 Addresses */
		 for (i = 0 ; i < 32 ; i++ ) {
			aux[aux_offset] = buffer[8+i];
			aux_offset += 1;

		 }

        /* protocol (UDP 0x11) */
		aux[aux_offset] = 0x0;
		aux[aux_offset+1] = 0x11;
		aux_offset += 2;

        /* payload length (UDP Header + Payload ) */
		aux[aux_offset] = buffer[4];
		aux[aux_offset+1] = buffer[5];
		aux_offset += 2;

    } /* Pseudo header end */

	uint16_t udp_payload_length = length_buffer - 40;
	for (i = 0; i < udp_payload_length ; i++) {
		aux[aux_offset] = buffer[40+i];
		aux_offset += 1;
	}

	PRINTLN("checksum_udp_ipv6 -> pseudo-header ready");

	return crc16(aux, aux_offset);

}

void create_udpIp6_packet(uint8_t * packet, const struct field_values *fv) {

	PRINTLN("create_udpIp6_packet -> preparing data");

	// Prepare ipv6 header
	struct ip6_hdr * ip6 = (struct ip6_hdr*) malloc(sizeof(struct ip6_hdr));
	// Combine version, traffic class and flow label into the flow field
	ip6->ip6_ctlun.ip6_un1.ip6_un1_flow = htonl((fv->ipv6_version << 7*4) + (fv->ipv6_traffic_class << 6*4) + fv->ipv6_flow_label);
	ip6->ip6_ctlun.ip6_un1.ip6_un1_plen = htons(fv->ipv6_payload_length);
	ip6->ip6_ctlun.ip6_un1.ip6_un1_nxt = fv->ipv6_next_header;
	ip6->ip6_ctlun.ip6_un1.ip6_un1_hlim = fv->ipv6_hop_limit;
	// Copy addresses
	for (int i = 0; i < 8; i++){
		ip6->ip6_src.s6_addr[i] = fv->ipv6_app_prefix[i];
	}
	for (int i = 8; i < 16; i++){
		ip6->ip6_src.s6_addr[i] = fv->ipv6_app_iid[i-8];
	}
	for (int i = 0; i < 8; i++){
		ip6->ip6_dst.s6_addr[i] = fv->ipv6_dev_prefix[i];
	}
	for (int i = 8; i < 16; i++){
		ip6->ip6_dst.s6_addr[i] = fv->ipv6_dev_iid[i-8];
	}

	PRINT_ARRAY(ip6, sizeof(*ip6));

	// Prepare udp header
	struct udphdr * udp = (struct udphdr*) malloc(sizeof(struct udphdr));
	udp->uh_sport = htons(fv->udp_app_port);
	udp->uh_dport = htons(fv->udp_dev_port);
	udp->uh_ulen = htons(fv->udp_length);
	udp->uh_sum = 0;

	PRINT_ARRAY(udp, sizeof(*udp));

	size_t payload_len = fv->udp_length - SIZE_UDP;
	// Calculate udp checksum
	// First, we prepare the buffer with the UDP/IPv6 packet
	memcpy(packet, ip6, SIZE_IPV6);
	memcpy(packet + SIZE_IPV6, udp, SIZE_UDP);
	//memcpy(packet + SIZE_IPV6 + SIZE_UDP, fv->payload, payload_len);

	uint8_t * coap_offset = packet + SIZE_IPV6 + SIZE_UDP;
   coap_offset[0] |= (0x1 << 6); //Coap Version == 1 -> binary [01.. ....]

	coap_offset[0] |= (fv->coap_type << 4); // Coap Type binary [..XX ....]

	coap_offset[0] |= (fv->coap_tkl & 0x0F);     // Coap TKL binary [.... YYYY]

	coap_offset += 1;

   // CoAP CODE
   coap_offset[0] = fv->coap_code;
	coap_offset += 1;

   // CoAP Message ID
	memcpy(coap_offset, &fv->coap_message_id, 2);
	coap_offset += 2;
	if (fv->coap_tkl > 0) {
		memcpy(coap_offset, fv->coap_token, fv->coap_tkl);
		coap_offset += fv->coap_tkl;
	}


   int i;

   uint16_t last_delta = 0;

   for (i = 0 ; i < fv->coap_option_num ; i++) {
     if (fv->coap_option[i].length > 0) {
        uint8_t initial_byte = 0;

        // This is only valid if the option number and the length are not more than 12 bytes,
        // in that case, we need to parse extended options as follows:
        // Ref. RFC7252 CoAP - Section 3.1
        //
        // 0   1   2   3   4   5   6   7
        //  +---------------+---------------+
        //  |               |               |
        //  |  Option Delta | Option Length |   1 byte (initial_byte)
        //  |               |               |
        //  +---------------+---------------+
        //  \                               \
        //  /         Option Delta          /   0-2 bytes
        //  \          (extended)           \
        //  +-------------------------------+
        //  \                               \
        //  /         Option Length         /   0-2 bytes
        //  \          (extended)           \
        //  +-------------------------------+
        //  \                               \
        //  /                               /
        //  \                               \
        //  /         Option Value          /   0 or more bytes
        //  \                               \
        //  /                               /
        //  \                               \
        //  +-------------------------------+


        int delta = fv->coap_option[i].delta - last_delta;

        last_delta = fv->coap_option[i].delta;


        if (delta <= 12) { // No Option Delta Extended
          initial_byte = initial_byte | (delta << 4);
        } else if (delta <= 255) { // Option Delta extended 1 byte
          initial_byte = initial_byte | (13 << 4);
        } else { // Option Delta extended 2 bytes
          initial_byte = initial_byte | (14 << 4);
        }

        if (fv->coap_option[i].length <= 12) { // No Option Delta Extended
          initial_byte = initial_byte | (fv->coap_option[i].length & 0x0F);
        } else if (fv->coap_option[i].length <= 255) { // Option Delta extended 1 byte
          initial_byte = initial_byte | (13 & 0x0F);
        } else { // Option Delta extended 2 bytes
          initial_byte = initial_byte | (14 & 0x0F);
        }


        memcpy(coap_offset, &initial_byte, 1);
        coap_offset += 1;

        if (delta <= 12) { // No Option Delta Extended
          // Do nothing, continue
        } else if (delta <= 255) { // Option Delta extended 1 byte
          *coap_offset = delta - 13;
          coap_offset ++;
        } else { // Option Delta extended 2 bytes
          *(uint16_t*)coap_offset = htons(delta - 269); // Ref RFC7252 Sec. 3.1
          coap_offset += 2;
        }


        if (fv->coap_option[i].length <= 12) { // No Option Delta Extended
          // Do nothing, continue
        } else if (fv->coap_option[i].length <= 255) { // Option Delta extended 1 byte
          *coap_offset = fv->coap_option[i].length - 13;
          coap_offset ++;
        } else { // Option Delta extended 2 bytes
          *(uint16_t*)coap_offset = htons(fv->coap_option[i].length - 269); // Ref RFC7252 Sec. 3.1
          coap_offset += 2;
        }



        memcpy(coap_offset, &fv->coap_option[i].value, fv->coap_option[i].length);
        coap_offset += fv->coap_option[i].length;
     }

     if (i >= SCHC_MAX_COAP_OPT) {
       break;
     }
   }
	PRINT("------- length: ");
	PRINTLN(fv->coap_payload_length);
	if (fv->coap_payload_length > 0) {
		memcpy(coap_offset, "\xFF", 1);
		coap_offset += 1;
		memcpy(coap_offset, fv->coap_payload, fv->coap_payload_length);
		coap_offset += fv->coap_payload_length;
	}	

	uint16_t sum = (uint16_t) checksum_udp_ipv6(packet, SIZE_IPV6 + SIZE_UDP + payload_len);

	udp->uh_sum = htons(sum);
	memcpy(packet + SIZE_IPV6, udp, SIZE_UDP);

	PRINTLN("create_udpIp6_packet -> packet created");

}


/*
 * lorawanmqtt module callback function.
 */
int ipv6_process_rx(struct field_values *fv)
{


  PRINTLN("ipv6_process_rx() received packet: ");
  print_udpIp6_packet(fv);

	// We create a well-formatted IPv6/UDP packet {

	uint8_t packet[SIZE_MTU_IPV6] = {0};

	// Encode the IPv6 packet
	create_udpIp6_packet(packet, fv);

	// }

	// We send the packet over to the Arduino main state machine for async processing.
	// {
	
	ipv6_rx_buff_len = fv->udp_length + SIZE_IPV6;
	memcpy(ipv6_rx_buff, packet, ipv6_rx_buff_len);

	// We raise the flag for a received IPv6 packet
	ipv6_rx_packet_ready    = 1;

	PRINTLN("send_fv_outbound IPv6 packet");
	PRINT_ARRAY(ipv6_rx_buff, ipv6_rx_buff_len);

	// }

	return 0;
}


/*
	Coap header:

	0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |Ver| T |  TKL  |      Code     |          Message ID           |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |   Token (if any, TKL bytes) ...
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |   Options (if any) ...
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |1 1 1 1 1 1 1 1|    Payload (if any) ...
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

*/

void read_coap_packet(struct field_values *fv, uint8_t * payload) {

	uint8_t * packet = payload;
	uint8_t packet_length = fv->udp_length - 8;

	uint8_t * offset = packet;
	uint16_t coap_length = 0;

	// CoAP header
	fv->coap_version = packet[0] >> 6;
	fv->coap_type = (packet[0] & 0x30) >> 4;
	fv->coap_tkl = packet[0] & 0x0F;
	fv->coap_code = packet[1];
	fv->coap_message_id[0] = packet[2];
	fv->coap_message_id[1] = packet[3];

	offset += 4;
	coap_length += 4;

	// If TKL is > 0 then there is a token
	if (fv->coap_tkl > 0) {
		memcpy(fv->coap_token, offset, fv->coap_tkl);
	}
	offset += fv->coap_tkl;
	coap_length += fv->coap_tkl;

	/*
		While there is more payload and it does not start by 0xFF, there are options

			0   1   2   3   4   5   6   7
		+---------------+---------------+
		|  Option Delta | Option Length |   1 byte
		+---------------+---------------+
	*/

   /* We initialize the value, at first we assume no CoAP Options */
   fv->coap_option_num = 0;

	uint16_t last_delta = 0;
	while (coap_length < packet_length && !(offset[0] == 0xff) && fv->coap_option_num < SCHC_MAX_COAP_OPT) {
		// Read options, if there is more than one, we have to compute deltas
		// FIXME: as a first approach to CoAP, we will only store a limited amount of CoAP Options
      // specified by SCHC_MAX_COAP_OPT.
		// If there are more options, the behaviour is undefined

     uint16_t delta = ((offset[0] & 0xF0) >> 4);
     uint16_t length = offset[0] & 0x0F;
     offset++;
     coap_length++;
		// This is only valid if the option number and the length are not more than 12 bytes,
		// in that case, we need to parse extended options as follows:
      // Ref. RFC7252 CoAP - Section 3.1
      //
      // 0   1   2   3   4   5   6   7
      //  +---------------+---------------+
      //  |               |               |
      //  |  Option Delta | Option Length |   1 byte
      //  |               |               |
      //  +---------------+---------------+
      //  \                               \
      //  /         Option Delta          /   0-2 bytes
      //  \          (extended)           \
      //  +-------------------------------+
      //  \                               \
      //  /         Option Length         /   0-2 bytes
      //  \          (extended)           \
      //  +-------------------------------+
      //  \                               \
      //  /                               /
      //  \                               \
      //  /         Option Value          /   0 or more bytes
      //  \                               \
      //  /                               /
      //  \                               \
      //  +-------------------------------+

      if (delta == 13) { /* an 8-bit unsigned integer follows the initial byte */
        delta += offset[0];
        offset ++;
        coap_length ++;
      } else if (delta == 14) { /* A 16-bit unsigned integer in network byte order follows the initial byte */
        delta += 255;
        delta += ntohs(*(uint16_t*)offset);
        offset += 2;
        coap_length += 2;
      }

      delta += last_delta;

      
      if (length == 13) { /* an 8-bit unsigned integer follows the initial byte */
        length += offset[0];
        offset ++;
        coap_length ++;
      } else if (length == 14) { /* A 16-bit unsigned integer in network byte order follows the initial byte */
        length += 255;
        length += ntohs(*(uint16_t*)offset);
        offset += 2;
        coap_length += 2;
      }


     
     fv->coap_option[fv->coap_option_num].delta = delta;
     last_delta = fv->coap_option[fv->coap_option_num].delta;
     fv->coap_option[fv->coap_option_num].length = length;


     PRINTLN("read_coap_packet");
     PRINT_ARRAY(offset, fv->coap_option[fv->coap_option_num].length);

     memcpy(fv->coap_option[fv->coap_option_num].value, offset, fv->coap_option[fv->coap_option_num].length);

     offset += fv->coap_option[fv->coap_option_num].length;
     coap_length += fv->coap_option[fv->coap_option_num].length;

     fv->coap_option_num ++;
	}

	// If there is more payload and it does start by 0xFF, there is payload
	if ((coap_length < packet_length) && (offset[0] == 0xff)) {
		// Dismiss end of options marker
		offset += 1;
		coap_length += 1;
		// Read payload
		fv->coap_payload_length = packet_length - coap_length;
		memcpy(fv->coap_payload, offset, fv->coap_payload_length);
	}
	else {
		PRINTLN("read_coap_packet -> packet without payload");
		fv->coap_payload_length = 0;
	}

}

/**********************************************************************/
/***        END OF FILE                                             ***/
/**********************************************************************/

/*
 * Deprecated or testing functions start here.
 */
