/**********************************************************************/
/***        Requirements                                            ***/
/**********************************************************************/

/*
 *
 * Hardware
 *
 * - SME Lion that includes an updated Microchip RN2483 to its firmware v1.0.5
 *   or superior. This way, the RN2483 accepts the LoRaWAN Class C mode. In case
 *   of doubt: use a serial passthrough and try the comand `mac set class c` to see
 *   if it is supported.
 *
 *
 * Software Libraries
 *
 * - Circular Buffer v1.3.3. Install via Arduino -> Manage Libraries...
 *    -> CircularBuffer https://github.com/rlogiacco/CircularBuffer
 * - Watchdog: Adafruit SleepyDog Library v1.3.2. Install via Arduino -> Manage Libraries...
 *    -> https://github.com/adafruit/Adafruit_SleepyDog
 *
 */

/**********************************************************************/
/***        Overview                                                ***/
/**********************************************************************/

/*
 * 
 * State Machine Configuration parameters:
 * 
 * - LO_COAP_EAP_RETRANSMIT_DUPLICATED_QUERY
 * 
 * 
 * 	By default, if we receive duplicated LO-CoAP-EAP packets from the network, we
 * 	silently discard them after only sending the first one. This significatively
 * 	improves power usage and the time that takes to perform all the exchange.  This
 * 	is due to saving all the overhead of repeating all the work, i.e.,
 * 	compression/fragmentation/sending.
 * 
 * 	CAUTION
 * 
 * 	Not sending duplicated answer to each duplicated query assumes that the first
 * 	message reached the IoT Controller successfully. However, if the
 * 	original transmitted packet did not reach the IoT Controller,
 * 	the procedure is halted mid-way and will not finish correctly.
 *
 *    In case of doubt, activate LO_COAP_EAP_RETRANSMIT_DUPLICATED_QUERY
 *    and monitor a packet trace. If you experience many retransmissions,
 *    maybe consider deactivating this.
 * 
 * 	To retransmit duplicated packets define the following macro:
 * 
 * 	#define LO_COAP_EAP_RETRANSMIT_DUPLICATED_QUERY
 * 
 * - EAP-PSK credentials
 * 
 * 	These must be configured equally in the Authentication server, the last
 * 	end-point of the EAP exchange.  Go to the freeradius device and paste the
 * 	credentials in the following file:
 * 
 * 	/home/student/freeradius-psk/etc/raddb/users
 * 
 * 	E.g.
 * 
 * 	// EAP User Credentials. Defined in INO file. Needed in eap-psk.cpp
 * 	const char *USER = "cysema01";
 * 	const uint8_t PSK[16] = {'I','o','T','4','I','n','d','u','s','t','r','y','_','_','_','_'};
 * 
 * - Authentication process restart timer
 * 
 * 	This sketch solely restarts the authentication procedure employing
 * 	LO-CoAP-EAP over and over again. The interval to restart this proces is
 * 	defined by a long-duration interval. E.g. 5 min.
 * 
 * 	The sorter the period, the more times the procedure takes place. However, if
 * 	the interval is too short, then the procedure never finishes.
 * 
 * 	Please time how much it takes on average to finish the exchange and always
 * 	keep the interval above that value. Otherwise, the procedure will never
 * 	finish successfully.
 * 
 * 	static uint32_t restart_auth_interval_ms = 600000;
 * 
 */

/**********************************************************************/
/***        Include files                                           ***/
/**********************************************************************/

#include <Arduino.h>
#include <stdint.h>
#include <CircularBuffer.h>

#include <Adafruit_SleepyDog.h> // Watchdog.enable() and Watchdog.reset()...

/**********************************************************************/
/***        Local Include files                                     ***/
/**********************************************************************/



#include "context.h"
#include "schc.h" 
#include "lorawan.h" 

#include "ipv6_layer.h"

#include "eap-peer.h"
#include "cantcoap.h"
#include "include.h"
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
		int i; \
		for (i = 0 ; i < (len) ; i++) { \
			if (i % 10 == 0) \
				Serial.println(); \
			if (i % 50 == 0) \
				Serial.println(); \
			Serial.print((((uint8_t*)(add))[i]), HEX); \
			Serial.print(" "); \
		} \
		Serial.println(); \
	} while(0)

#else /* DEBUG */

	#define PRINT(...)
	#define PRINTLN(...)
	#define PRINT_ARRAY(add, len)

