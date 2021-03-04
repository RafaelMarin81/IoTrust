# Overview

Este firmware es una integración entre el Módulo NB-IoT BG96 (Odins IPex3G)
y un port a Arduino de CoAP-EAP de Dan para hacer un intercambio EAP sobre CoAP.

El firmware se conecta a un servidor OpenPana modificado por Dan para que
funcione con CoAP en lugar de con PANA.
La dirección del servidor OpenPANA debe especificarse en las siguientes variables:

```
#define CONTROLLER_ADDRESS "puesto8.duckdns.org" // EAP server address
#define CONTROLLER_PORT  3002 // EAP Server Port
```

Los credenciales de EAP están definidos abajo, deberán estar también
en el servidor OpenPANA.


```
// EAP User Credentials. Defined in INO file. Needed in eap-psk.cpp
const char *USER = "cysema01";
const uint8_t PSK[16] = {'I','o','T','4','I','n','d','u','s','t','r','y','_','_','_','_'};
```


## Configuración de Parámetros 3GPP para NB-IoT

La tarjeta USIM compatible con NB-IoT viene con un **APN y usuario y contraseñas**.

Estos tres parámetros de configuración se especifican en el comando `AT+QICSGP`
del Quectel BG96. En este Sketch, se especifican en la función `BG96_setNBIoT()`

```
uint8_t BG96_setNBIoT(char *apn, char *band)
...
sprintf(temp_string,"AT+QICSGP=1,1,\"%s\",\"\",\"\",1",apn);
```

Nótese que en este caso, el usuario y contraseña tienen un valor vacío.

## Configuración de parámetros para experimentos

El firmware ejecuta una máquina de estados en bucle infinito para realizar
autenticaciones CoAP-EAP contra el servidor. Las variables timer involucradas
con los intervalos están en `coap_eap_integration.ino`:


```
/*
 * Arduino loop() state machine
 */
static uint32_t check_downlink_previous_millis = 0; // Last time downlink was handled was 
static uint32_t check_downlink_interval_ms = 200; // Set to 0 for minimum possible polling. WARNING. Excessive polling might freeze the BG96. Refer to the documentation.

static uint32_t restart_auth_previous_millis = 0; // Last time a auth restart was done
static uint32_t restart_auth_interval_ms = 20000;

static int arduino_loop_state = RESTART_AUTH;
```

La variable `check_downlink_interval_ms` define el intervalo de tiempo en el que
se le hace polling al módulo BG96 para saber si hay bytes de downlink disponibles (osea
que si hemos recibido paquetes). Un menor valor bajará la latencia de los intercambios,
pero también consume más recursos por que estaría todo el tiempo consultando.

**Nota:** la variable `check_downlink_interval_ms` configurada a valores muy bajos
y si activamos muchas depuraciones del firmware con el `#define DEBUG` en archivos
como `at_client.cpp` o `bg96.cpp` se puede llegar a congelar la MCU. En otras palabras,
si queremos bajar el `check_downlink_interval_ms` a 0, es obligatorio desactivar tantos
printf() de depuración como podamos. De lo contrario, el uso excesivo de printf() puede
congelar la ejecución.

La variable `restart_auth_interval_ms` define cada cuánto tiempo se reinicia el proceso
completo de autenticación desde cero. Esto es útil para obtener resultados con un cierto
intervalo de confianza.



# Organization

El punto de entrada se encuentra en `coap_eap_integration.ino`, donde está  la lógica
necesaria para hacer el intercambio. Una vez acabado el proceso de autenticación EAP,
el firmware no hará nada más y se quedará en un bucle infinito donde no hace nada.


**Pinout**
Pinout_IPex3G_BG96_SmartEverything_Fox.png

**Sketch Arduino**
panatiki_integration.ino


**Archivos portados de PANATIKI**
aes.cpp
aes.h
eap-peer.cpp
eap-peer.h
eap-psk.cpp
eap-psk.h
eax.cpp
eax.h
include.h
uthash.h

**CoAP encode library modificada**
cantcoap.cpp
cantcoap.h

**Driver Quectel BG96 - OdinS IPex3G**
tuino096.h
at_client.cpp
at_client.h
bg96.cpp
bg96.h




El primer bloque de código que se ejecuta en `setup()`, levanta la configuración
del módulo NB-IoT BG96. Una vez conseguido, se pasa a la parte EAP.

## Tabla de equivalencia de funcionalidades de otros proyectos

| coap-eap-integration      |  examples/coap-eap-udp-based-cantcoap-optimized de la VM de Eduardo inglés | panatiki-integration     | Panatiki que me pasa Dan | Panatiki de sourceforge |
| ---                       |  ---                                                                       | ---                      | ---                      | ---                     |
| coap-eap-integration.ino  |  udp-client-energy.c                                                       | panatiki-integration.ino | panatiki.c               | pac.c                   |
| cantcoap.c (encoder CoAP) |  cantcoap.c (encoder CoAP)                                                 | state-machine.c          | state-machine.c          | state-machine.c         |


# Dependencias

## Configuración

- Servidor OpenPana (descrito abajo)
- Credenciales EAP del servidor OpenPana
- Credenciales de la tarjeta USIM

## Software

Todo el software necesario para la compilación y ejecución de éste código
está autocontenida en este repositorio.

## Hardware

- SmartEverything Fox
- Tarjeta USIM con tarifa de datos NB-IoT/LTE-M.
	 - Es necesario conocer el PIN, APN, usuario y contraseña.


# Building and running

Ejecutar el ArduinoIDE embebido en este mismo repositorio:

`./arduino-1.8.0/arduino`

Abrir el archivo `panatiki_integration.ino`. Darle al botón `Upload`.
Configurar la Serial Console a 9600 bauds.

## Hacer funcionar la simulación dentro de la VM de Eduardo

- OJO, debe intentarse dentro de la VM de Eduardo tal y como me la da, si cambias cosas puede petar.
- Recordar que las carpetas de `contiki-2.7/examples` van mapeadas a branches del repositorio PC coap-eap.
- Hacer el git checkout de la rama correspondiente para que funcione.
Ejecutar los siguientes comandos en diferentes terminales (e.g. tmux):
- `init_coap.sh` Ejecuta el binario CoAP-EAP.
- `init_radios.sh` - Inicializa freeradius en el VM.
- Fin.


# Validation

Ejecutar Wireshark en el puerto escogido en el servidor PANA.

En la serial console de arduino deberá aparecer lo siguiente:

```
authKeyAvailable=1
```

Eso significa que el intercambio CoAP-EAP ha tenido exito.

# Support

- Jesús Sánchez - jesus.sanchez4@um.es 2019-11-07
- Dan García

# Issues

## USIM PIN

La librería no maneja el caso de que la tarjeta USIM tenga un PIN asignado.
En caso de utilizar una tarjeta con PIN, será necesario hacer modificaciones
en el código de `bg96.cpp`.



# References


- Toda la documentación técnica, especialmente los PDf con la lista de
  comandos AT del módulo Quectel BG96, disponible en la web del
  producto.

