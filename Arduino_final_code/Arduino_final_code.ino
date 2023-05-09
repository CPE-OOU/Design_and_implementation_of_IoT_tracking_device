#define TINY_GSM_MODEM_SIM800
#define TINY_GSM_RX_BUFFER 256

#include <TinyGPS++.h> 
#include <TinyGsmClient.h> 
#include <ArduinoHttpClient.h>


const char FIREBASE_HOST[]  = ""; // get url from firebase
const String FIREBASE_AUTH  = ""; // get secret code from firebase
const String FIREBASE_PATH  = "/"; 
const int SSL_PORT          = 443;

// Your GPRS credentials
// Leave empty, if missing user or pass
char apn[]  = "web.gprs.mtnnigeria.net";
char user[] = "";
char pass[] = "";

HardwareSerial sim800(2);
TinyGsm modem(sim800);


HardwareSerial neogps(1);
TinyGPSPlus gps;

TinyGsmClientSecure gsm_client_secure_modem(modem, 0);
HttpClient http_client = HttpClient(gsm_client_secure_modem, FIREBASE_HOST, SSL_PORT);

unsigned long previousMillis = 0;
long interval = 10000;

int Sms_pin = 23;

int Call_pin = 22;


double lati = 0.00;
double lngi = 0.00;
 String latitude1, longitude1;
 //String date, time;  

void setup() {
  Serial.begin(9600);
  Serial.println("Arduino serial initialize");
  
  sim800.begin(9600, SERIAL_8N1,16,17);
  Serial.println("SIM800L serial initialize");

  neogps.begin(9600, SERIAL_8N1, 2, 4);
  Serial.println("neogps serial initialize");
      
    delay(3000);

  pinMode (Sms_pin,INPUT_PULLUP);
  
  pinMode (Call_pin,INPUT_PULLUP);  
  
  Serial.println("Initializing modem...");
  modem.restart();
  String modemInfo = modem.getModemInfo();
  Serial.print("Modem: ");
  Serial.println(modemInfo);
  
  http_client.setHttpResponseTimeout(90 * 1000); //^0 secs timeout
}

void loop() {
 
  Serial.print(F("Connecting to "));
  Serial.print(apn);
  if (!modem.gprsConnect(apn, user, pass)) {
    Serial.println(" fail");
    delay(1000);
    return;
  }
  Serial.println(" OK");
    
  http_client.connect(FIREBASE_HOST, SSL_PORT);
  
    while (true) {
    if (!http_client.connected()) {
      Serial.println();
      http_client.stop();// Shutdown
      Serial.println("HTTP  not connect");
      break;
    }
    else{
      gps_loop();
    }
  }
  
}

void PostToFirebase(const char* method, const String & path , const String & data, HttpClient* http) {
  String response;
  int statusCode = 0;
  http->connectionKeepAlive(); // Currently, this is needed for HTTPS

  String url;
  if (path[0] != '/') {
    url = "/";
  }
  url += path + ".json";
  url += "?auth=" + FIREBASE_AUTH;
  Serial.print("POST:");
  Serial.println(url);
  Serial.print("Data:");
  Serial.println(data);
   
  String contentType = "application/json";
  http->put(url, contentType, data);
  
 
  statusCode = http->responseStatusCode();
  Serial.print("Status code: ");
  Serial.println(statusCode);
  response = http->responseBody();
  Serial.print("Response: ");
  Serial.println(response);
  
  if (!http->connected()) {
    Serial.println();
    http->stop();// Shutdown
    Serial.println("HTTP POST disconnected");
  }
  
}

void gps_loop()
{
  boolean newData = false;
  for (unsigned long start = millis(); millis() - start < 2000;){
    while (neogps.available()){
      if (gps.encode(neogps.read())){
        newData = true;

      delay(500);

     int sms = digitalRead(Sms_pin); 
     int call = digitalRead(Call_pin);
        
        if ( sms == 0) {
        send_SMS();
              
      }   

        //delay(1000);  
            
      if (call == 0) {
        Call();
          
      }         
        break;
      }
    }
  }
  
  if(true){
  newData = false;
  
  String latitude, longitude;
  //float altitude;
  //unsigned long date, time, speed, satellites;
  
  Serial.print(F("Location: "));
  if (gps.location.isValid()){
    Serial.println(gps.location.lat(), 6);
    lati= gps.location.lat(), 6;
    Serial.print(F(","));
    Serial.println(gps.location.lng(), 6);
    lngi= gps.location.lng(), 6;
  }
  
  latitude = String(gps.location.lat(), 6); // Latitude in degrees (double)
  longitude = String(gps.location.lng(), 6); // Longitude in degrees (double)
  
  //altitude = gps.altitude.meters(); // Altitude in meters (double)
  //date = String(gps.date.value()); // Raw date in DDMMYY format (u32)
  //time = String(gps.time.value()); // Raw time in HHMMSSCC format (u32)
  //speed = gps.speed.kmph();
  
  Serial.print("Latitude= "); 
  Serial.print(latitude);  
  Serial.print(" Longitude= "); 
  Serial.println(longitude);

 // Serial.print("date= "); 
  //Serial.print(date);
  //Serial.print("time= "); 
  //Serial.println(time);
        
        
  String gpsData = "{";
  gpsData += "\"lat\":" + latitude + ",";
  gpsData += "\"lng\":" + longitude + "";
  //gpsData += "\"date\":" + date + "";
  //gpsData += "\"time\":" + time + ",";  
  //gpsData += "\"link\":\"http://maps.google.com/maps?q=loc:" + String(latitude, 6) + "," + String(longitude, 6) + "\"";
  gpsData += "}";

  //PUT   Write or replace data to a defined path, like messages/users/user1/<data>
  //PATCH   Update some of the keys for a defined path without replacing all of the data.
  //POST  Add to a list of data in our Firebase database. Every time we send a POST request, the Firebase client generates a unique key, like messages/users/<unique-id>/<data>
  //https://firebase.google.com/docs/database/rest/save-data
  
  PostToFirebase("PATCH", FIREBASE_PATH, gpsData, &http_client);
  

  }
}

void updateSerial(){
  delay(500);
  while (Serial.available())  {
    neogps.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while (neogps.available())  {
    Serial.write(neogps.read());//Forward what Software Serial received to Serial Port
  }
}


void updateSerialsim800()
{
  delay(100);
  //Serial.println("PREPARING up");
  while (Serial.available())
  {
    sim800.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while (sim800.available())
  {
    Serial.write(sim800.read());//Forward what Software Serial received to Serial Port
  }
}


void Call(){

  sim800.println("AT"); //Once the handshake test is successful, i t will back to OK
  updateSerialsim800();
  Serial.println("Calling");
  sim800.println("ATD+ +2347036074909;");    
  updateSerialsim800();
  delay(20000); // wait for 20 seconds...
  sim800.println("ATH"); //hang up
  updateSerialsim800();
  
}

void send_SMS()
{
  sim800.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerialsim800();
  sim800.println("AT+CMGS=\"+2347036074909\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms
  updateSerialsim800();
  sim800.print("I need help please " );  
  sim800.print("http://maps.google.com/maps?q=loc:" ); //text content
  sim800.print((gps.location.lat()), 6 ); //text content
  sim800.print("," ); //text content
  sim800.print(( gps.location.lng()), 6 ); //text content
  
  updateSerialsim800();
  
  Serial.println();
  Serial.println("Message Sent");
  sim800.write(26);
  Serial.println(" Sent");
  delay(1000);
  sim800.begin(9600, SERIAL_8N1,16,17);
delay(2000);
}
