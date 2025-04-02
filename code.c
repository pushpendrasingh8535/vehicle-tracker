#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <LiquidCrystal.h>

SoftwareSerial gpsSerial(2, 3);   // TX, RX for GPS
SoftwareSerial gsmm(7, 8);        // TX, RX for SIM800L
TinyGPSPlus gps;

// LCD Pins: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(9, 10, 5, 6, 11, 12);

const int buttonPin = 4;    // Push button for Theft Mode
const int ignitionPin = 13; // Push button for Ignition

bool theftMode = false;
bool ignitionState = false;  // Ignition OFF initially
bool lastButtonState;
bool lastIgnitionState;
bool theftAlertSent = false; // Flag to send "Vehicle Stolen" alert only once

unsigned long lastDebounceTime = 0;
const long debounceDelay = 50; 

unsigned long lastLCDUpdate = 0; 
const long lcdUpdateInterval = 500; // Update LCD every 500ms

unsigned long lastSendTime = 0; // Timer for GPS location updates

float latitude, longitude;
String receivedData = "";
String senderNumber = ""; 
String keyword = "LOCATION";  
String responseMessage;
String theftNumber = "+xxxxxxxxxxxx";  // Emergency number  (With country code)
String authorizedNumber = "+xxxxxxxxxxxx";  // Only this number can request location  (With country code)

void setup() {
    Serial.begin(9600);
    
    pinMode(buttonPin, INPUT_PULLUP);
    pinMode(ignitionPin, INPUT_PULLUP);  

    // Read initial button states to prevent startup toggling
    lastButtonState = digitalRead(buttonPin);
    lastIgnitionState = digitalRead(ignitionPin);

    lcd.begin(16, 2);
    updateLCD();  

    gpsSerial.begin(9600);
    gsmm.begin(9600);
    gsmm.listen();

    Serial.println("Initializing SIM800L...");
    sendATCommand("AT");
    sendATCommand("AT+CMGF=1");  // SMS text mode
    sendATCommand("AT+CNMI=1,2,0,0,0"); // Enable SMS notifications

    Serial.println("Waiting for SMS command...");

    delay(1000);  // Stabilization delay after power-up
}

void loop() {
    checkButtonPress();
    checkIgnitionToggle();
    
    gsmm.listen();
    
    if (gsmm.available()) {
        char c = gsmm.read();
        receivedData += c;

        if (c == '\n') { 
            Serial.println("Received SMS:");
            Serial.println(receivedData);
            
            if (receivedData.indexOf("+CMT:") >= 0) {
                senderNumber = extractPhoneNumber(receivedData);
                Serial.print("Sender Number: ");
                Serial.println(senderNumber);
            }

            if (receivedData.indexOf(keyword) >= 0 && senderNumber == authorizedNumber) {  
                if (getGPSData()) {
                    responseMessage = "https://www.google.com/maps/place/" + String(latitude, 6) + "," + String(longitude, 6);
                    sendSMS(senderNumber, responseMessage);
                } else {
                    Serial.println("GPS data not available.");
                }
            } else if (receivedData.indexOf(keyword) >= 0) {
                Serial.println("Unauthorized request, ignoring.");
            }

            receivedData = "";  
        }
    }

    // If Theft Mode is ON and Ignition is turned ON
    if (theftMode && ignitionState) {  
        // First send "Vehicle Stolen" alert once
        if (!theftAlertSent) {
            sendSMS(theftNumber, "ALERT: Vehicle has been stolen!");
            theftAlertSent = true;
        }

        // Send GPS location every 30 seconds
        if (millis() - lastSendTime >= 30000) {  
            lastSendTime = millis();
            if (getGPSData()) {
                responseMessage = "https://www.google.com/maps/place/" + String(latitude, 6) + "," + String(longitude, 6);
                sendSMS(theftNumber, responseMessage);
            } else {
                Serial.println("GPS not available for Theft Mode.");
            }
        }
    } else {
        theftAlertSent = false; // Reset alert flag when ignition is OFF
    }

    // Update LCD every 500ms to prevent flickering
    if (millis() - lastLCDUpdate >= lcdUpdateInterval) {
        updateLCD();
        lastLCDUpdate = millis();
    }
}

// Function to toggle Theft Mode
void checkButtonPress() {
    bool currentButtonState = digitalRead(buttonPin);

    if (currentButtonState == LOW && lastButtonState == HIGH) {
        if (millis() - lastDebounceTime > debounceDelay) {
            theftMode = !theftMode; // Toggle Theft Mode
            Serial.println(theftMode ? "Switched to Theft Mode" : "Switched to User Mode");
        }
        lastDebounceTime = millis();
    }
    
    lastButtonState = currentButtonState;
}

// Function to toggle Ignition ON/OFF
void checkIgnitionToggle() {
    bool currentIgnitionState = digitalRead(ignitionPin);

    if (currentIgnitionState == LOW && lastIgnitionState == HIGH) {
        if (millis() - lastDebounceTime > debounceDelay) {
            ignitionState = !ignitionState; // Toggle Ignition ON/OFF
            Serial.println(ignitionState ? "Ignition Turned ON" : "Ignition Turned OFF");
        }
        lastDebounceTime = millis();
    }

    lastIgnitionState = currentIgnitionState;
}

// Function to update LCD with Theft Mode and Ignition Status
void updateLCD() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Mode: ");
    lcd.print(theftMode ? "THEFT" : "USER");

    lcd.setCursor(0, 1);
    lcd.print("Ignition: ");
    lcd.print(ignitionState ? "ON" : "OFF");
}

void sendATCommand(String command) {
    Serial.print("Sending: ");
    Serial.println(command);
    
    gsmm.println(command);
    delay(1000);

    while (gsmm.available()) {
        Serial.write(gsmm.read());
    }
    Serial.println();
}

String extractPhoneNumber(String data) {
    int start = data.indexOf("+CMT: \"") + 7;
    int end = data.indexOf("\"", start);
    if (start > 6 && end > start) {
        return data.substring(start, end);
    }
    return "";
}

bool getGPSData() {
    Serial.println("Fetching GPS data...");
    gpsSerial.listen();
    delay(300);

    unsigned long startTime = millis();
    while (millis() - startTime < 30000) {
        while (gpsSerial.available() > 0) {
            gps.encode(gpsSerial.read());
        }

        if (gps.location.isUpdated()) {
            latitude = gps.location.lat();
            longitude = gps.location.lng();
            
            Serial.print("LAT: ");
            Serial.println(latitude, 6);
            Serial.print("LON: ");
            Serial.println(longitude, 6);
            return true;
        }

        delay(500);
    }

    Serial.println("GPS Timeout.");
    return false;
}

void sendSMS(String number, String message) {
    gsmm.println("AT+CMGS=\"" + number + "\"");
    delay(2000);
    gsmm.print(message);
    delay(500);
    gsmm.write(26);  
    delay(3000);
    Serial.println("SMS Sent!");
}
