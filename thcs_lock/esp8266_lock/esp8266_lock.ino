#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

// Thông tin về wifi
#define ssid "Hung"
#define password "hungnh06"
#define host "maker.ifttt.com"
#define apiKey "bhjFm_AotOcA3NOLIRti6k";

bool state = false;
String lockState;

//#define mqtt_server "broker.hivemq.com"
//#define mqtt_port 1883

#define mqtt_server IPAddress (203, 162, 10, 118)
#define mqtt_port 8800
#define mqttUser "IB12345"
#define mqttPass "12345"

#define topic1 "home/lock/unlocked"
#define topic2 "home/lock/locked"

WiFiClient espClient, espWifi;
PubSubClient client(espClient);

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  // Wait for serial port to connect. Needed for native USB port only
  while (!Serial) {;}
  
  // Connect to WIFI
  setup_wifi();
  // MQTT broker setting
  client.setServer(mqtt_server, mqtt_port);

  // Checking connection
  if (!client.connected())
  {
    reconnect();
  }
}

// Reconnect function
void reconnect()
{
  while (!client.connected())
  {
    if (client.connect("ESP8266", mqttUser, mqttPass)) // Connect to broker
    {
      Serial.println("Connected");
    }
    else
    {
      // Can't connect to broker
      Serial.print("Error:, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5s
      delay(5000);
    }
  }
}

// Wifi function
void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  // in ra thông báo đã kết nối và địa chỉ IP của ESP8266
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
// Run over and over
void loop()
{
  client.loop();
  if (Serial.available() > 0) 
  {
    // Read message from UNO board
    String msg = Serial.readString();
    
    StaticJsonDocument<200> doc;
    deserializeJson(doc, msg);
    char STATUS = doc["STATUS"];
    char PASS   = doc["PASS"];
    char METHOD = doc["METHOD"];

    String Status   = doc["STATUS"];
    String Pass   = doc["PASS"];
    String Method = doc["METHOD"];
    
    char buffer[100];
    serializeJson(doc, buffer);
    Serial.println(buffer);
    
    // Publish MSG to Topic
    if (Status == "Unlocked")
    {
      state = true;
      lockState = "unlocked";
      client.publish(topic1, buffer);
    }
    if (Status == "Lockout")
    {
      client.publish(topic1, buffer);      
    }
    if (Status == "Locked" && Pass == "Invalid")
    {
      client.publish(topic1, buffer);
    }
    if (Status == "Locked" && Method == "Auto")
    {
      state = true;
      lockState = "locked";
      client.publish(topic2, buffer);
    }
    
    if (!espWifi.connect(host, 80)) {
        Serial.println("connection failed");
        return;
    }  
    if(state = true)
    {
      state = false;
      String url = "/trigger/lock/with/key/";
      url += apiKey;

      Serial.print("Requesting URL: ");
      Serial.println(url);
      espWifi.print(String("POST ") + url + " HTTP/1.1\r\n" +
                          "Host: " + host + "\r\n" + 
                          "Content-Type: application/x-www-form-urlencoded\r\n" + 
                          "Content-Length: 13\r\n\r\n" +
                          "value1=" + lockState + "\r\n");        
    }    
  }
}













