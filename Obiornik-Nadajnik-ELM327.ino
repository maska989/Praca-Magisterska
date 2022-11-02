#include "BluetoothSerial.h"
#include "ELMduino.h"
#include <esp_now.h>
#include <WiFi.h>


BluetoothSerial SerialBT;
#define ELM_PORT   SerialBT
#define DEBUG_PORT Serial

typedef enum { ENG_RPM,COOLANT_TEMP,ENGINE_LOAD_PER,TIMING,SPEED } obd_pid_states;
typedef struct struct_message { char PaySend[150]; } struct_message;
uint8_t address[6]  = {0x01, 0x23, 0x45, 0x67, 0x89, 0xBA};
String payload  = "";
uint8_t broadcastAddress[] = {0x84, 0xF3, 0xEB, 0x89, 0xB1, 0xED};
uint8_t broadcastAddressBalckBox[] = {0x84, 0xF3, 0xEB, 0x59, 0xE9, 0x07};
int iter = 0;


float rpm = 0;
String rpm_catcher = "";
bool rpmget = false;

float coolant = 0;
String coolant_catcher = "";
bool coolantget = false;

float eload = 0;
String eload_catcher = "";
bool eloadget = false;

float timing = 0;
String timing_catcher = "";
bool timingget = false;

float speed = 0;
String speed_catcher = "";
bool speedget = false;

ELM327 myELM327;
obd_pid_states obd_state = ENG_RPM;
struct_message myData;
esp_now_peer_info_t peerInfo;


void(* resetFunc)(void) = 0;




void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivered" : "Delivery Fail");
}

void setup()
{
  DEBUG_PORT.begin(115200);

  ELM_PORT.begin("ArduHUD", true);

  SerialBT.setPin("1234");

    WiFi.mode(WIFI_STA);
  if(esp_now_init() != ESP_OK){
    Serial.println("ESP CONNECTION ERROR (ESP_NOW)");
    return;
  }
  
  esp_now_register_send_cb(OnDataSent);
  
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
    memcpy(peerInfo.peer_addr, broadcastAddressBalckBox, 6);

  if(esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Peer Error");
    return;
  }
  
   memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  
  if(esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Peer Error");
    return;
  }
  
  if (!ELM_PORT.connect(address))
  {
    DEBUG_PORT.println("Connection error ELM327 Phase 1");
    resetFunc();
    while(1);
  }

  if (!myELM327.begin(ELM_PORT))
  {
    Serial.println("Connection error ELM327 Phase 2");
    resetFunc();
    while (1);
  }

  Serial.println("Connected to ELM327");
}




void ResetMacro(){
  payload="";
  rpmget = false;
  coolantget = false;
  eloadget = false;
  timingget = false;
  speedget = false;

  rpm_catcher = "";
  coolant_catcher = "";
  eload_catcher = "";
  timing_catcher = "";
  speed_catcher = "";
}



void loop()
{
  

  digitalWrite(LED_BUILTIN, HIGH);  
  
  StateCaseSwitch();

  /////////// Sekcja Data Transmit ESP-NOW


    if(rpmget==true && coolantget == true && eloadget == true && timingget == true && speedget == true){
      char Buff[150];
      payload = rpm_catcher + "/" + coolant_catcher + "/" + eload_catcher + "/" + timing_catcher + "/" + speed_catcher + "/";
      payload.toCharArray(Buff,150);
      strcpy(myData.PaySend,Buff);
      Serial.println(payload);
      esp_err_t result = esp_now_send(NULL, (uint8_t *) &myData, sizeof(myData));
      delay(200);
      if (result == ESP_OK) {
        Serial.println("success");
        digitalWrite(LED_BUILTIN, LOW);
        ResetMacro();
        //delay(500);
      }
        
      else {
        Serial.println("Error ESP data");
        digitalWrite(LED_BUILTIN, LOW); 
      }
    }
  
}




//////////// Sekcja PID OBD


void StateCaseSwitch(){
  switch (obd_state)
  {
    case ENG_RPM:
    {
      rpm = myELM327.rpm();
      
      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        Serial.print("rpm: ");
        Serial.println(rpm);
        rpm_catcher = String(rpm,2);
        rpmget = true;
        obd_state = COOLANT_TEMP;
      }

      
      break;
    }
    
    case COOLANT_TEMP:
    {
      coolant = myELM327.engineCoolantTemp();
      
      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        Serial.print("Coolant: ");
        Serial.print(coolant);
        Serial.println("*");
        coolant_catcher = String(coolant,2);
        coolantget = true;
        obd_state = ENGINE_LOAD_PER;
      }

      break;
    }

        case ENGINE_LOAD_PER:
    {
      eload = myELM327.engineLoad();
      
      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        Serial.print("EngineLoad: ");
        Serial.print(eload);
        Serial.println("%");
        eload_catcher = String(eload,2);
        eloadget = true;
        obd_state = TIMING;
      }

      
      break;
    }

        case TIMING:
    {
      timing = myELM327.timingAdvance();
      
      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        Serial.print("TA: ");
        Serial.print(timing);
        Serial.println("*");
        timing_catcher = String(timing,2);
        timingget = true;
        obd_state = SPEED;
      }
      break;
    }

    case SPEED:
    {
      speed = myELM327.kph();
      
      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        Serial.print("Speed: ");
        Serial.print(speed);
        Serial.println("km/h");
        Serial.println("");
        speed_catcher = String(speed,2);
        speedget = true;
        obd_state = ENG_RPM;
      }
      break;
    }
  }
}