#endif /* DEBUG */





/*
 * Arduino loop() state machine
 */
#define LOOP_READY 1
#define LOOP_SEND_PACKET 2
#define LOOP_PROCESS_TX_PACKET 3
#define LOOP_LORAWAN_SENDING 4
#define LOOP_POLL_LORAWAN_DL 5
#define LOOP_PROCESS_RX_PACKET 6
#define LOOP_TCPIP_HANDLE   7
#define LOOP_RESTART_AUTH   8


// TICKS indicates the print of a log line to measure the time
// a given operation or group of operations take
#define TICKS 0

#define SEQ_LEN 22
#define KEY_LEN 16
#define AUTH_LEN 16


#define LO_COAP_EAP_RETRANSMIT_DUPLICATED_QUERY

/**********************************************************************/
/***        Type Definitions                                        ***/
/**********************************************************************/

/**********************************************************************/
/***        Forward Declarations                                    ***/
/**********************************************************************/

/**********************************************************************/
/***        Constants                                               ***/
/**********************************************************************/


// EAP User Credentials. Defined in INO file. Needed in eap-psk.cpp
const char *USER = "cysema01";
const uint8_t PSK[16] = {'I','o','T','4','I','n','d','u','s','t','r','y','_','_','_','_'};

/**********************************************************************/
/***        Global Variables                                        ***/
/**********************************************************************/

/*
 * Circular buffer Format

 *   Payload     LoRaWAN
 *   Length+1    fPort       LoRaWAN Message payload bytes E.g. 51 Bytes
 * +-----------+----------+-----------------------------------------------+
 * |           |          |                                               |
 * |  52       |  0x7     |  0x75, 0x79, 0x62.... 51 bytes total          |
 * |           |          |                                               |
 * +-----------+----------+-----------------------------------------------+
 *   uint8_t     uint8_t       uint8_t[payload length] bytes
 */


CircularBuffer<uint8_t,2048> lorawan_msg_tx_cb;     // LoRaWAN messages transmission circular buffer
CircularBuffer<uint8_t,2048> lorawan_msg_rx_cb;     // LoRaWAN messages received circular buffer




/**********************************************************************/
/***        Static Variables                                        ***/
/**********************************************************************/


// static struct uip_udp_conn *client_conn; // Contiki Specific code.
static uint8_t seq_number;
static uint32_t currentPort;
// static struct etimer et;

//LO-CaAP-EAP logic data
// {
uint8_t 	sent	 [70];
uint8_t 	received [70];
uint16_t 	sent_len;
uint16_t 	received_len;
char 		URI[5] = {'/','b', 0, 0, 0};


uint32_t nonce_c, nonce_s;

unsigned char auth_key[KEY_LEN] = {0};
unsigned char sequence[SEQ_LEN] = {0};

uint8_t authKeyAvailable;
uint8_t state;
static uint8_t seq_id = 0;

char URIcheck[5] = {0};
uint16_t URIcheck_len;


CoapPDU *response, *request;

uint8_t uip_appdata[1024] = {0};



static int print_diff_time = 0;

// }


// const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

// byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
// WiFiUDP udp;

/*
 * Thin abstraction layer for the integration of Contiki-CoAP-EAP code
 * into arduino.
 *
 * {
 */
static int16_t uip_datalen_s = 0;
/*
 * }
 */

// 1098 Bytes == SCHC RuleID + 1097 Bytes of payload.
// char lorem[] = /* UDP payload */ "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec consectetur eu risus at consequat. Donec dictum viverra diam, non consequat massa sollicitudin id. Sed hendrerit sed turpis a dapibus. Sed eu tincidunt urna, in fermentum purus. Donec efficitur dolor vel magna varius dictum. Sed porta urna finibus tempus porta. Vivamus scelerisque fringilla neque, sit amet venenatis urna blandit quis. Suspendisse pretium id felis in tincidunt. Nulla at dolor felis. Donec sit amet condimentum integer.";
static char lorem[] = /* UDP payload */ "Lorem ipsum.";


// SCHC Fragmentation/Reassembly {

static uint8_t schc_rx_buff[MAX_LORAWAN_PKT_LEN+1] = {0};
static uint8_t schc_rx_buff_len = 0;

// }


static uint8_t coap_tx_buf[SIZE_MTU_IPV6] = {0};
static size_t coap_tx_buf_len = 0;



