// mqtt.ino - sending potmeter readings to an MQTT broker

// We connected a potmeter to GND,A0,3V3
// We are using test.mosquitto.org as mqtt broker
// We are using https://play.google.com/store/apps/details?id=snr.lab.iotmqttpanel.prod as client
//   In the app create a dashboard (connection name is up to you, client id is left blank, broker is test.mosquitto.org, port is 1883, network is TCP)
//   Add eg a gauge and fill out MCPmqtt1POT (see #define below) as topic name
//   Add eg a text log and fill out MCPmqtt1CNT (see #define below) as topic name, single payload, digital display
//   Add eg a button and fill out MCPmqtt1BUT (see #define below) as topic name, and red as payload, make the button color red

// Potmeter sends value to Nano ESP32, which sends it to MQTT broker, which sends it to iotmqttpanel on Android phone
// Android phone sends color via iotmqttpanel to MQTT broker to ESP32, which sets RGB led


// LED =====================================================


void led_single_set( int on ) {
  digitalWrite(LED_BUILTIN, on); // Hi-active
}


#define LED_RGB_OFF   0
#define LED_RGB_RED   1
#define LED_RGB_GREEN 2
#define LED_RGB_BLUE  3


void led_rgb_set( int col) {
  digitalWrite(LED_RED  , col!=LED_RGB_RED   ); // Lo-active
  digitalWrite(LED_GREEN, col!=LED_RGB_GREEN ); // Lo-active
  digitalWrite(LED_BLUE , col!=LED_RGB_BLUE  ); // Lo-active
}


void led_init() {
  pinMode(LED_BUILTIN, OUTPUT);
  led_single_set(0);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  led_rgb_set(LED_RGB_OFF);

  Serial.printf("led  : init\n");
}


// WiFi =====================================================


#include <WiFi.h>


#define wifi_ssid "GuestFamPennings"
#define wifi_pswd "there_is_no_password"


void wifi_init() {
  Serial.printf("wifi : .");
  WiFi.begin( wifi_ssid, wifi_pswd );
  while( WiFi.status()!=WL_CONNECTED ) {
    delay(500);
    Serial.printf(".");
  }
  Serial.printf( " %s\n", WiFi.localIP().toString().c_str());
}


// MQTT =====================================================


#include <ArduinoMqttClient.h> // Install via Arduino Library manager
WiFiClient mqtt_wifi;
MqttClient mqtt_client(mqtt_wifi);

#define mqtt_broker    "test.mosquitto.org"
#define mqtt_port      1883           // MQTT, unencrypted, unauthenticated

// Random values for key, use these to subscribe in the Android app dashboard
#define mqtt_topic_msg "MCPmqtt1MSG"  // key for textual message (ESP->phone)
#define mqtt_topic_pot "MCPmqtt1POT"  // key for potmeter value (ESP->phone)
#define mqtt_topic_cnt "MCPmqtt1CNT"  // key for counter value (ESP->phone)
#define mqtt_topic_but "MCPmqtt1BUT"  // key for color name (phone->ESP)


// Called when incoming message is received
void mqtt_onmessage(int size) {
  Serial.printf("rx '%s' %d - ", mqtt_client.messageTopic().c_str(), size );
  int len = 32;
  char msg[len];
  if( mqtt_client.messageTopic() != mqtt_topic_but ) { Serial.printf(" wrong topic\n"); return; }
  if( size>=len ) { Serial.printf("message len (%d) must be below %d\n",size, len ); return; }
  mqtt_client.read( (uint8_t*)msg,size);
  msg[size]='\0';
  Serial.printf("'%s'\n",msg);
  if( strcmp(msg,"red"  )==0 ) led_rgb_set(LED_RGB_RED);
  else if( strcmp(msg,"green")==0 ) led_rgb_set(LED_RGB_GREEN);
  else if( strcmp(msg,"blue" )==0 ) led_rgb_set(LED_RGB_BLUE);
  else led_rgb_set(LED_RGB_OFF);
}


void mqtt_init() {
  // mqtt_client.setUsernamePassword("user", "password");
  Serial.printf("mqtt : ... %s:%d ",mqtt_broker, mqtt_port);
  while( !mqtt_client.connect(mqtt_broker, mqtt_port) ) {
    Serial.printf("[%s] ", mqtt_client.connectError() );
  }
  // Also subscribe to incoming messages (only one topic)
  mqtt_client.onMessage(mqtt_onmessage);
  mqtt_client.subscribe(mqtt_topic_but);
  // Done
  Serial.printf("\n");
}


void mqtt_send(const char * topic, int value) {
  mqtt_client.beginMessage(topic);
  mqtt_client.print(value);
  mqtt_client.endMessage();
}


void mqtt_text_send(const char * topic, int pot, int cnt) {
  mqtt_client.beginMessage(topic);
  mqtt_client.print("pot: "); mqtt_client.println(pot);
  mqtt_client.print("cnt: "); mqtt_client.println(cnt);
  mqtt_client.endMessage();
}


// App =====================================================


void setup() {
  // Setup Serial
  Serial.begin(115200);
  delay(1500);
  Serial.printf("\n\nPotmeter MQTT test 2\n\n");

  led_init();
  led_rgb_set(LED_RGB_RED); // RED: booted
  wifi_init();
  led_rgb_set(LED_RGB_BLUE); // BLUE: WiFi connected
  mqtt_init();
  led_rgb_set(LED_RGB_GREEN); // GREEN: WiFi & MQTT connected

  delay(2000);
  Serial.printf("\n");
  led_rgb_set(LED_RGB_OFF); // OFF: running
}


void loop() {
  static uint32_t old_time;
  static int      old_pot;
  static int      cnt;

  mqtt_client.poll(); // checks incoming messages
  uint32_t time = millis();
  int pot = ( analogRead(A0) + analogRead(A0) + analogRead(A0) + analogRead(A0) ) / 4;
  if( abs(pot-old_pot) > 50 || time-old_time>60000 ) {
    Serial.printf("pot %d cnt %d\n",pot,cnt);
    mqtt_text_send(mqtt_topic_msg,pot,cnt) ;
    mqtt_send(mqtt_topic_pot,pot);
    mqtt_send(mqtt_topic_cnt,cnt);
    old_pot = pot;
    old_time = time;
    cnt++;
    led_single_set(cnt%2); // Blink single led
  }
}
