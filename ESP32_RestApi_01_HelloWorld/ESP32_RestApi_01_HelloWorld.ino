#include <WiFi.h>
#include <WebServer.h>

// SSID und Passwort zum Verbinden des ESP32 mit dem lokalen WLAN Netzwerk -> muss nachträglich ausgefüllt werden
const char *ssid = "";
const char *password = "";

// Definition der zusätzlich angelegten Funktionen
void handleNotFound();
void GET_HelloWord();
void GET_Base();

//Initialisieren des Web Servers mit dem Port 80
WebServer server(80);

void setup(void)
{
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

    // Definition des Handler für nicht definierte/erreichbare Endpunkte
    server.onNotFound(handleNotFound);

    // Starten des WebServers
    server.begin();
    Serial.println("");
    Serial.println("HTTP Server erfolgreich gestartet");
}

void loop(void)
{
    server.handleClient();
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