/*
 * We set this variable to 1 if we are reassembling an SCHC packet and
 * keep polling the LoRaWAN server for more downlink packets. If the reassembly
 * finished (either with success or with failure) we reset this variable to 0.
 */
static int ask_next_fragment = 0;

// Research tests counters, only useful when running tests applications.
// Not needed for the operation of the SCHC logic.
// {
static int long_packet_tx_counter          = 0;
static int short_packet_tx_counter         = 0;
static int rx_packet_counter               = 0;
static int mac_tx_error_counter            = 0; // If mac tx returns something different from mac_tx_ok.
static int duplicated_packet_counter       = 0;
static int schc_reassemble_success_counter = 0;
static int schc_reassemble_fail_counter    = 0;
static int rx_len_was_zero_counter         = 0;
static int ipv6_packet_sent_counter        = 0; // Increased Everytime we send the whole "lorem[]" payload.
// }

/*
 * Arduino loop() state machine
 */


static uint32_t restart_auth_previous_millis = 0; // Last time an uplink packet was generated
static uint32_t restart_auth_interval_ms = 120000;


// LoRaWAN Class C polling interval
static uint32_t poll_lorawan_rx_previous_millis = 0; // Last time an uplink packet was generated
static uint32_t poll_lorawan_rx_interval_ms = 500;


static int arduino_loop_state = LOOP_RESTART_AUTH;

/*
 * Flags if there is an outbound ipv6 packet ready to be sent.
 * 0 = false (no packet available), 1 = true (there is a packet ready)
 */
static int ipv6_tx_packet_ready = 0;
static struct field_values udpIp6_packet = {0};

// static uint8_t payload[] = {0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46, 0x11, 0xff, 0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, // IPv6 header
//                         0x27, 0xff, 0xfe, 0x00, 0x00, 0x00, 0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0xf0,
//                         0x08, 0xda, 0x05, 0xcb, 0xe1, 0x9a,
//                         0xe7, 0xdb, 0x16, 0x33, 0x00, 0x46, 0xb2, 0x98, // UDP Header
//                         0x42, 0x02, 0x19, 0xeb, 0x4d, 0x6a, 0xb7, 0x73, 0x74, 0x6f, 0x72, 0x61, 0x67, 0x65, 0xff, // CoAP header
//                         0x31, 0x30, 0x30, // Payload
//                         0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
//                         0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
//                         0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
//                         0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31}; // 111 B 

/**********************************************************************/
/***        Static Functions                                        ***/
/**********************************************************************/

static int uip_newdata(void)
{
	return uip_datalen_s;
}

static size_t uip_datalen(void)
{
	return uip_datalen_s;
}


static void uip_udp_packet_send(const uint8_t *data, size_t data_len)
{

	PRINTLN("UDP send packet!");
	PRINT_ARRAY(data, data_len);

	memcpy(coap_tx_buf, data, data_len);
	coap_tx_buf_len = data_len;

	// We signal the main loop state machine that we have a Tx packet ready
	ipv6_tx_packet_ready = 1;


}


	static void
