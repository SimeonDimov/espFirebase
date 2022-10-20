#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include "time.h"
#include <SoftwareSerial.h>
#include <TinyGPS.h>
//gps setup
TinyGPS gps;
SoftwareSerial ss;

//For storing data as string
String gpsData = "";
char buff[10];
//GPS data
float flat, flon;
unsigned long age;

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Fontys"
#define WIFI_PASSWORD "fontys2019"

// Insert Firebase project API Key
#define API_KEY "AIzaSyA5pI9g1mfA30pWmI7O8lTAMkZA6-9PMAc"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "moni42@abv.bg"
#define USER_PASSWORD "123456"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://espmonitoring-a9cc6-default-rtdb.europe-west1.firebasedatabase.app/"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;
// Database child nodes
String noisePath = "/noise";
String timePath = "/timestamp";
String gpsPath = "/coordinates";


// Parent Node (to be updated in every loop)
String parentPath;

int timestamp;
FirebaseJson json;

const char* ntpServer = "pool.ntp.org";

const int potPin = 34;


// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 18000;



// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

bool noise;

String floatToString(float val, float invalid, int len, int prec) {
  String out = "";
  if (val == invalid) {
    while (len-- > 1){
      return "inv" ;
    }
  }
  else{
    for (int i = 0; i < 10; i++) {
       dtostrf(val, len, prec, buff);  
       out += buff;
       return out;
    }
  }
}
void setup(){
  Serial.begin(115200);
 ss.begin(9600, SWSERIAL_8N1, 4, 3, false);
  if(!ss){
    Serial.println("Invalid SoftwareSerial pin configuration, check config"); 
    while (1) { // Don't continue with invalid configuration
      delay (1000);
    }
  }
  initWiFi();
  configTime(0, 0, ntpServer);

  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/UsersData/" + uid + "/readings";
}

void loop(){
  

long sum = 0;
    for(int i=0; i<100; i++)
    {
       sum += analogRead(potPin);
    }
 
    sum = sum/100;

if(sum>60){
  noise = true;
}
else{
  noise = false;
}

// Function for converting gps float values to string



  // Send new readings to database
  if (Firebase.ready() && noise == true){

    //Get current timestamp
    timestamp = getTime();
    Serial.print ("time: ");
    Serial.println (timestamp);

    parentPath= databasePath + "/" + String(timestamp);

    json.set(noisePath.c_str(), String(noise));

    json.set(String(timePath), String(timestamp));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
  }
}
