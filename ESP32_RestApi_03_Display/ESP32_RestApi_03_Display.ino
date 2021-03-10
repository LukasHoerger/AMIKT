#include <WiFi.h>
#include <WebServer.h>
#include <LiquidCrystal.h>

// SSID und Passwort zum Verbinden des ESP32 mit dem lokalen WLAN Netzwerk -> muss nachträglich ausgefüllt werden
const char *ssid = "";
const char *password = "";

// Definition der Pins und Variablen für die Verwendung des Drehgebermoduls
const int pinCLK = 23;
const int pinDT = 22;
int encoderPosCount = 0;
int pinALast;
int aVal;

// Definition der zusätzlich angelegten Funktionen
void rotaryEncoder();
void handleNotFound();
void GET_Base();
void GET_HelloWord();
void GET_RotaryEncoder();
void POST_DisplayMessage();

// Initialisieren des Web Servers mit dem Port 80
WebServer server(80);

// Initialisieren des LCD Displays mit den dazugehörigen Interface Pins
LiquidCrystal lcd(19, 21, 18, 17, 16, 15);

void setup(void)
{
    // Pins für den Drehgeber auf INPUT setzen -> Erkennen von Flanken
    pinMode(pinCLK, INPUT);
    pinMode(pinDT, INPUT);
    pinALast = digitalRead(pinCLK);

    // Anzahl der Spalten (16) und Zeilen (2) des LCD Displays definieren = Beschränckung der Darstellung durch HW
    lcd.begin(16, 2);

    // Initialer Text für die erste Zeile festlegen
    lcd.print("POST body:");

    // BAUD-Rate für Serieller-Monitor wird auf 115200 gesetzt
    Serial.begin(115200);
    Serial.print("Folgende SSID wurde definiert: ");
    Serial.print(ssid);

    // ESP32 mit der angegebenen SSID (WLAN) verbinden
    WiFi.begin(ssid, password);

    // Warten/Überprüfen, ob eine Verbindung mit dem WLAN hergestellt werden konnte
    Serial.println("");
    Serial.println("Es wird versucht eine Verbindung mit dem WLAN aufzubauen");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(100);
    }
    Serial.println("");
    Serial.println("WLAN Verbindung wurde erfolgreich hergestellt");
    Serial.print("Dem ESP32 wurde folgende IP-Adresse zugewiesen: ");
    Serial.print(WiFi.localIP());

    // Definition der Endpunkte des Webservers mit Routenpfad, HTTP-Methode und Handler (Achtung: case-sensitiv)
    server.on("/", HTTP_GET, GET_Base);
    server.on("/helloworld", HTTP_GET, GET_HelloWord);
    server.on("/rotaryencoder", HTTP_GET, GET_RotaryEncoder);
    server.on("/lcdmessage", HTTP_POST, POST_DisplayMessage);

    // Definition des Handler für nicht definierte/erreichbare Endpunkte
    server.onNotFound(handleNotFound);

    // Starten des WebServers
    server.begin();
    Serial.println("");
    Serial.println("HTTP Server erfolgreich gestartet");
}

void loop(void)
{
    rotaryEncoder();
    server.handleClient();
}

// Berechnen der Stellung des Drehgebers
void rotaryEncoder()
{
    aVal = digitalRead(pinCLK);
    if (aVal != pinALast)
    {
        // Überprüfen wo die Flanke zu erst erkannt wurde -> Aussage über Drehrichtung
        if (digitalRead(pinDT) != aVal)
        {
            //Flanke wurde zu erst an pinCLK erkannt -> Drehung im Uhrzeigersinn
            encoderPosCount++;
        }
        else
        {
            //Flanke wurde zu erst an pinDT erkannt -> Drehung gegen den Uhrzeigersinn
            encoderPosCount--;
        }
        Serial.print("Position des Drehgebermoduls: ");
        Serial.println(encoderPosCount);
    }
    pinALast = aVal;
}

// Handler für nicht definierte/erreichbare Endpunkte
void handleNotFound()
{
    String message = "Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++)
    {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
}

// Handler für den Base/Root-Endpunkt -> liefert HTML zurück
void GET_Base()
{
    String HtmlDoc = "<!DOCTYPE html>\n";
    HtmlDoc += "<html>\n";
    HtmlDoc += "<body>\n";
    HtmlDoc += "<h1>Wilkommen auf dem REST Web Server - AMIKT 2021</h1>\n";
    HtmlDoc += "</body>\n";
    HtmlDoc += "</html>\n";

    server.send(200, "text/html", HtmlDoc);
}

// Handler für den HelloWorld-Endpunkt -> liefert Text zurück
void GET_HelloWord()
{
    server.send(200, "text/plain", "Hello world from ESP32");
}

// Handler für den RotaryEncoder-Endpunkt -> liefert JSON zurück
void GET_RotaryEncoder()
{
    String Json = "{ \"name\": \"Drehgeber\", \"value\": ";
    Json += encoderPosCount;
    Json += " }";

    server.send(200, "application/json", Json);
}

// Handler für den DisplayMessage-Endpunkt -> Schreibt Body der Anfrage auf den LCD Display
void POST_DisplayMessage()
{
    String postBody = server.arg("plain");
    Serial.print("POST Body: ");
    Serial.println(postBody);

    lcd.clear();
    lcd.print("POST body:");
    lcd.setCursor(0, 1);
    lcd.print(postBody);

    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", "OK");
}