tcpip_handler(void)
{

	PRINT("State: ");
	PRINTLN(state);

	PRINT("authKeyAvailable: ");
	PRINTLN(authKeyAvailable);

	// if(uip_newdata()) {
	// If new downlink data available
	if(uip_datalen() != 0) {
		PRINTLN("tcpip_handler Downlink packet received");

		//printf("R: (%d)\n", uip_datalen());
		//printf_hex(uip_appdata, uip_datalen());
		#if TICKS
		printf("tick %d\n",seq_id);
		#endif

		// Check for retransmission
		if(memcmp(uip_appdata, received ,uip_datalen()) == 0)
		{
			PRINTLN("tcpip_handler Retransmission received, skiping");
			/*
			 * Here we can send a retransmission of the previous packet, just in case,
			 * but since it consumes so much power, we don't do it and simply silently
			 * discard the received duplicated packet.
			 *
			 * CAUTION
			 *
			 * Not sending here the last sent packet assumes that the first
			 * message reached the IoT Controller successfully. However, if the
			 * original transmitted packet did not reach the IoT Controller,
			 * the procedure is stoped midway.
			 *
			 * In case of doubt, activate LO_COAP_EAP_RETRANSMIT_DUPLICATED_QUERY
			 * and monitor a packet trace. If you experience many retransmissions,
			 * maybe consider deactivating this.
			 */
			#ifdef LO_COAP_EAP_RETRANSMIT_DUPLICATED_QUERY
			uip_udp_packet_send(sent, sent_len);
			#endif

			return;
		}

		// Store last message received for future duplicate checking
		memcpy(received, uip_appdata, uip_datalen());
		_CoapPDU_buf_withCPDU(request, (uint8_t*)uip_appdata,uip_datalen());	

		if(!validate(request) || getType(request) == COAP_ACKNOWLEDGEMENT) {
			PRINTLN("ERR 250 packet is not valid or is ACK");

			return; 
		}

		#ifdef DEBUG_SKETCH
		printHuman(request);
		#endif

		//		getURI(request, URIcheck, 10, &URIcheck_len);
		//		printf("URIcheck: %s | URI %s\n",URIcheck,URI);
		//		if(memcmp(URIcheck, URI , URIcheck_len) != 0)
		//			return;					

		unsigned char *payload, *ptr; 
		uint8_t mac2check[AUTH_LEN] 	={0};
		memset(mac2check,0,AUTH_LEN);
		uint8_t mac[AUTH_LEN] 		={0};
		memset(mac,0,16);
		unsigned char _auth_key[KEY_LEN] = {0};
		memset(_auth_key,0,KEY_LEN);

		uint8_t responsecode = COAP_CHANGED;

		if((getCode(request) == COAP_POST)){
			if(!state) {
				PRINTLN("DBG 279");
				responsecode = COAP_CREATED;

				// EAP Restart
				memset(&msk_key,0, MSK_LENGTH);
				eapRestart=TRUE;
				eap_peer_sm_step(NULL);

				// creating the id of the service
				URI[2] = '/';
				unsigned int random =rand()*1000; 
				URI[3] = '0' + (random % 10);

			}
			if(eapKeyAvailable){

				PRINTLN("DBG eapKeyAvailable");
				ptr = (unsigned char*)&sequence;

				unsigned char label[] = "IETF COAP AUTH";
				memcpy(ptr,label,(size_t)14);
				ptr += 14;

				memcpy(&nonce_c, getOptionPointer(request,COAP_OPTION_NONCE),(size_t)getOptionLength(request,COAP_OPTION_NONCE));

				memcpy(ptr, &(nonce_c),sizeof(uint32_t));
				ptr += 4;

				memcpy(ptr, &(nonce_s),sizeof(uint32_t));

				do_omac(msk_key, sequence, SEQ_LEN, _auth_key);
				authKeyAvailable = TRUE;
				// Verify the AUTH Option
				// Copy the mac
				memcpy(&mac2check,getPDUPointer(request)+getPDULength(request)-16-5,16);
				// Zeroing the mac in meesage
				memcpy(getPDUPointer(request)+getPDULength(request)-16-5,&mac,16);
				// Setting the MAC
				do_omac(_auth_key, getPDUPointer(request),getPDULength(request), mac);

				if(memcmp(&mac2check, &mac,AUTH_LEN) != 0)
				{
					Serial.println("error");					}

				memset(mac2check,0,AUTH_LEN);

				memcpy(auth_key, _auth_key, KEY_LEN);


			}

			eapReq = TRUE;
			eap_peer_sm_step(getPayloadPointer(request));

		}

		else{
			// Es el ACK del GET
			PRINTLN("DBG - ACK del Get");
			return;
		}

		PRINTLN("DBG 339 creating response");
		// Creating Response
		reset(response);
		setVersion(response,1);
		setType(response,COAP_ACKNOWLEDGEMENT);
		setCode(response,(Code)responsecode);
		setToken(response,
				getTokenPointer(request),
				(uint8_t)getTokenLength(request));

		setMessageID(response,getMessageID(request));



		if((getCode(request) == COAP_POST)){

			if(! state){
				state++; 
				_setURI(response,&URI[0],4);
			}
			if(!authKeyAvailable){
				if (eapResp){
					uint16_t len = ntohs( ((struct eap_msg*) eapRespData)->length);
					setPayload(response, eapRespData, len);
				}
			}else{
				addOption(response,COAP_OPTION_AUTH, AUTH_LEN, (uint8_t *)&mac2check);

				do_omac(_auth_key, getPDUPointer(response),
						getPDULength(response), mac2check);
				memcpy(getPDUPointer(response)+getPDULength(response)-16,&mac2check,16);
			}


			//printf("S: (%d) \n", (size_t)getPDULength(response));
			//printf_hex(getPDUPointer(response), getPDULength(response));
			printHuman(response);

			if (state == 0 && authKeyAvailable == 0) {
				Serial.println("[AU DN 01]");
				Serial.println("[AU UP 01]");
			} else if (state == 1 && authKeyAvailable == 0) {
				Serial.println("[AU DN 03]");
				Serial.println("[AU UP 03]");
			} else if (state == 1 && authKeyAvailable == 1) {
				Serial.println("[AU DN 05]");
				Serial.println("[AU UP 05]");
			}


			uip_udp_packet_send(getPDUPointer(response), (size_t)getPDULength(response));

			memcpy(sent, getPDUPointer(response), (size_t)getPDULength(response));
			sent_len = getPDULength(response);

		}
	}

	if(authKeyAvailable){
		PRINTLN("tick finish\n");
		// powertrace_print("");
		// etimer_set(&et, 5 * CLOCK_SECOND);
		state = 0;
		return;
	}

	// etimer_set(&et, 45 * CLOCK_SECOND);


}

