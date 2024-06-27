#include "shared_data.h"

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
  wm.setConfigPortalTimeout(120); // portal de wifi aberto por 180s

  if (is_first_initialization)
  {
    is_first_initialization = 0;
    EEPROM.write(FIRST_INIT_ADDRESS, is_first_initialization);
    EEPROM.commit();
    wm.resetSettings(); // sempre que ligar resetar WiFi
  }

  auto_wifi_connect = wm.autoConnect("ESP_SAMUEL", "password"); // password protected ap
                                                                //
                                                                //   if(!res) {
                                                                //        Serial.println("Failed to connect");
                                                                //        // ESP.restart();
                                                                //   }
                                                                //   else {
                                                                //        //if you get here you have connected to the WiFi
                                                                //        Serial.println("connected...yeey :)");
                                                                //   }
  if (DEBUG_WIFI)
  {
    Serial.println("\n*************** WiFi INFORMATION ***************");
    Serial.println("SAVED:" + ((String)wm.getWiFiIsSaved()) ? "WiFi SAVED" : "WiFi NOT SAVED");
    Serial.println("SSID: " + (String)wm.getWiFiSSID());
    Serial.println("PASS: " + (String)wm.getWiFiPass());
    Serial.println("************************************************");
  }

  (WiFi.status() != WL_CONNECTED) ? is_wifi_connected = 0 : is_wifi_connected = 1;
}

void callback(char *topic, byte *payload, unsigned int length)
{
  if (SERIAL)
    Serial.print("Message arrived in topic: ");
  if (SERIAL)
    Serial.println(TOPIC);
  if (SERIAL)
    Serial.print("Message:");
  for (int i = 0; i < length; i++)
  {
    if (SERIAL)
      Serial.print((char)payload[i]);
  }
  if (SERIAL)
    Serial.println();
  if (SERIAL)
    Serial.println("-----------------------");
}

void mqtt_initialization(void)
{
  client.setServer(SERVER, PORT); // conectar com broker e porta 1883
  client.setCallback(callback);

  (!client.connect("ESP-32", MQTT_USER, MQTT_PSWD)) ? is_mqtt_connected = 0 : is_mqtt_connected = 1;
  if (SERIAL)
    Serial.println("MQTT- Connect OK");
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
  digitalWrite(LED_RED, LOW);
  digitalWrite(PIN_POS3, LOW);
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
  io_initialization();
  display_initialization();
  wifi_initialization();
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
  void handle_errors(void)
  {
    wifi_reconnect(); // tentativa de reconectar wifi
    // mqtt_reconnect(); //tentativa de reconectar mqtt
  }

  void setup()
  {
    Serial.begin(115200);
    system_initialization();
  } // end setup

  void loop()
  {
    handle_timers();

    if (is_1s)
    {
      is_1s = 0;
      handle_errors();
    }
  } // end loop
