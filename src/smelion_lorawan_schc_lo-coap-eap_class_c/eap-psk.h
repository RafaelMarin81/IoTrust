/* 
 *  Copyright (C) Pedro Moreno Sánchez on 25/04/12.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *  
 *  
 *  https://sourceforge.net/projects/openpana/
 */

#ifndef __EAP_PSK
#define __EAP_PSK

#include <stdint.h>
#include "eap-peer.h"
//#include "ahi_aes.h"
#include "aes.h"
#include "eax.h"


#define ID_P_LENGTH 6

//uint8_t psk_key_available;

uint8_t check(const uint8_t * eapReqData);
void process(const uint8_t * eapReqData, uint8_t * methodState, uint8_t * decision);
void buildResp( uint8_t * eapRespData, uint8_t reqId);
//uint8_t * getKey();
//uint8_t isKeyAvailable();
void initMethodEap();


#endif
