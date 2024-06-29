#include "shared_data.h"


//==========================================================================================
// INSTANCIANDO OBJETOS
//==========================================================================================

Oled oled;
Rele rele;

void handle_timers(void)
{
  /*      Descrição de funcionamento:

         Definição de tempo de timers
  */
  unsigned long t_atual;
  t_atual = millis(); // função milis retorna o tempo que o microcontrolador está ligado

  if (t_atual - millis_atual_100ms >= 99) // 100ms
  {
    millis_atual_100ms = t_atual;
    is_100ms = 1;
  }
  if (t_atual - millis_atual_1s >= 999) // 1s
  {
    millis_atual_1s = t_atual;
    is_1s = 1;
  }
  if (t_atual - millis_atual_60s >= 59999) // 60s
  {
    millis_atual_60s = t_atual;
    is_60s = 1;
  }
}

void wifi_initialization(void)
{
  wm.setDebugOutput(false);       // TRUE para verificar infos de conexão no monitor serial
  WiFi.mode(WIFI_STA);            // conectar no WiFi com dados salvos na memória flash
  wm.setConfigPortalTimeout(180); // portal de wifi aberto por 180s

  if (is_first_initialization)
  {
    is_first_initialization = 0;
    EEPROM.write(FIRST_INIT_ADDRESS, is_first_initialization);
    EEPROM.commit();
    wm.resetSettings(); // sempre que ligar pela primeira vez resetar WiFi
  }

  auto_wifi_connect = wm.autoConnect("ESP_SAMUEL", "password"); //conexão automática no WiFi

  if (DEBUG_WIFI)
  {
    Serial.println("\n*************** WiFi INFORMATION ***************");
    Serial.println("SAVED:" + ((String)wm.getWiFiIsSaved()) ? "WiFi SAVED" : "WiFi NOT SAVED");
    Serial.println("SSID: " + (String)wm.getWiFiSSID());
    Serial.println("PASS: " + (String)wm.getWiFiPass());
    Serial.println("************************************************");
  }

  (WiFi.status() == WL_CONNECTED) ? is_wifi_connected = 1 : is_wifi_connected = 0;
}

void callback(char *topic, byte *payload, unsigned int length)
{
  if (DEBUG_WIFI){
    Serial.print("Message arrived in topic: ");
    Serial.println(TOPIC);
    Serial.print("Message:");
  for (int i = 0; i < length; i++)
  {
      Serial.print((char)payload[i]);
  }
    Serial.println();
    Serial.println("-----------------------");
  }
}

void mqtt_initialization(void)
{
  client.setServer(SERVER, PORT); // conectar com broker e porta 1883
  client.setCallback(callback);

  (!client.connect("ESP-32", MQTT_USER, MQTT_PSWD)) ? is_mqtt_connected = 0 : is_mqtt_connected = 1;
  if (DEBUG_WIFI && is_mqtt_connected) Serial.println("MQTT- Connect OK");
  client.subscribe(TOPIC);
}

char io_initialization(void)
{
  char ret = -1;
  pinMode(PIN_FEED3, INPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(PIN_NEG3, OUTPUT);
  pinMode(PIN_POS3, OUTPUT);

  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED,   LOW);
  digitalWrite(PIN_POS3,  LOW);
  digitalWrite(PIN_NEG3, HIGH);

  ret = 0;
  return ret;
}

char display_initialization(void)
{
  char ret = -1;
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  ret = 0;
  return ret;
}

void system_initialization(void)
{
  // btStop();
  EEPROM.begin(EEPROM_SIZE);
  is_first_initialization = EEPROM.read(FIRST_INIT_ADDRESS); // É a primeira inicialização ? Se sim, configura WiFi
  io_initialization()     ;        //inicializar pinos
  display_initialization();        //inicializar display
  wifi_initialization()   ;        //inicializar wifi
  mqtt_initialization()   ;        //inicializar mqtt
  oled.tela = START       ;        //inicializar o display na tela de início
}

