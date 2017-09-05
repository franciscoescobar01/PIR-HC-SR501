#include <Arduino.h>

 #include <ESP8266WiFi.h>
 #include <ESP8266HTTPClient.h>
 #include <PubSubClient.h>
 #include <Ethernet.h>
 #include <SPI.h>


 //Constantes de coneccion a WiFi
 const char* ssid = "CEISUFRO";
 const char* password = "DCI.2016";

 //Constantes de servidor MQTT
 const char* mqtt_server = "ipame.cl";
 const char* mqtt_topic = "movimiento";


/* HC-SR501 Motion Detector */
#define ledPin D7
#define pirPin D1 // Input for HC-S501
int pirValue; // Place to store read PIR Value

void getPirValue(void)
{
  pirValue = digitalRead(pirPin);
  if (pirValue)
  {
    Serial.println("==> Motion detected");
  }else{
    Serial.println("==>NO Motion ndetected");
  }

}

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
int threhold=50;

void setup_wifi() {
   delay(100);
  // We start by connecting to a WiFi network
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
    }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length)
{
} //end callback

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    //if you MQTT broker has clientID,username and password
    //please change following line to    if (client.connect(clientId,userName,passWord))
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
     //once connected to MQTT broker, subscribe command if any
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 6 seconds before retrying
      delay(6000);
    }
  }
}


void setup()
{
  Serial.begin(115200);
  delay(10);

  pinMode(ledPin, OUTPUT);
  pinMode(pirPin, INPUT);
  digitalWrite(ledPin, LOW);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  Serial.println("Setup!");

}

void loop()
{
  getPirValue();
   delay(1000);
   Serial.println(pirValue);


   if (!client.connected()) {
     reconnect();
   }
   client.loop();
   long now = millis();
  //send data every second

   if (now - lastMsg > 6000) {
     int valor = pirValue;
     lastMsg = now;
     String msg="movimiento: "+pirValue;
     msg= msg+ pirValue;

     char message[58];
     msg.toCharArray(message,58);
     Serial.println(message);

     //publish sensor data to MQTT broker

      client.publish(mqtt_topic, message);


     lastMsg = now;

     msg=""+pirValue;

     msg.toCharArray(message,58);
     Serial.println(message);
      //publish sensor data to MQTT broker
     client.publish(mqtt_topic, message);
     //webservice datos unidad_medida_1=Temperatura+C&dato_1=90&unidad_medida_2=Humedad++%25&dato_2=55&codigo_sensor=Humedad+y+Temperatura&accion=agregar
     String data = "unidad_medida_1=Movimiento+C&dato_1=";
     data = data + pirValue;
     data = data + "&codigo_sensor=PIR&accion=agregar";

     if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status

     HTTPClient http;    //Declare object of class HTTPClient

     http.begin("http://praga.ceisufro.cl/ipame_dev/index.php?eID=obtenerDatos");
     http.addHeader("Content-Type", "application/x-www-form-urlencoded");

     int httpCode = http.POST(data);   //Send the request
     String payload = http.getString();                  //Get the response payload

     Serial.println(httpCode);   //Print HTTP return code
     Serial.println(payload);    //Print request response payload

     http.end();  //Close connection

     }else{

     Serial.println("Error in WiFi connection");
     }

     Serial.println(data);
     Serial.print("Response body from server: ");
     Serial.println(message);
   }
}
