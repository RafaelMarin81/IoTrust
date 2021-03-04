/**********************************************************************/
/***        Include files                                           ***/
/**********************************************************************/

#include <Arduino.h>
#include "Sodaq_RN2483.h"

/**********************************************************************/
/***        Local Include files                                     ***/
/**********************************************************************/

#include "lorawan.h"


/**********************************************************************/
/***        Macro Definitions                                       ***/
/**********************************************************************/

// For debug purposes. Instead of sending packets, only print on console and return.
// #define LORAWAN_DRY_RUN

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
			if ((((uint8_t*)(add))[i]) < 16) {Serial.print("0");} \
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

#define LORASERIAL iotAntenna


#define PRINT_ARRAY_LOG(add, len) \
	do { \
		int i; \
		int print_array_log_value; \
		for (i = 0 ; i < (len) ; i++) { \
			if ((((uint8_t*)(add))[i]) < 16) {Serial.print("0");} \
			Serial.print((((uint8_t*)(add))[i]), HEX); \
		} \
	} while(0)

/**********************************************************************/
/***        Type Definitions                                        ***/
/**********************************************************************/

/**********************************************************************/
/***        Forward Declarations                                    ***/
/**********************************************************************/

/**********************************************************************/
/***        Constants                                               ***/
/**********************************************************************/


/*
 * Valores de ejemplo para pruebas con Join OTAA (.initOTA()).
 * 
 * deveui
 * 0004A30B001B27EE
 * 
 * appeui
 * 0D0E0A0D0B0E0E0F
 * 
 * appKey
 * 0D0E0A0D0B0E0E0F0C0A0F0E0B0A0B0E
 * 
 * 
 */

const uint8_t devAddr[4] =
{
	0x00, 0x1A, 0x62, 0xAE
};

// USE YOUR OWN KEYS!
const uint8_t appSKey[16] =
{
	0x0D, 0x0E, 0x0A, 0x0D,
	0x0B, 0x0E, 0x0E, 0x0F,
	0x0C, 0x0A, 0x0F, 0x0E,
	0x0B, 0x0A, 0x0B, 0x0E,
};

// USE YOUR OWN KEYS!
const uint8_t nwkSKey[16] =
{
	0x0D, 0x0E, 0x0A, 0x0D,
	0x0B, 0x0E, 0x0E, 0x0F,
	0x0C, 0x0A, 0x0F, 0x0E,
	0x0B, 0x0A, 0x0B, 0x0E,
};

const uint8_t devEUI[8] =
{
  0x00, 0x04, 0xA3, 0x0B, 0x00, 0x1B, 0x27, 0xEE
};

// USE YOUR OWN KEYS!
const uint8_t appEUI[8] =
{
  0x0D, 0x0E, 0x0A, 0x0D,
  0x0B, 0x0E, 0x0E, 0x0F,
};

// USE YOUR OWN KEYS!
const uint8_t appKey[16] =
{
  0x0D, 0x0E, 0x0A, 0x0D,
  0x0B, 0x0E, 0x0E, 0x0F,
  0x0C, 0x0A, 0x0F, 0x0E,
  0x0B, 0x0A, 0x0B, 0x0E,
};


/**********************************************************************/
/***        Global Variables                                        ***/
/**********************************************************************/

/**********************************************************************/
/***        Static Variables                                        ***/
/**********************************************************************/

// LoRaWAN buffers {
uint8_t tx_buff[242] = {0};
uint8_t tx_buff_len = 0;
uint8_t tx_fport = 0;

uint8_t rx_buff[242] = {0};
uint8_t rx_buff_len = 0;
uint8_t rx_fport = 0;
// }


// Research tests counters, only useful when running tests applications.
// Not needed for the operation of the SCHC logic.
// {
static int long_packet_tx_counter          = 0;
static int short_packet_tx_counter         = 0;
static int rx_packet_counter               = 0;
static int mac_tx_error_counter            = 0; // If mac tx returns something different from mac_tx_ok.
static int duplicated_packet_counter       = 0;
// }

/**********************************************************************/
/***        Static Functions                                        ***/
/**********************************************************************/

