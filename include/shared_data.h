
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


//==========================================================================================
// INSTANCIANDO OBJETOS
//==========================================================================================

WiFiManager wm; //WiFi
WiFiClient espClient;
PubSubClient client(espClient);


//==========================================================================================
// DEBUG
//==========================================================================================

#define SERIAL     0
#define DEBUG_WIFI 0 //debug WiFi e MQTT

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
// DEFINIÇÕES MQTT e WIFI
//==========================================================================================

#define SERVER     "45.55.139.88"                         //servidor node-red
#define PORT       1883                                   //porta mqtt
#define DEVICE_ID  ""                                     //nome do dispositivo (cliente name)
#define TOPIC      "SB/esp32-rele-latch@esp32/devices/"   //tópico mqtt
#define MQTT_USER  "SB"             
#define MQTT_PSWD  "Grandchase2" 

#define MAX_ATTEMPTS_WIFI 5         //máximo de tentativas de reconectar no wifi
#define MAX_ATTEMPTS_MQTT 5         //máximo de tentativas de reconectar no mqtt


//==========================================================================================
// DEFINIÇÕES DISPLAY
//==========================================================================================  

#define SCREEN_WIDTH      128    // OLED display width, in pixels
#define SCREEN_HEIGHT     64     // OLED display height, in pixels
#define OLED_RESET       -1      // Reset pin # (or -1 if sharing Arduino resetpin)
#define TEXT_SIZE_LARGE   2      // Tamanho da letra: grande
#define TEXT_SIZE_MEDIUM  1.9    // Tamanho da letra: médio
#define TEXT_SIZE_SMALL   1.0    // Tamanho da letra: pequeno
#define TEXT_COLOR        WHITE  // cor de letra: branco

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//==========================================================================================
// FLAGS GERAIS
//==========================================================================================

bool is_first_initialization = 1;  //indica se é a primeira inicialização do ESP32
bool is_wifi_connected       = 0;  //indica se o dispositivo está conectado no WiFi
bool is_mqtt_connected       = 0;  //indica se o dispositivo está conectado no MQTT
bool auto_wifi_connect       = 0;  //variável para a conexão automática do WiFi Manager 
bool is_start_screen_display = 0;  //variável para tela inicial do sistema
bool is_new_mqtt_message     = 0;  //indica se há nova mensagem mqtt

//==========================================================================================
// VARIABLES
//==========================================================================================
char mqtt_global_message[6]; // 5 caracteres + caractere nulo

//==========================================================================================
// TYPEDEF ENUM
//==========================================================================================

 typedef enum  //referenciar numero com palavra
 {
  START     = 0,
  SITUATION = 1,
 }Tela;


//==========================================================================================
// STRUCT
//==========================================================================================

typedef struct 
{
    unsigned long lastUpdate; // Última vez que o temporizador foi verificado
    unsigned long interval;   // Duranção do intervalo em ms
    bool flag;                // Flag indicando se o intervalo foi alcançado
} Timer;

typedef struct
{
  uint16_t nivel;   // nível do menu
  uint16_t tela;    // nível da tela
}Oled;

typedef struct 
{
  const char* rele_1_state;   // Estado do relé 1 (ligado/desligado)
  const char* rele_2_state;   // Estado do relé 2 (ligado/desligado)
  const char* rele_3_state;   // Estado do relé 3 (ligado/desligado)
}Rele;

#endif // SHARED_DATA_H
