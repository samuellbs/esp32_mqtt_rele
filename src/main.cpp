#include "shared_data.h"


//==========================================================================================
// INSTANCIANDO OBJETOS
//==========================================================================================

Timer timer100ms = {0, 100,   false};  // lastUpdate = 0 | interval = 100ms   | flag = false
Timer timer1s    = {0, 1000,  false};  // lastUpdate = 0 | interval = 1000ms  | flag = false
Timer timer60s   = {0, 60000, false};  // lastUpdate = 0 | interval = 60000ms | flag = false

Oled oled;
Rele rele;

void handle_timers(Timer* timer) 
{
  unsigned long currentMillis = millis();                   // Obtém o tempo atual
  if (currentMillis - timer->lastUpdate >= timer->interval) // Verifica se o intervalo foi alcançado
    { 
      timer->lastUpdate = currentMillis;                    // Atualiza o lastUpdate com previousMillis
      timer->flag = true;                                   // Define o flag como true (intervalo alcançado)
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

  (WiFi.status() == WL_CONNECTED) ? is_wifi_connected = 1 : is_wifi_connected = 0;    // Para verificar se WiFi conectou
  (is_wifi_connected) ? digitalWrite(LED_GREEN, HIGH) : digitalWrite(LED_GREEN, LOW); // WiFi ON = LED ON
}

void callback(char *topic, byte *payload, unsigned int length) 
{
  if (length < sizeof(mqtt_global_message))
  {
    memcpy(mqtt_global_message, payload, length);
    mqtt_global_message[length] = '\0';  // Adiciona o caractere nulo ao final
    is_new_mqtt_message = true;          // Indica que uma nova mensagem foi recebida
  }
  
  if (DEBUG_WIFI) 
  {
    Serial.print("Message arrived in topic: ");
    Serial.println(TOPIC);
    Serial.print("Message: ");
    Serial.println(mqtt_global_message);
    Serial.println("-----------------------");
  }
}


void mqtt_initialization(void)
{
  client.setServer(SERVER, PORT); // conectar com broker e porta 1883
  client.setCallback(callback);

  (!client.connect("ESP-32", MQTT_USER, MQTT_PSWD)) ? is_mqtt_connected = 0 : is_mqtt_connected = 1;
  if (DEBUG_WIFI && is_mqtt_connected) Serial.println("MQTT- Connect OK");
  (is_mqtt_connected) ? digitalWrite(LED_RED, HIGH) : digitalWrite(LED_RED, LOW); // MQTT ON = LED ON
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
  EEPROM.begin(EEPROM_SIZE);
  is_first_initialization = EEPROM.read(FIRST_INIT_ADDRESS); // É a primeira inicialização ? Se sim, configura WiFi
  io_initialization()     ;        //inicializar pinos
  display_initialization();        //inicializar display
  wifi_initialization()   ;        //inicializar wifi
  mqtt_initialization()   ;        //inicializar mqtt
  oled.tela = SITUATION   ;        //inicializar o display na tela de início
}

void wifi_reconnect(void)
{
  static uint16_t reconnect_wifi_counter    = 0;    // contador para tentar conectar a cada x segundos no wifi
  static uint16_t reconnect_attempt_counter = 0;    // contador de tentativas de reconectar no wifi

  (WiFi.status() == WL_CONNECTED) ? is_wifi_connected = 1 : is_wifi_connected = 0;
  (is_wifi_connected) ? digitalWrite(LED_GREEN, HIGH) : digitalWrite(LED_GREEN, LOW);

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

void display_start_screen(const char* companyName, const char* titleName, const char* authorName, const char* profession) 
{
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

const char* rele_update(void)
{ 
  return digitalRead(PIN_FEED3) ? "ON" : "OFF";  // se tiver ligado ON, desligado OFF
}

void display_update(void)
{
  rele.rele_3_state = rele_update();       //atualizar o estado dos reles (feedback)
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

void processMessageMQTT(const char* message) 
{
  if (strcmp(message, "true") == 0)
  {
    digitalWrite(PIN_POS3, HIGH); //ligar o relé
    digitalWrite(PIN_NEG3, LOW);
  }
  else if (strcmp(message, "false") == 0) 
  {
    digitalWrite(PIN_POS3, LOW); //desligar o relé
    digitalWrite(PIN_NEG3, HIGH);
  }
}

void setup()
{
    Serial.begin(115200);
    system_initialization();
} // end setup

void loop()
  {
    if(is_new_mqtt_message) 
    {
      is_new_mqtt_message = false;
      processMessageMQTT(mqtt_global_message);
    }

     handle_timers(&timer100ms); // Passa o endereço de timer100ms para a função
     handle_timers(&timer1s);    // Passa o endereço de timer1s para a função
     handle_timers(&timer60s);   // Passa o endereço de timer60s para a função
    
    if (timer100ms.flag) 
    {
        timer100ms.flag = false; // Reseta o flag
        client.loop();    
        display_update();
      }

    if (timer1s.flag) 
     {
        timer1s.flag = false;  // Reseta o flag
        handle_errors(); 
      }

     if (timer60s.flag) 
     {
      timer60s.flag = false; // Reseta o flag
      }

} // end loop
