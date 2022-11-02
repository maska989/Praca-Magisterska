#include <espnow.h>
#include <ESP8266WiFi.h>
#include "Nextion.h"

typedef struct struct_message { char PaySend[50]; } struct_message;

String Payload = "";
char *strings[6];
char *ptr = NULL;

float rpm = 0.0;
float coolant = 0.0;
float eload = 0.0;
float timing = 0.0;
float speedval = 0.0;
int rpmpb = 0;

NexPage page0 = NexPage(0,0, "page0");
NexPage page1 = NexPage(1,0, "page1");
NexNumber Obroty = NexNumber(1,2,"x0");
NexNumber Temp = NexNumber(1,3,"x1");
NexNumber Obci = NexNumber(1,4,"x2");
NexNumber Kat = NexNumber(1,5,"x3");
NexNumber KMH = NexNumber(2,5,"kmh");
NexProgressBar pb1  = NexProgressBar(2, 3, "j0");
NexProgressBar pb2  = NexProgressBar(2, 3, "j1");





struct_message myData;

void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&myData, incomingData, sizeof(myData));
  char index = 0; //zmienić na byte
  ptr = strtok(myData.PaySend,"/");
  while (ptr != NULL){
    strings[index] = ptr;
    index++;
    ptr = strtok(NULL,"/");
  }
  index = 0;
    rpm = atof(strings[0]);
  coolant = atof(strings[1]);
  eload = atof(strings[2]);
  timing = atof(strings[3]);
  speedval = atof(strings[4]);
  delay(2000);
  Obroty.setValue(rpm);
  Temp.setValue(coolant);
  Kat.setValue(timing*10);
  Obci.setValue(eload);
  KMH.setValue(speedval);

  rpmpb = rpm/80;
  pb1.setValue(rpmpb);
  pb2.setValue(coolant);

  if(rpmpb < 35){
    pb1.Set_font_color_pco(7489);
  }
  else if(rpmpb<75){
    pb1.Set_font_color_pco(43840);
  }
  else{
    pb1.Set_font_color_pco(43136);
  }

    if(coolant<90){
    pb2.Set_font_color_pco(7489);
  }
  else{
    pb2.Set_font_color_pco(43136);
  }
  


}


 void smartSend()
 {
      Serial.write(0xff); 
      Serial.write(0xff);   
      Serial.write(0xff);
 }

void setup() {
  Serial.begin(115200);
  nexInit(115200);
  WiFi.mode(WIFI_STA);
  if(esp_now_init() != 0){
    Serial.println("Error ESP-NOW");
    return;
  }
  
  page1.show();
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  


}
