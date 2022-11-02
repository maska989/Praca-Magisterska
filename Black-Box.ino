#include <espnow.h>
#include <ESP8266WiFi.h>

typedef struct struct_message { char PaySend[150]; } struct_message;

String Payload = "";
char *strings[6];
char *ptr = NULL;

float rpm = 0.0;
float coolant = 0.0;
float eload = 0.0;
float timing = 0.0;
float speedval = 0.0;
int rpmpb = 0;

struct_message myData;

void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.println(myData.PaySend);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if(esp_now_init() != 0){
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {

}
