/* -----------------------------------------------------------------------------
   Item Scanner
  
   Accepts item numbers from PS/2 connection and submits initiates an Item
   process via a RESTful web service.
 
   Circuit:
   * Ethernet shield attached to pins 10, 11, 12, 13
   * PS/2 keyboard attached to pins 3 (clock), 4 (data)
   * Status LEDs attached to pins 7 (red), 8 (yellow), 9 (green)
 
   Created 05/27/2011 by Jon Brule
----------------------------------------------------------------------------- */

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetDHCP.h>
#include <PS2Keyboard.h>
#include <WebServiceClient.h>

#define PS2_BUFFER_SIZE 8
#define PS2_DATA_PIN 4
#define PS2_TIMEOUT 2000
#define STATUS_PIN_ERROR 7
#define STATUS_PIN_SUCCESS 9
#define STATUS_PIN_WARNING 8
#define STATUS_TIMEOUT 3000
#define WS_BUFFER_SIZE 64
#define WS_CONTENT_TYPE "application/json"
#define WS_SERVER_IP 192, 168, 100, 148
#define WS_SERVER_PORT 8080
#define WS_SERVER_URI "/proctestws-items/process/item"
#define WS_SERVER_TIMEOUT 15000
#define WS_USER_AGENT "Arduino Ethernet Shield"

const char* ip_to_str(const uint8_t*);

// Controller MAC address - must be unique on network for DHCP to work
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
const uint8_t* clientAddr;

// PS2 keyboard configuration
PS2Keyboard keyboard;
char keyBuffer[PS2_BUFFER_SIZE + 1] = { '\0' };
int keyBufferIndex = 0;
boolean haveItem = false;
unsigned long startInput = millis();

// Web Service client configuration
char wsBuffer[WS_BUFFER_SIZE + 1] = { '\0' };
char wsContentBuffer[WS_BUFFER_SIZE + 1] = { '\0' };
byte serverAddr[] = { WS_SERVER_IP };
int serverPort =  WS_SERVER_PORT;
unsigned long statusTimestamp = millis();
WebServiceClient wsclient(serverAddr, serverPort);


/******************************************************************************
 * Main Arduino Subroutines
 ******************************************************************************/

//------------------------------------------------------------------------------
// setup the arduino board
//
void setup() {
  Serial.begin(9600);
  
  // setup keyboard
  keyboard.begin(PS2_DATA_PIN);
  
  // setup status indicators
  pinMode(STATUS_PIN_ERROR, OUTPUT);
  pinMode(STATUS_PIN_SUCCESS, OUTPUT);
  pinMode(STATUS_PIN_WARNING, OUTPUT);
  
  // obtain dhcp address
  Serial.println("Blocking to obtain a DHCP lease...");
  EthernetDHCP.begin(mac);
  clientAddr = EthernetDHCP.ipAddress();
  Serial.print("IP Address: <");
  Serial.print(ip_to_str(clientAddr));
  Serial.println(">");
}

//------------------------------------------------------------------------------
// main processing loop
//
void loop() {

  // poke dhcp configuration to keep current
  EthernetDHCP.maintain();
  
  // pull character from keyboard buffer
  if (keyboard.available()) {
    byte key = keyboard.read();
    if (key == PS2_KC_ENTER && keyBufferIndex > 0) {
      haveItem = true;
    } else if (key == PS2_KC_ESC) {
      reset_key_buffer();
    } else if (keyBufferIndex < PS2_BUFFER_SIZE) {
      byte val = key - '0';
      if (val >= 0 && val <= 9) {
        Serial.print(key);
        keyBuffer[keyBufferIndex++] = key;
        keyBuffer[keyBufferIndex] = '\0';
      }
      startInput = millis();
    } 
  } else {
    if (keyBufferIndex > 0) {
      haveItem = (millis() - startInput) > PS2_TIMEOUT;
    }
  }
  
  // call web service, if necessary
  if (haveItem) {
    reset_status_pins();
    sprintf(wsContentBuffer, "{\"name\": \"%s\"}", keyBuffer);
    wsclient.call(WS_SERVER_URI, 
                  wsContentBuffer,
                  handle_web_service_success,
                  handle_web_service_failure);
    reset_key_buffer();
  }
  
  // reset error indicator if set and timeout expired
  if ((millis() - statusTimestamp) > STATUS_TIMEOUT) {
    reset_status_pins();
  }

}


/******************************************************************************
 * Internal Subroutines
 ******************************************************************************/

//------------------------------------------------------------------------------
void handle_web_service_failure(int rc) {
  if (rc == 400) {
    set_status_pin(STATUS_PIN_WARNING);
    Serial.println("WARNING");
  } else {
    set_status_pin(STATUS_PIN_ERROR);
    sprintf(wsBuffer, "FAILURE (rc = %d)", rc);
    Serial.println(wsBuffer);
  }
}

//------------------------------------------------------------------------------
void handle_web_service_success(int rc) {
  set_status_pin(STATUS_PIN_SUCCESS);
  Serial.println("SUCCESS");
}

//------------------------------------------------------------------------------
const char* ip_to_str(const uint8_t* ipAddr) {
  static char buf[16];
  sprintf(buf, "%d.%d.%d.%d\0", ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
  return buf;
}

//------------------------------------------------------------------------------
void reset_key_buffer() {
  haveItem = false;
  startInput = millis();
  keyBuffer[keyBufferIndex = 0] = '\0';
}

//------------------------------------------------------------------------------
void reset_status_pins() {
  digitalWrite(STATUS_PIN_ERROR, LOW);
  digitalWrite(STATUS_PIN_SUCCESS, LOW);
  digitalWrite(STATUS_PIN_WARNING, LOW);
}

//------------------------------------------------------------------------------
void set_status_pin(int pin) {
  digitalWrite(pin, HIGH);
  statusTimestamp = millis();
}

