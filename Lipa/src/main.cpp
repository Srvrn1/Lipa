

#include <Arduino.h>
#include <GyverHub.h>

#define led 2
#define press 1

///////////////=====================================
uint32_t time_sist;         //время системы
uint32_t  t_on;             //время вкл аквариум
uint32_t  t_off;            //время выкл аквариум
uint8_t sw_stat;            //положение перекл аквариума
uint8_t sw_press;           //положение переключателя компрессора
uint8_t sw_mg;              //положение переключателя в туалете
const char*  vers_mg = "0";             //версия прошивы туалетного контроллера

//////////////======================================

GyverHub hub("MyDev", "ДОМ", "f0ad");  // имя сети, имя устройства, иконка
WiFiClient espClient;

///   WI-FI  ///////////
//const char* ssid = "RT-WiFi-0FBE";
//const char* password = "YeNu5VAyeY";
//const char* ssid = "srvrn";
//const char* password = "2155791975";
const char* ssid = "Dnlvrn";
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
void sw_presss(){
  digitalWrite(press,!sw_press);
}

void build(gh::Builder& b){
  if(b.beginRow()){
  b.Time_(F("time"), &time_sist).label(F("время")).color(gh::Colors::Blue);
  b.Display(F("V1.5.9")).label(F("Releases")).color(gh::Colors::Blue);
  b.Display_(F("vers")).color(gh::Colors::Blue);             //сюда шлет свою версию прибор из туалета
  b.Button_(F("supd"));                                               //по нажатию, все удаленные устройства ищут обновы.
   b.endRow();
  }

  if(b.beginRow()){
    b.Time_(F("t_on"), &t_on).label(F("вкл")).color(gh::Colors::Red).click();
    b.Time_(F("t_off"), &t_off).label(F("выкл")).color(gh::Colors::Green);
    b.Switch_(F("Swit"), &sw_stat).label(F("акваСвет")).attach(sw_f);
    b.Switch_(F("Swit"), &sw_stat).label(F("компрессор")).attach(sw_presss);
    b.endRow();
  }
  if(b.beginRow()){
    b.Display_(F("hvs")).label(F("ХВС")).color(gh::Colors::Aqua);
    b.Display_(F("gvs")).label(F("ГВС")).color(gh::Colors::Orange);
    b.Switch_(F("mg"), &sw_mg).label(F("М/Ж")).color(gh::Colors::Red);
    b.endRow();
  }

}

void setup(){
 /* Serial.begin(74880);
  Serial.println("");
  Serial.println("Hello");
  Serial.println("версия 1.6");*/

  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);

  pinMode(press, OUTPUT);
  digitalWrite(press, LOW);
  
  setup_wifi();

  hub.mqtt.config(mqtt_server, mqtt_port, mqtt_user, mqtt_password);
  hub.setVersion("Srvrn1/Lipa@1.6.1");
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