void wifi_reconnect(void)
{
  static uint16_t reconnect_wifi_counter    = 0;    // contador para tentar conectar a cada x segundos no wifi
  static uint16_t reconnect_attempt_counter = 0;    // contador de tentativas de reconectar no wifi

  (WiFi.status() == WL_CONNECTED) ? is_wifi_connected = 1 : is_wifi_connected = 0;

  if (!is_wifi_connected)
  {
    reconnect_wifi_counter++;
    if (reconnect_wifi_counter > MAX_ATTEMPTS_WIFI)  //para tentar a cada 5 segundos e durante 5 segundos
    {
      reconnect_attempt_counter++;
      if (reconnect_attempt_counter <= MAX_ATTEMPTS_WIFI) //passaram-se 5 segundos, faça 5 tentativas
      {
        WiFi.begin();
        //delay(1000);
        if ((WiFi.status() == WL_CONNECTED))
        {
          is_wifi_connected         = 1;
          reconnect_wifi_counter    = 0;
          reconnect_attempt_counter = 0;
          if (DEBUG_WIFI) Serial.println("WiFi reconectado com sucesso");
        }
        else
        {
          is_wifi_connected = 0;
          if (DEBUG_WIFI) Serial.println("Falha na tentativa de reconexão ao WiFi.");
        }
      }
      else
      {
        if (DEBUG_WIFI)
          Serial.println("Número de tentativas atingidas");
        reconnect_wifi_counter    = 0; 
        reconnect_attempt_counter = 0; // para tentar novamente
      }
    }
  }
    else
    {
      reconnect_wifi_counter = 0; // se tiver conectado, mantém o contador em zero
      if (DEBUG_WIFI) Serial.println("WiFi já está conectado");
    }
  }

void mqtt_reconnect(void)
{
  static uint16_t reconnect_mqtt_counter    = 0;    // contador para tentar conectar a cada x segundos no mqtt
  static uint16_t reconnect_attempt_counter = 0;    // contador de tentativas de reconectar no mqtt

  if (!is_mqtt_connected && is_wifi_connected) //para garantir tentativa de reconexão somente se WiFi estiver conectado
  {
    reconnect_mqtt_counter++;
    if(reconnect_mqtt_counter > MAX_ATTEMPTS_MQTT)
    {
      reconnect_attempt_counter++;
      if(reconnect_attempt_counter <= MAX_ATTEMPTS_MQTT)
      {
        mqtt_initialization(); //função para conexão MQTT
        if(is_mqtt_connected)
        {
          reconnect_mqtt_counter    = 0;
          reconnect_attempt_counter = 0;
          if (DEBUG_WIFI) Serial.println("MQTT reconectado com sucesso");
        }
         else
         {
          is_mqtt_connected = 0;
          if (DEBUG_WIFI) Serial.println("Falha na tentativa de reconexão ao MQTT.");
         }
      }
      else
      {
        if (DEBUG_WIFI)
          Serial.println("Número de tentativas atingidas");
        reconnect_mqtt_counter    = 0; 
        reconnect_attempt_counter = 0; // para tentar novamente
      }
    }
  }
  else
  {
    reconnect_mqtt_counter = 0; // se tiver conectado, mantém o contador em zero
    if (DEBUG_WIFI) Serial.println("MQTT já está conectado");
  }
}

void handle_errors(void)
{
    wifi_reconnect(); //tentativa de reconectar wifi (tem que ser primeiro para verificar status wifi)
    mqtt_reconnect(); //tentativa de reconectar mqtt
}

void display_start_screen(const char* companyName, const char* titleName, const char* authorName, const char* profession) {
  display.clearDisplay();
  display.setTextSize(TEXT_SIZE_SMALL);
  display.setTextColor(TEXT_COLOR);
  display.setRotation(2);
  display.setCursor(28, 0);
  display.println(companyName);
  display.setCursor(20, 18);
  display.print(titleName);
  display.setCursor(0, 40);
  display.print("Autor: ");
  display.print(authorName);
  display.setCursor(0, 55);
  display.print(profession);
  display.display();
}

void display_situation_screen(const char* titleName, const char* statusRele_3)
{
  display.clearDisplay();
  display.setTextSize(TEXT_SIZE_SMALL);
  display.setTextColor(TEXT_COLOR);
  display.setRotation(2);
  display.setCursor(28, 0);
  display.println(titleName);
  display.setCursor(0, 40);
  display.print("Rele 3: ");
  display.print(statusRele_3);
  display.display();
}

void rele_update(void)
{ 
  rele.rele_3_state = digitalRead(PIN_FEED3) ? "ON" : "OFF";  //se tiver ligado ON, desligado OFF
}

void display_update(void)
{
  static uint8_t start_screen_counter = 0; //contador para manter tela de início por x segundos
  rele_update();                           //atualizar o estado dos reles (feedback)
  oled.nivel = 0;                          //menu só tem um nível
  switch (oled.nivel) {
    case 0:
      switch (oled.tela) {
        case START:
          display_start_screen("SB INDUSTRIES", "Reles Latch MQTT", "Samuel", "Eng Eletronico");
          break;
        case SITUATION:
          display_situation_screen("RELES SITUATION", rele.rele_3_state);
          break;
      }
      break;
  }
}

void setup()
{
    Serial.begin(115200);
    system_initialization();
} // end setup

void loop()
  {
    handle_timers();

    if(is_100ms)
    {
      is_100ms = 0;
      client.loop();
      display_update();
    }
    if (is_1s)
    {
      is_1s = 0;
      handle_errors();
    }
} // end loop