/*---------------------------------------------------------------------------*/
	static void
timeout_handler(void)
{
	// etimer_stop(&et);

	seq_id = 0;
	// etimer_restart(&et);
	state = 0;

	memset(&msk_key,0, MSK_LENGTH);
	eapRestart=TRUE;
	eap_peer_sm_step(NULL);

	memset(&auth_key,0, AUTH_LEN);
	memset(&sequence,0, SEQ_LEN);

	authKeyAvailable = 0;
	//currentPort++;

//	udp_bind(client_conn, UIP_HTONS( (currentPort) )  );

	URI[2] = 0;
	URI[3] = 0;
	// powertrace_print("");
	// printf("tick init\n");

	reset(request);
	setVersion(request,1);
	setType(request,COAP_NON_CONFIRMABLE);
	setCode(request,COAP_POST);
	int token=1;
	setToken(request,(uint8_t*)&token,0);
	setMessageID(request,htons(0x0000));
	uint8_t noresponseValue = 0x7f;
	addOption(request,COAP_OPTION_NO_RESPONSE,1,&noresponseValue);
	_setURI(request,"/b",2);

	nonce_s = rand();
	addOption(request,COAP_OPTION_NONCE,4,(uint8_t*)&nonce_s);
	setPayload(request, (uint8_t*)USER, strlen(USER));

#if TICKS		
	printf("tick init\n");
#endif
	Serial.println("[AU UP 00]");
	uip_udp_packet_send(getPDUPointer(request),(size_t)getPDULength(request));
#if TICKS		
	printf("tick init\n");
#endif
	// etimer_set(&et, 45 * CLOCK_SECOND);


}

/**********************************************************************/
/***        Public Functions                                        ***/
/**********************************************************************/



/**********************************************************************/
/***        main() setup() loop()                                   ***/
/**********************************************************************/

void setup() {
  Serial.begin(115200);

	// We wait for the Serial UART to be ready.
   delay(200);

  

	/*
	 * NOTE: the library does not accept any arbitrary parameter value,
	 * it will default to the closest thing, e.g. rounding up or down.
	 * for this reason, please check the rval because it gives you the
	 * actual value configured.
	 */
	int rval = Watchdog.enable(16000); // ms

	Serial.print("Watchdog configured to (ms): ");
	Serial.println(rval);

	lorawan_setup();

	Watchdog.reset();


	//print_local_addresses();
	URI[5] += (rand()%9)+1;


	request  = _CoapPDU();
	response = _CoapPDU();

	PRINTLN("Setup end");
}


