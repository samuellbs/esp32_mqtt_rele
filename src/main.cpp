#include "shared_data.h"

void handle_timers(void)
{
  /*      Descrição de funcionamento:

         Definição de tempo de timers
  */
  unsigned long t_atual;
  t_atual = millis();                     // função milis retorna o tempo que o microcontrolador está ligado

  if (t_atual - millis_atual_100ms >= 99) // 100ms
  {
    millis_atual_100ms = t_atual;
    fs_100ms = 1;
  }
  if (t_atual - millis_atual_1s >= 999) // 1s
  {
    millis_atual_1s = t_atual;
    fs_1s = 1;
  }
  if (t_atual - millis_atual_60s >= 59999) // 60s
  {
    millis_atual_60s = t_atual;
    fs_60s = 1;
  }
}

void wifi_initialization(void)
{
  bool res;
  wm.setDebugOutput(false);                             //TRUE para verificar infos de conexão no monitor serial
  WiFi.mode(WIFI_STA);                                  //conectar no WiFi com dados salvos na memória flash

  if(is_first_initialization)
   {
    is_first_initialization = 0;
    EEPROM.write(FIRST_INIT_ADDRESS, is_first_initialization);
    EEPROM.commit();
    wm.resetSettings();       //sempre que ligar resetar WiFi
  }

  res = wm.autoConnect("ESP_SAMUEL","password"); // password protected ap
//
//   if(!res) {
//        Serial.println("Failed to connect");
//        // ESP.restart();
//   } 
//   else {
//        //if you get here you have connected to the WiFi    
//        Serial.println("connected...yeey :)");
//   }
  if(SERIAL){
  Serial.println("\n*************** WiFi INFORMATION ***************");
  Serial.println("SAVED:" + ((String)wm.getWiFiIsSaved()) ? "WiFi SAVED" : "WiFi NOT SAVED");
  Serial.println("SSID: " + (String)wm.getWiFiSSID());
  Serial.println("PASS: " + (String)wm.getWiFiPass());
  Serial.println("************************************************");
  }
}

char io_initialization(void)
{
  char ret = -1;
  pinMode(PIN_FEED3, INPUT );
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED,   OUTPUT);
  pinMode(PIN_NEG3,  OUTPUT);
  pinMode(PIN_POS3,  OUTPUT);

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
  io_initialization();
  display_initialization();
  wifi_initialization();

}
void setup()
{
  Serial.begin(115200);
  system_initialization();
} //end setup

void loop()
{
  handle_timers();
} //end loop

