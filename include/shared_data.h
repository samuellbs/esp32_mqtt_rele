
#ifndef SHARED_DATA_H     //IF NOT DEFINED
#define SHARED_DATA_H

#include <Arduino.h>
#include <Wire.h>              
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFiManager.h>
#include <EEPROM.h>

WiFiManager wm; //WiFi



//==========================================================================================
// DEBUG
//==========================================================================================

#define SERIAL 1

//==========================================================================================
// PINOS - ESP32
//==========================================================================================

#define LED_GREEN         33
#define LED_RED           32
#define PIN_FEED3         13
#define PIN_POS3          12
#define PIN_NEG3          14


//==========================================================================================
// DEFINIÇÕES EEPROM
//==========================================================================================

#define EEPROM_SIZE        512
#define FIRST_INIT_ADDRESS 10  //endereço para salvar se é a primeira inicialização


//==========================================================================================
// DEFINIÇÕES MQTT
//==========================================================================================

#define SERVER     "45.55.139.88"   //servidor node-red
#define PORT       1883             //porta mqtt
#define DEVICE_ID  ""               //nome do dispositivo (cliente name)
#define TOPIC      "topico/teste"   //tópico mqtt
#define MQTT_USER  "SB"             
#define MQTT_PSWD  "Grandchase2" 
  

//==========================================================================================
// DEFINIÇÕES DISPLAY
//==========================================================================================  

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino resetpin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


//==========================================================================================
// FLAGS E VARIÁVEIS TIMER 
//==========================================================================================

char fs_100ms = 0;   //flags de estado para cada x segundos
char fs_1s    = 0;
char fs_60s   = 0;

unsigned long millis_atual_1ms;
unsigned long millis_atual_10ms;
unsigned long millis_atual_100ms;
unsigned long millis_atual_1s;
unsigned long millis_atual_60s;

//==========================================================================================
// FLAGS GERAIS
//==========================================================================================

bool is_first_initialization = 1;  //indica se é a primeira inicialização do ESP32
bool is_wifi_connected       = 0;  //indica se o dispositivo está conectado no WiFi


//==========================================================================================
// VARIABLES
//==========================================================================================

//==========================================================================================
// STRUCT
//==========================================================================================



struct Payload {
  String        macAddress      ;
  char          devAddress[12]  ;
  String        type            ;
  unsigned int  slot            ;
  float         battery         ;
  char          status[10]      ;
};

#endif // SHARED_DATA_H