/**********************************************************************/
/***        Public Functions                                        ***/
/**********************************************************************/

void lorawan_setup()
{
#ifdef LORAWAN_DRY_RUN
	// For debugging purposes we deactivate this module to avoid sending information.
	Serial.println("LoRaWAN Module in Dry-RUN mode, not actually transmitting");
	return;
#endif

	LORASERIAL.begin(LoRaBee.getDefaultBaudRate()); // Typically RN2483 uses 57600 bauds
	delay(100);

	//
	//	if (LoRaBee.initABP(loraSerial, devAddr, appSKey, nwkSKey, true))
	//	{
	//		Serial.println("Connection to the network was successful.");
	//	}
	//	else
	//	{
	//		Serial.println("Connection to the network failed!");
	//	}


	// Funcion -> bool initOTA(SerialType& stream, const uint8_t devEUI[8], const uint8_t appEUI[8], const uint8_t appKey[16], bool adr = true, int8_t resetPin = -1);

	int i;
	for (i = 0; i < 5; i++) {
		if (LoRaBee.initOTA(LORASERIAL, devEUI, appEUI, appKey, false))
		{
			Serial.println("[Lo JOIN Success]");
			break;
		}
		else
		{
			Serial.println("Connection to the network failed!");
		}
	}

	if (i == 5) {
		Serial.println("Too many retries, Check your DevEUI and AppKey. Stopping here");
		/* TODO salir de la función gracefully y no entrar en bucle aquí. */
		for (;;);
	}

	// ACTIVATE Sodaq_RN2483 debug
	// LoRaBee.setDiag(Serial);


	int rval = 0;

	/*
	 * Establecemos el duty cycle a infinito en los tres
	 * canales que se utilizan por defecto (canales 0-2).
	 */
	rval = LoRaBee.sendCommand("mac set ch dcycle 0 0");
	rval = LoRaBee.sendCommand("mac set ch dcycle 1 0");
	rval = LoRaBee.sendCommand("mac set ch dcycle 2 0");

	/*
	 * README en la estacion base el /global.json
	 */

	/*
	 * Anadimos canales a la configuracion.
	 */
	rval = LoRaBee.sendCommand("mac set ch freq 3 867100000");
	rval = LoRaBee.sendCommand("mac set ch dcycle 3 0");
	rval = LoRaBee.sendCommand("mac set ch drrange 3 0 5");
	rval = LoRaBee.sendCommand("mac set ch status 3 on");

	/*
	 * Anadimos canales a la configuracion.
	 */
	rval = LoRaBee.sendCommand("mac set ch freq 4 867300000");
	rval = LoRaBee.sendCommand("mac set ch dcycle 4 0");
	rval = LoRaBee.sendCommand("mac set ch drrange 4 0 5");
	rval = LoRaBee.sendCommand("mac set ch status 4 on");

	/*
	 * Anadimos canales a la configuracion.
	 */
	rval = LoRaBee.sendCommand("mac set ch freq 5 867500000");
	rval = LoRaBee.sendCommand("mac set ch dcycle 5 0");
	rval = LoRaBee.sendCommand("mac set ch drrange 5 0 5");
	rval = LoRaBee.sendCommand("mac set ch status 5 on");

	/*
	 * Anadimos canales a la configuracion.
	 */
	rval = LoRaBee.sendCommand("mac set ch freq 6 867700000");
	rval = LoRaBee.sendCommand("mac set ch dcycle 6 0");
	rval = LoRaBee.sendCommand("mac set ch drrange 6 0 5");
	rval = LoRaBee.sendCommand("mac set ch status 6 on");

	/*
	 * Anadimos canales a la configuracion.
	 */
	rval = LoRaBee.sendCommand("mac set ch freq 7 867900000");
	rval = LoRaBee.sendCommand("mac set ch dcycle 7 0");
	rval = LoRaBee.sendCommand("mac set ch drrange 7 0 5");
	rval = LoRaBee.sendCommand("mac set ch status 7 on");

#ifdef DEBUG
	LoRaBee.setDiag(Serial);
#endif

}

