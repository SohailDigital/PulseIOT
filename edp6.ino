#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30100_PulseOximeter.h"
#include <WiFi.h>
#include <HTTPClient.h>

const char* name = "droidd";
const char* pwd  =  "hitler87";

String SCRIPT_URL="https://script.google.com/macros/s/AKfycbzeRXdcjWaBGkUVP_5A25zD4V7eLIFZKQcK5hahYi2dwGqs_5lpP7uJtS1C0cevsMmqQQ/exec?action=create&";
  
// Define constants
#define NTC_PIN A0
#define VD_PIN 34
#define VREF 3300 // in millivolts
#define R_DIV 9000 // in ohms
#define NOMINAL_TEMP 25 // in degrees Celsius
#define NOMINAL_RES 10000 // in ohms
#define B_VALUE 3425



// OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// MAX30100 sensor settings
#define REPORTING_PERIOD_MS 1000
PulseOximeter pox;

//avg
const int size=30;
 int ind=0;
int HRarray[size];
int logValHr=0;
float logValTemp=0.00f,Temarray[size];
bool last_data_sent_status;

int currentHR=0;

void setup() {
  pinMode(NTC_PIN,INPUT);
  Serial.begin(9600);
   pinMode(VD_PIN, OUTPUT); // Set voltage divider pin as output

  // OLED display setup
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  display.clearDisplay();                     // clear the display buffer
  display.setTextSize(1);                     // set the text size to 1
  display.setTextColor(SSD1306_WHITE);        // set the text color to white


    display.setCursor(0, 0);
    display.print(F("connecting to:"));
    display.setCursor(0, 13);
    display.print(F(name));
    display.display();
   
    WiFi.begin(name, pwd);

  while (WiFi.status() != WL_CONNECTED) {
     delay(500);
    Serial.print(".");
    Serial.println(WiFi.status());
   
  }
       
       Serial.println("out of while loop");
 
  display.clearDisplay(); 
  display.setCursor(0, 0);
  display.print(F("connected to:"));
    display.setCursor(0, 13);
    display.print(F(name));
    display.display();
  delay(3000); 
  
  display.clearDisplay();
    
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.setTextSize(1);
    display.print(F("Connected"));
    display.display();

  // MAX30100 sensor setup
  if(!pox.begin()) {
    Serial.println(F("MAX30100 sensor not found"));
    for(;;);
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_27_1MA);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(36,16);
  display.print(F("BPM"));

  display.setCursor(122,16);
  display.print(F("2"));

  display.setCursor(0,24);
  display.print(F("Temperature:"));

  display.setTextSize(2);
  display.setCursor(110,8);
  display.print(F("O"));

  display.display();

}

void loop() {
  
//wifi status-----
  if (WiFi.status() != WL_CONNECTED) {
    display.setTextColor(BLACK);
    display.fillRect(0, 0, 120, 8,BLACK);
    display.display();

    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.setTextSize(1);
    display.print(F("Connecting...."));
    display.display();

    
    while(WiFi.status() != WL_CONNECTED){
      WiFi.reconnect();
      delay(700);
    }
    display.setTextColor(BLACK);
    display.fillRect(0, 0, 120, 8,BLACK);
    display.display();
    
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.setTextSize(1);
    display.print(F("Connected"));
    display.display();
  }
  
  //----------update...later
  if(last_data_sent_status){
    last_data_sent_status=false;
  }else{
    last_data_sent_status=true;
  }
  //---------------

  static uint32_t lastReportTime = 0;
  // Serial.print(F("Heart rate: "));

  pox.update();
  currentHR=0;

  if (millis() - lastReportTime > REPORTING_PERIOD_MS) {

    lastReportTime = millis();
    currentHR=pox.getHeartRate();

    

    if (currentHR != 0) {

      
      //temperature---------
      // Read analog input from NTC thermistor
        int raw_adc = analogRead(NTC_PIN);
        // Serial.println(raw_adc);

        // Calculate voltage across thermistor
        float thermistor_voltage = raw_adc * (VREF / 4095.0);

        // Calculate resistance of thermistor using voltage divider formula
        float thermistor_resistance = R_DIV * ((VREF / thermistor_voltage) - 1.0);

        // Calculate temperature using Steinhart-Hart equation
        float steinhart;
        steinhart = thermistor_resistance / NOMINAL_RES; // (R/Ro)
        steinhart = log(steinhart); // ln(R/Ro)
        steinhart=steinhart / B_VALUE; // 1/B * ln(R/Ro)
        steinhart = steinhart +1.0 / (NOMINAL_TEMP + 273.15); // + (1/To)
        steinhart = 1.0 / steinhart; // Invert
        steinhart = steinhart-273.15; // Convert to Celsius

        
      updateReadings(currentHR, pox.getSpO2(), steinhart );
      HRarray[ind]=currentHR;
      Temarray[ind++]=steinhart;
      if(ind==size-1){
        ind=0;
        for(int i=0;i<size;i++){
          logValHr=logValHr+HRarray[i];
          logValTemp=logValTemp+Temarray[i];
          Serial.print(Temarray[i]);
          Serial.print("  ");
          Serial.println(logValTemp);

          HRarray[i]=0;
          Temarray[i]=0;
        }
        logValHr=logValHr/size;
        Serial.println(logValTemp);
        logValTemp=logValTemp/size;
        Serial.println(logValTemp);
        sendData( logValHr,logValTemp,pox.getSpO2() );
        // last_data_sent_status=sendData( logValHr,logValTemp,pox.getSpO2() );
        if(!pox.begin()) {
          Serial.println(F("MAX30100 sensor not found"));
          Serial.println("Restarting");
           ESP.restart();
        }


      }

      //Serailprint

            Serial.print("Temperature: ");
            Serial.print(steinhart);
            Serial.println(" C");
            
            Serial.print(F("Heart rate: "));
            Serial.print(currentHR);
            Serial.println(F(" BPM"));
          
            Serial.print(F("SpO2: "));
            Serial.print(pox.getSpO2());
            Serial.println(F(" %"));
        
    }else{
        display.setTextColor(BLACK);
        display.fillRect(65, 8, 36, 16, BLACK);
        display.fillRect(0, 8, 36, 16, BLACK);
        display.display();

      }
    }
}

void updateReadings(int pulse, int spo2 ,float temperature){
  display.setTextColor(BLACK);
  display.fillRect(65, 8, 36, 16, BLACK);
  display.fillRect(0, 8, 36, 16, BLACK);
  display.fillRect(75,24,29,8,BLACK);
  display.display();
  display.setTextColor(WHITE);
  display.setTextSize(2);
   display.setCursor(65,8);//spo2
  display.print(spo2);
  display.setCursor(0,8);//bpm
  display.print(pulse);
  display.setCursor(91, 8);
  display.print("%");
  display.setTextSize(1);
   display.setCursor(75,24);//spo2
  display.print(temperature);
  display.display();
 

}



void sendData(int pulserate,float temperature, int oxygen){
  String url=SCRIPT_URL+"pulserate="+pulserate+"&oxygen="+oxygen+"&temperature="+temperature;
  if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status
  
    HTTPClient http;
  
    http.begin(url); //Specify the URL
    int httpCode = http.GET();                                     
  
    if (httpCode > 0) { //Check for the returning code
  
        String payload = http.getString();
        Serial.println(httpCode);
        Serial.println(payload);
      }
  
    else {
      Serial.println("Error on HTTP request");
      // http.end();
      // return false;
    }
  
    http.end(); //Free the resources
  }else{
    Serial.println("connection disturbed");
    // return false;
  }
  // return true;


}