void loop() {

	uint32_t current_millis = millis(); // Current timestamp
	int i = 0;

	/*
	 * Arduino Specific Code
	 *
	 * Manage downlink data.
	 *
	 * {
	 */
	switch (arduino_loop_state) {


		case LOOP_RESTART_AUTH:
			//Init pana state machine
			Serial.print("\nPANATIKI: Starting Auth Process current_millis: ");
			Serial.println(current_millis);
			timeout_handler();

			print_diff_time = 1;
			arduino_loop_state = LOOP_READY;

			break;

		case LOOP_READY:

			Watchdog.reset();

			if (ipv6_rx_packet_ready != 0) {
				ipv6_rx_packet_ready = 0;

				arduino_loop_state = LOOP_TCPIP_HANDLE;
				
				break;
			}

			if (ipv6_tx_packet_ready != 0) {
				ipv6_tx_packet_ready = 0;

				arduino_loop_state = LOOP_SEND_PACKET;
				
				break;
			}

			if (lorawan_msg_rx_cb.size() != 0) { // There are pending LoRaWAN Rx messages
				arduino_loop_state = LOOP_PROCESS_RX_PACKET;
				break;
			}



			/*
			 * Each time that the interval is met, we change the state
			 * and reset the timer
			 */
			if (current_millis - restart_auth_previous_millis >= restart_auth_interval_ms) {
				arduino_loop_state = LOOP_RESTART_AUTH;
				restart_auth_previous_millis = current_millis;
				break;
			}


			/*
			 * Each time that the interval is met, we change the state
			 * and reset the timer
			 */
			if (current_millis - poll_lorawan_rx_previous_millis >= poll_lorawan_rx_interval_ms) {
				arduino_loop_state = LOOP_POLL_LORAWAN_DL;
				poll_lorawan_rx_previous_millis = current_millis;
				break;
			}

			break;


		case LOOP_SEND_PACKET:
			//Init pana state machine
			Serial.println("Generating IPv6 packet");

			/*
			 * Since we do not implement a IPv6/UDP stack,
			 * we handcraft here the IPv6/UDP part of the intermediate
			 * representation in our struct field_values.
			 */

			// Ipv6 header fields 
			udpIp6_packet.ipv6_version = 6;
			udpIp6_packet.ipv6_traffic_class = 0;
			udpIp6_packet.ipv6_flow_label = 0;
			udpIp6_packet.ipv6_payload_length = 0; // Nwk to host
			udpIp6_packet.ipv6_next_header = 17;
			udpIp6_packet.ipv6_hop_limit = 64;
			// Udp header fields
			//udpIp6_packet.udp_dev_port = 59355; // Nwk to host
			udpIp6_packet.udp_dev_port = 0xE7DB;
			//udpIp6_packet.udp_app_port = 5683; // Nwk to host
			udpIp6_packet.udp_app_port = 0x1633;
			udpIp6_packet.udp_length = 0; // Nwk to host, length includes header
			udpIp6_packet.udp_checksum = 0;

			/* -----*/
			string_to_bin(udpIp6_packet.ipv6_dev_prefix, "FE80000000000000");
			string_to_bin(udpIp6_packet.ipv6_dev_iid,    "080027FFFE000000");
			string_to_bin(udpIp6_packet.ipv6_app_prefix, "FE80000000000000");
			string_to_bin(udpIp6_packet.ipv6_app_iid,    "0A0027FFFE542E4A");

			// Coap header and payload

			/*
			 * At this point the remaining part of the packet is stored
			 * in binary at coap_tx_buf, this was generated by other LO-CoAP-EAP states
			 * of the main loop state machine. What we have left, is to
			 * decode the data here into our struct field_values.
			 *
			 * {
			 */

			udpIp6_packet.udp_length = SIZE_UDP + coap_tx_buf_len;

			read_coap_packet(&udpIp6_packet, coap_tx_buf);


			/*
			 * }
			 */



		//  IP addresses
		//  for (int i = 0; i < 8; i++){
		//    udpIp6_packet.ipv6_dev_prefix[i] = 1;
		//  }
		//  for (int i = 8; i < 16; i++){
		//    udpIp6_packet.ipv6_dev_iid[i-8] = 1;
		//  }
		//  for (int i = 0; i < 8; i++){
		//    udpIp6_packet.ipv6_app_prefix[i] = 1;
		//  }
		//  for (int i = 8; i < 16; i++){
		//    udpIp6_packet.ipv6_app_iid[i-8] = 1;
		//  }






			arduino_loop_state = LOOP_PROCESS_TX_PACKET;

			break;

		case LOOP_PROCESS_TX_PACKET:

			//Print the contents of the packet in human readable form

			print_udpIp6_packet(&udpIp6_packet);

			/*
			 * Compress headers and insert schc fragments in the lorawan tx circular buffer circular buffer packets
			 */
			schc_compress(udpIp6_packet);

			if (lorawan_msg_tx_cb.size() != 0) { // There are pending LoRaWAN Tx messages
				arduino_loop_state = LOOP_LORAWAN_SENDING;
				break;
			}

			arduino_loop_state = LOOP_READY;
			break;

		case LOOP_LORAWAN_SENDING:

			Watchdog.reset();
			/*
			 * extract lorawan message from circular buffer and copy it to
			 * lorawan tx buffer.
			 */
			tx_buff_len = lorawan_msg_tx_cb.shift() -1;
			tx_fport = lorawan_msg_tx_cb.shift();
			for (i = 0 ; i < tx_buff_len ; i++) {
				tx_buff[i] = lorawan_msg_tx_cb.shift(); 
			}

			lorawan_send();



			if (lorawan_msg_tx_cb.size() == 0) { // No more lorawan messages to send

				arduino_loop_state = LOOP_READY;
				break;
			}

			if (lorawan_msg_tx_cb.size() != 0) { // There are pending LoRaWAN Tx messages, we keep sending them over the radio.
				arduino_loop_state = LOOP_LORAWAN_SENDING;
				break;
			}

			break;

		case LOOP_POLL_LORAWAN_DL:


			/*
			 * Since we are a LoRaWAN Class C device,
			 * we can receive a new downlink packet whenever.
			 *
			 * We poll the lorawan library to seek new downlink messages.
			 */





			/*
			 * At this point, if the network sever sent a downlink packet, it is stored temporarily in the reception buffer
			 * of the LoRaWAN library. We check for the existence of a downlink packet and, in such case, must consume this packet here and store it in the circular buffer for received packets.
			 */
			lorawan_poll_rx();

			/*
			 * We process here the received packet and insert it in the circular buffer.
			 * {
			 */

			if (rx_buff_len != 0) { //We received a packet in the downlink
				ask_next_fragment = 1;

				/*
				 * We build the SCHC packet by concatenating the fport, which contains the
				 * SCHC RuleID, and the LoRaWAN payload.
				 * {
				 */
				lorawan_msg_rx_cb.push(rx_buff_len +1);
				lorawan_msg_rx_cb.push(rx_fport);
				int j;
				for (j = 0 ; j < rx_buff_len; j++) {
					lorawan_msg_rx_cb.push(rx_buff[j]);
				}
				/*
				 * }
				 */
			}


			arduino_loop_state = LOOP_READY;
			break;

		case LOOP_PROCESS_RX_PACKET:

			/*
			 * extract lorawan message from circular buffer and copy it to
			 * temporal buffer.
			 */
			schc_rx_buff_len = lorawan_msg_rx_cb.shift();
			for (i = 0 ; i < schc_rx_buff_len ; i++) {
				schc_rx_buff[i] = lorawan_msg_rx_cb.shift(); 
			}


			schc_reassemble(schc_rx_buff, schc_rx_buff_len);

			if (ipv6_rx_packet_ready == 1) { //There is a full IPv6 pcket ready to be processed
				arduino_loop_state = LOOP_READY;
				break;
			}

			if (lorawan_msg_rx_cb.size() == 0) { //There are no more packets left in the circular buffer
				arduino_loop_state = LOOP_READY;
				break;
			}

			arduino_loop_state = LOOP_POLL_LORAWAN_DL;
			break;
		case LOOP_TCPIP_HANDLE:

			PRINTLN("LOOP_TCPIP_HANDLE received packet: ");
			
			/*
			 * Store the CoAP PDU into the thin adaptation layer for the
			 * LO-CoAP-EAP libraries
			 *
			 * {
			 */

			// CoAP PDU length
			uip_datalen_s = ipv6_rx_buff_len - SIZE_IPV6 - SIZE_UDP;


			// Copy only the CoAP PDU, ommit all the IPv6/UDP stuff
			memcpy(uip_appdata, &ipv6_rx_buff[SIZE_IPV6 + SIZE_UDP], uip_datalen_s);

			/*
			 * }
			 */

			// Call the LO-CoAP-EAP reception logic
			tcpip_handler();


			Serial.print("authKeyAvailable: ");
			Serial.println(authKeyAvailable);
			if (authKeyAvailable && print_diff_time) {
				Serial.print("Key available: ");
				PRINT_ARRAY(auth_key, KEY_LEN);
				Serial.print("    time: ");
				Serial.println(current_millis - restart_auth_previous_millis);
				print_diff_time = 0;
			}


			arduino_loop_state = LOOP_READY;
			break;

	}



}

/**********************************************************************/
/***        END OF FILE                                             ***/
/**********************************************************************/

/*
 * Deprecated or testing functions start here.
 */

// int freeRam () {
//   extern int __heap_start, *__brkval; 
//   int v; 
//   return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
// }
