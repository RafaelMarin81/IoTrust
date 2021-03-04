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

#ifndef IPV6_LAYER_HEADER
#define IPV6_LAYER_HEADER

using namespace std;

/**********************************************************************/
/***        Global Includes       E.g. <foo.h>                      ***/
/**********************************************************************/

#include <stdint.h>

/**********************************************************************/
/***        Local Includes        E.g. "bar.h"                      ***/
/**********************************************************************/

/**********************************************************************/
/***        Macro Definitions                                       ***/
/**********************************************************************/


/*
 * IPv6/UDP checksum related structures that are normally defined
 * in system libraries. Instead, in Arduino we must define them
 * explicitly here.
 *
 * Here we fix the stdint types manually, please check the Type Definitions
 * block of code
 *
 * {
 */

#define u_int32_t uint32_t
#define u_int16_t uint16_t
#define u_int8_t  uint8_t

#define u_short uint16_t

/*
 * }
 */


#define SIZE_ETHERNET 14
#define SIZE_IPV6 40
#define SIZE_UDP 8
#define SIZE_MTU_IPV6 1280

/**********************************************************************/
/***        Type Definitions                                        ***/
/**********************************************************************/

/*
 * IPv6/UDP checksum related structures that are normally defined
 * in system libraries. Instead, in Arduino we must define them
 * explicitly here.
 *
 * {
 */

/*	
	http://unix.superglobalmegacorp.com/Net2/newsrc/netinet/udp.h.html
*/

struct udphdr {
	u_short	uh_sport;		// source port
	u_short	uh_dport;		// destination port
	short	uh_ulen;		// udp length
	u_short	uh_sum;			// udp checksum
};	


/*
	https://linux.die.net/man/7/ipv6
*/

struct in6_addr {
	unsigned char   s6_addr[16];   // IPv6 address
};


/*
	https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.bpxbd00/ip6h.htm
	https://github.com/afabbro/netinet/blob/master/ip6.h
*/

struct ip6_hdr {
	union {
		struct ip6_hdrctl {
			u_int32_t ip6_un1_flow;	// 20 bits of flow-ID
			u_int16_t ip6_un1_plen;	// payload length
			u_int8_t  ip6_un1_nxt;	// next header
			u_int8_t  ip6_un1_hlim;	// hop limit 
		} ip6_un1;
		u_int8_t ip6_un2_vfc;	// 4 bits version, top 4 bits class 
	} ip6_ctlun;
	struct in6_addr ip6_src;	// source address
	struct in6_addr ip6_dst;	// destination address
} __packed;


/*
 * } IPv6/UDP related structs.
 */

/**********************************************************************/
/***        Forward Declarations                                    ***/
/**********************************************************************/

int ipv6_process_rx(struct field_values *fv);

uint16_t checksum_udp_ipv6(uint8_t *buffer, int length_buffer);

void read_coap_packet(struct field_values *fv, uint8_t * payload);

/*
 * Print in stdout the content of a udpIp6 packet
 * The fields of the packet must have host endianess
 */
void print_udpIp6_packet(const struct field_values *udpIp6_packet);

/**********************************************************************/
/***        Constants                                               ***/
/**********************************************************************/

/**********************************************************************/
/***        Global Variables                                        ***/
/**********************************************************************/


// We store here the received ipv6 packet
extern uint8_t  ipv6_rx_buff[SIZE_MTU_IPV6];
extern uint16_t ipv6_rx_buff_len;
extern int      ipv6_rx_packet_ready; // 0 == no packet received, 1 == packet received

/**********************************************************************/
/***        Static Variables                                        ***/
/**********************************************************************/

/**********************************************************************/
/***        Static Functions                                        ***/
/**********************************************************************/

/**********************************************************************/
/***        Main Sketch routines, i.e. setup(), loop(), main()      ***/
/**********************************************************************/

/**********************************************************************/
/***        END OF FILE                                             ***/
/**********************************************************************/

#endif /* IPV6_LAYER_HEADER */

/*
 * Deprecated or testing functions start here.
 */

