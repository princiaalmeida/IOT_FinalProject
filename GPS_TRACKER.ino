#include <WiFi.h>
#include <TinyGPS++.h>

HardwareSerial SerialGPS(1); 
TinyGPSPlus gps;

// Your Wi-Fi credentials
const char* ssid = "Techno";
const char* password = "meGhana12";

// Global variables to hold GPS data
float Latitude, Longitude;
int year, month, date, hour, minute, second;
String DateString, TimeString, LatitudeString, LongitudeString;

WiFiServer server(80);

void setup() {
    Serial.begin(9600);
    // Initialize GPS serial communication with specified pins
    SerialGPS.begin(9600, SERIAL_8N1, 16, 17);
    
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    server.begin();
    Serial.println("Server started");
}

void loop() {
    // --- GPS Data Processing (Non-blocking) ---
    while (SerialGPS.available() > 0) {
        if (gps.encode(SerialGPS.read())) {
            if (gps.location.isValid()) {
                Latitude = gps.location.lat();
                LatitudeString = String(Latitude, 6);
                Longitude = gps.location.lng();
                LongitudeString = String(Longitude, 6);
            }

            if (gps.date.isValid()) {
                year = gps.date.year();
                month = gps.date.month();
                date = gps.date.day();
                DateString = String(date) + " / " + String(month) + " / " + String(year);
            }

            if (gps.time.isValid()) {
                // Time adjustment for UTC+5, adjust as needed. 
                // Note: Does not account for daylight saving time.
                hour = gps.time.hour() + 5; 
                minute = gps.time.minute();
                second = gps.time.second();
                TimeString = String(hour) + " : " + String(minute) + " : " + String(second);
            }
        }
    }

    // --- Web Server Client Handling (Non-blocking) ---
    WiFiClient client = server.available();
    if (client) {
        String html = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
        html += "<!DOCTYPE html><html><head><title>NEO-6M GPS Readings</title>";
        html += "<meta http-equiv='refresh' content='10'>"; // Auto-refresh the page every 10 seconds
        html += "<style>body{font-family: Arial, sans-serif;} table, th, td {border: 1px solid blue; border-collapse: collapse; padding: 8px;} table{margin: 0 auto;}</style>";
        html += "</head><body><h1 align=center>NEO-6M GPS Readings</h1>";
        html += "<p align=center style=\"font-size:150%;\"><b>Location Details</b></p>";
        html += "<table align=center style=\"width:50%;\">";
        html += "<tr><th>Latitude</th><td align=center>" + LatitudeString + "</td></tr>";
        html += "<tr><th>Longitude</th><td align=center>" + LongitudeString + "</td></tr>";
        html += "<tr><th>Date</th><td align=center>" + DateString + "</td></tr>";
        html += "<tr><th>Time</th><td align=center>" + TimeString + "</td></tr>";
        html += "</table>";

        // Corrected Google Maps URL
        if (gps.location.isValid()) {
            html += "<p align=center><a style=\"color:blue; font-size:125%;\" href=\"https://www.google.com/maps/place/";
            html += LatitudeString;
            html += ",";
            html += LongitudeString;
            html += "\" target=\"_blank\">Click here to open the location in Google Maps.</a></p>";
        }

        html += "</body></html>\r\n";

        client.print(html);
        delay(100);
        client.stop();
    }
}