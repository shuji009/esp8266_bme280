/////https://thingspeak.com/
////C:\Users\Shuji\Documents\Arduino\weatherreffer
#include <ESP8266WiFi.h>
#include <Wire.h>
///#include <SPI.h>

///https://github.com/embeddedadventures/BME280
#include <BME280_MOD-1022.h>

extern "C" {
  #include "user_interface.h"
}

const char* ssid     = "************";		// myRouter SSID
const char* password = "************";		// myRouter Password
const char* host = "api.thingspeak.com";	// thingspeak URL
const char* thingspeak_key = "**********";	// thingspeak Write KEY

void setup() {	// not use

}
 
void loop() {

  Serial.begin(115200);
  delay(50);  ////<---- 100
  Serial.println("Start");
  pinMode(2, OUTPUT);  ///IO2
  digitalWrite(2, HIGH); 
  delay(10);  ////<---- 100
  
 //  Serial.begin(74880);
  Wire.begin(13, 14); 
  delay(10);             // SDA=GPIO_13,SCL=GPIO_14  Wire.begin(); 
///  delay(10);             // SDA=GPIO_13,SCL=GPIO_14
///  delay(20);             // SDA=GPIO_13,SCL=GPIO_14
 
  BME280.readCompensationParams();           // read the NVM compensation parameters
  BME280.writeOversamplingTemperature(os1x); // 1x over sampling
  BME280.writeOversamplingHumidity(os1x);    // 1x over sampling
  BME280.writeOversamplingPressure(os1x);    // 1x over sampling

  float volt  = system_adc_read()*11.0/1024.0;	/// R:10R ----> 1/11
  String vol = String(volt);    //

  BME280.writeMode(smForced); // After taking the measurement the chip goes back to sleep
  while (BME280.isMeasuring()){
    Serial.println("Measuring...");
    delay(10);  // <----- 50
  }
  Serial.println("Done!");
  BME280.readMeasurements();                     // read out the data

  float tmp = BME280.getTemperature();
  float hmd = BME280.getHumidity();
  float prs = BME280.getPressure();

  String temp = String(tmp);    // Temp
  String humi = String(hmd);    // Humidity
  String pres = String(prs);    // Pressure

  if(tmp>60.0 || hmd > 99.9 || hmd == 0 || prs>1190.0 || prs < 750.0) { ///異常値オミット
    String mg = "";
    mg += thingspeak_key;                         // thingspeak Write KEY
    mg += " temp =";mg += temp;                 // Temp
    mg += " humd =";mg += humi;                 // Humidity
    mg += " pres =";mg += pres;                 // Pressure  
    
    Serial.print("data : ");Serial.println(mg);
    Serial.println("Oh! No!! reset !");
    delay(100);  //
    pinMode(16, OUTPUT);  ///
    digitalWrite(16, LOW); /// reset
  }
  
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password); 

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
   WiFi.config(IPAddress(192, 168, 0, 25), WiFi.gatewayIP(), WiFi.subnetMask());
//// */
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("connecting to ");
  Serial.println(host);
  WiFiClient client;      // Use WiFiClient class to create TCP connections
  const int httpPort = 80;
  if (!client.connect(host,httpPort)){
    Serial.println("connection failed");
    ///return;  <---bug
    delay(100);  //
    pinMode(16, OUTPUT);  ///
    digitalWrite(16, LOW); /// reset
  }
   
  String url = "/update?key=";
  url += thingspeak_key;                         // thingspeak Write KEY
  url += "&field1=";url += temp;                 // Temp
  url += "&field2=";url += humi;                 // Humidity
  url += "&field3=";url += pres;                 // Pressure  
  url += "&field4=";url += vol;                  // voltage  
  url += "&field5=";url += "1";                  // counter  
  Serial.print("Requesting URL: ");Serial.println(url);
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" +  
               "Connection: close\r\n\r\n");
  delay(10); 
  while(client.available()){                     // Read all the lines of the reply
    String line = client.readStringUntil('\r'); 
    Serial.print(line);
  }
  Serial.println();
  Serial.println("closing connection. going to sleep...");
  digitalWrite(2, LOW); 
  delay(10);

  ESP.deepSleep(10 * 60 * 1000 * 1000 , WAKE_RF_DEFAULT);	//10 minute(s)  /// usec = uint_32
  ///ESP.deepSleep(1 * 30 * 1000 * 1000 , WAKE_RF_DEFAULT);	//30sec test
  ///ESP.deepSleep(1 * 60 * 1000 * 1000 , WAKE_RF_DEFAULT);	//1min. test

}

