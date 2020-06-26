#define SENSORPIN 0
#define ACTUATORPIN 4

//Constant definitions
const unsigned int NUMREADS = 12;  //Samples to average for smoothing

double consKp = 0.4, consKi = 0.2, consKd = 0.0; //Controller constants

//Variable definitions
double sp =0, y = 0, out = 0; //System Variables

//Variable definitions
unsigned int val = 0;
//Smoothing vars (filter electronic noise)
unsigned int readings[NUMREADS] = {0};
unsigned int readIndex = 0;
unsigned int total = 0;
unsigned int outputValor = 0;

//si usas esp32
//#include <HTTPClient.h>
//#include <WiFi.h>

//si usas esp8266
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.hpp>
#include <ArduinoJson.h>
#include <PID_v1.h>



const char* ssid = "*******";
const char* password = "********";
const String url="http://*****:8080/Control/dataVisualization";
const String projectName="example2";

//Library definitions
PID myPID(&y, &out, &sp, consKp, consKi, consKd, DIRECT);


//Subroutines and functions
unsigned int smooth() { //Recursive moving average subroutine
  total = total - readings[readIndex]; // subtract the last reading
  readings[readIndex] = analogRead(SENSORPIN); // read from the sensor:
  total = total + readings[readIndex]; // add the reading to the total:
  readIndex = readIndex + 1; // advance to the next position in the array:
  if (readIndex >= NUMREADS) {// if we're at the end of the array...
    readIndex = 0; // ...wrap around to the beginning:
  }
  return total / NUMREADS; // calculate the average:
}
void WiFiInit() {
   Serial.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED) { //Check for the connection
    delay(500);
    Serial.print(".");
  }
  Serial.print("Successfully connected, IP adress: ");
  Serial.println(WiFi.localIP());
}


void setup() {
  delay(10);
  Serial.begin(115200);
  
  pinMode(ACTUATORPIN, OUTPUT);
  digitalWrite(ACTUATORPIN, LOW);
  myPID.SetMode(AUTOMATIC);
  
  WiFi.begin(ssid, password);
  WiFiInit();//WiFi communications initialization
}
void postService() {
    HTTPClient http;
    http.begin(url);  //Endpoint service

    String json;
    StaticJsonDocument<300> doc;
    doc["times"] = millis()/1000;
    doc["data"] = y;
    doc["set"] = sp;
    doc["name"] = "proyect2";
    serializeJson(doc, json);

    
    int codigo_respuesta = http.POST(json); // request
    
    if(codigo_respuesta>0){
      Serial.println("Code HTTP ► " + String(codigo_respuesta));   //Print return code
      if(codigo_respuesta == 200){   
        // server response 
        String cuerpo_respuesta = http.getString();
        sp=cuerpo_respuesta.toInt();
        Serial.println("Server response ▼ ");
        Serial.println(sp);     
      }

    }else{
    
     Serial.print("Error sending POST, code: ");
     Serial.println(codigo_respuesta);
    }
    http.end();  //freed resources
  }


void loop() {

  if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
    y = smooth(); 
    myPID.Compute();
    analogWrite(ACTUATORPIN, out );
    postService();
 
  }else{
     Serial.println("WIFI connection error");
  }

}
