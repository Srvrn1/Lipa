#include <Arduino.h>
#include <GyverHub.h>

#define led 2

///////////////=====================================
uint32_t time_sist;         //время системы
uint32_t  t_on;             //время вкл
uint32_t  t_off;            //время выкл
uint8_t sw_stat;            //положение переключателя

//////////////======================================

GyverHub hub("MyDev", "Липовка", "f0ad");  // имя сети, имя устройства, иконка
WiFiClient espClient;

///   WI-FI  ///////////
//const char* ssid = "RT-WIFI-0FBE";
//const char* password = "YeNu5VAyeY";
const char* ssid = "srvrn";
const char* password = "2155791975";

//   MQTT  /////////////
const char* mqtt_server = "m4.wqtt.ru";
const int mqtt_port = 9478;
const char* mqtt_user = "u_5A3C2X";
const char* mqtt_password = "HilZPRjD";

void onunix(uint32_t stamp) {                //получаем дату время. ра6отает!!!
    time_sist = (stamp + 32400) % 86400;    //получаем только время и корректируем часовой пояс
}

void setup_wifi() {

  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void sw_f(){                      //функция вкл-выкл диода
  digitalWrite(led, !sw_stat);
}

void build(gh::Builder& b){
  b.Time_(F("time"), &time_sist).label(F("время")).color(gh::Colors::Mint);
  b.Display("Версия  1.2");
  if(b.beginRow()){
    //b.Time_(F("time"), &time_sist).label(F("время")).color(gh::Colors::Mint);
    b.Time_(F("t_on"), &t_on).label(F("включить")).color(gh::Colors::Red).click();
    b.Time_(F("t_off"), &t_off).label(F("выкл")).color(gh::Colors::Green);
    b.Switch_(F("Swit"), &sw_stat).label(F("включатель")).attach(sw_f);
    b.endRow();
  }

}

void setup(){
  Serial.begin(74880);
  Serial.println("");
  Serial.println("Hello");
  Serial.println("ПОЕХАЛИ!");

  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  
  setup_wifi();

  hub.mqtt.config(mqtt_server, mqtt_port, mqtt_user, mqtt_password);
  hub.setVersion("Srvrn1/Lipa@1.2");
  hub.onUnix(onunix);
  hub.onBuild(build);               // подключаем билдер
  hub.begin();   
  
}

void loop(){
  hub.tick();

  static GH::Timer tmr(1000);                    //запускаем таймер
  if(tmr){
    time_sist++;
    if(time_sist >= 86400) time_sist = 0;                   //время в Unix формате с6расываем в 00 часов
    hub.sendUpdate(F("time"));

    if(time_sist == t_on){                                 //6удильник включаем Switch
      sw_stat = 1;
      sw_f();
      hub.sendUpdate(F("Swit"));
    }
    if(time_sist == t_off){
      sw_stat = 0;
      sw_f();
      hub.sendUpdate(F("Swit"));
    }
  }

}