void lorawan_send()
{


	static uint8_t rval;
	rval = -1;

	PRINT("lorawan_send Sending on fPort: ");
	PRINTLN(tx_fport);
	PRINT_ARRAY(tx_buff, tx_buff_len);
#ifdef LORAWAN_DRY_RUN
	// For debug purposes we execute a dry run, we simply print the contents and return.
	return;
#endif


	// Send without ACK (mac tx uncnf) 


	do {

		// rval = LoRaBee.sendReqAck(tx_fport, tx_buff, tx_buff_len,0); // mac tx cnf

		// deactivate overhead ADR payload bytes in the downlink, needed when
		// using the maximum payload size available. e.g. 51, 115, 242.
		LoRaBee.sendCommand("mac set adr off"); 
		LoRaBee.sendCommand("mac set retx 0"); // Set uplink TX retransmissions retries
		LoRaBee.sendCommand("mac set dr 1");

		rval = LoRaBee.sendClassC(tx_fport, tx_buff, tx_buff_len); // mac tx uncnf

		Serial.print("[Lo UP ");
		Serial.print(LoRaBee.get_upctr());
		Serial.print(" ");
		if (tx_fport < 16) Serial.print("0");
		Serial.print(tx_fport, HEX);
		Serial.print(" ");
		PRINT_ARRAY_LOG(tx_buff, tx_buff_len);
		Serial.println("]");

		PRINT("Sendreq rval: ");
		PRINTLN(rval);

		PRINT("Uplink  framecounter rval: ");
		PRINTLN(LoRaBee.get_upctr());
		PRINT("Downlink framecounter rval: ");
		PRINTLN(LoRaBee.get_dnctr());



		if (rval == 5) { // Device is still transmitting, lets give it more time
			Serial.println("LoRaWAN module busy, retrying");
			delay(1000);
			continue;
		}

		if (rval != 0) { // Something went wrong in the Sodaq_RN2483.cpp library
			mac_tx_error_counter++;
			Serial.print("RN2483 ERROR Sodaq rval: ");
			Serial.println(rval);
			break;
		}

	} while (rval != 0);

}

void lorawan_receive()
{
  
	/*
	 * We check if we received a downlink payload. If so, we copy it
	 * to the rx buffer.
	 * {
	 */

	rx_fport = 0;
	rx_buff_len = LoRaBee.receive(&rx_fport, rx_buff, sizeof(rx_buff));

	PRINTLN("LoRaBee.receive():");
	PRINT("rx_len:   ");
	PRINTLN(rx_buff_len);

	if (rx_buff_len == 0) { // No downlink packet was received by the LoRaWAN module
		return;
	}


	PRINT("rx_fport: ");
	PRINTLN(rx_fport);
	PRINT_ARRAY(rx_buff, rx_buff_len);
	
	Serial.print("[Lo DW ");
	if (rx_fport < 16) Serial.print("0");
	Serial.print(rx_fport, HEX);
	Serial.print(" ");
	PRINT_ARRAY_LOG(rx_buff, rx_buff_len);
	Serial.println("]");

	/*
	 * }
	 */



}

// LORAWAN CLASS C DEVICE ONLY
void lorawan_poll_rx(void)
{


	/*
	 * We check if we received a downlink payload. If so, we copy it
	 * to the rx buffer.
	 * {
	 */

	rx_fport = 0;
	rx_buff_len = LoRaBee.pollMacRXClassC(&rx_fport, rx_buff, sizeof(rx_buff));

	PRINTLN("LoRaBee.pollMacRX():");
	PRINT("rx_len:   ");
	PRINTLN(rx_buff_len);

	if (rx_buff_len == 0) { // No downlink packet was received by the LoRaWAN module
		return;
	}

	
	Serial.print("[Lo DN ");
	Serial.print(LoRaBee.get_dnctr());
	Serial.print(" ");
	if (rx_fport < 16) Serial.print("0");
	Serial.print(rx_fport, HEX);
	Serial.print(" ");
	PRINT_ARRAY_LOG(rx_buff, rx_buff_len);
	Serial.println("]");

	PRINT("rx_fport: ");
	PRINTLN(rx_fport);
	PRINT_ARRAY(rx_buff, rx_buff_len);

	/*
	 * }
	 */



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

