#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WiFiClientSecure.h>
#include <LiquidCrystal.h>
#include <Bounce2.h>
#include <ArduinoSpotify.h>
#include <ArduinoJson.h>
#include <ArduinoSpotifyCert.h>

#include "secrets.h"

const char ssid[] = SECRET_SSID;
const char password[] = SECRET_PASS;

const char clientId[] = CLIENT_ID;
const char clientSecret[] = CLIENT_SECRET;

#define SPOTIFY_MARKET "DE"

#define REDPIN 33
#define GREENPIN 32
#define BUTTONPIN 25

const char *refreshToken = REFRESH_TOKEN;

char scope[] = "user-read-playback-state%20user-modify-playback-state";
char callbackURItemplate[] = "%s%s%s";
char callbackURIProtocol[] = "http%3A%2F%2F"; // "http://"
char callbackURIAddress[] = "%2Fcallback%2F"; // "/callback/"
char callbackURI[100];

unsigned long delayBetweenRequests = 5000; // Time between requests (1 minute)
unsigned long requestDueTime;               //time when request due

boolean redState = 0;
boolean greenState = 0;
boolean isPlaying = 0;

LiquidCrystal lcd(19, 23, 18, 17, 16, 15);

WebServer server(80);

WiFiClientSecure client;

 ArduinoSpotify spotify(client, clientId, clientSecret);

Bounce2::Button button = Bounce2::Button();

const char *webpageTemplate =
    R"(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no" />
  </head>
  <body>
    <div>
     <a href="https://accounts.spotify.com/authorize?client_id=%s&response_type=code&redirect_uri=%s&scope=%s">spotify Auth</a>
    </div>
  </body>
</html>
)";

void handleRoot();
void handleCallback();
void handleNotFound();
void printCurrentlyPlaying(CurrentlyPlaying currentlyPlaying);

void setup() {

  Serial.begin(115200);

  lcd.begin(16, 2);

  pinMode (REDPIN, OUTPUT);
  pinMode (GREENPIN, OUTPUT);

  button.attach( BUTTONPIN, INPUT );
  button.interval(5); 
  button.setPressedState(LOW); 
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  IPAddress ipAddress = WiFi.localIP();
  Serial.println(ipAddress);

  if (MDNS.begin("amikt"))
  {
    Serial.println("MDNS responder started");
  }

  client.setCACert(spotify_server_cert);

  sprintf(callbackURI, callbackURItemplate, callbackURIProtocol, "amikt.local", callbackURIAddress);

  if (refreshToken[0] != '\0') {
    spotify.setRefreshToken(refreshToken);
    if (!spotify.refreshAccessToken()) {
        Serial.println("Failed to get access tokens");
    }
  }

    
  server.on("/", handleRoot);
  server.on("/callback/", handleCallback);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  button.update();
  
  if ((refreshToken != NULL) && (refreshToken[0] != '\0')) { 
    if (millis() > requestDueTime) {
        //Serial.println(String(refreshToken));

        CurrentlyPlaying currentlyPlaying = spotify.getCurrentlyPlaying(SPOTIFY_MARKET);
        printCurrentlyPlaying(currentlyPlaying);

        PlayerDetails playerDetails = spotify.getPlayerDetails(SPOTIFY_MARKET);
        isPlaying = playerDetails.isPlaying;
        
        requestDueTime = millis() + delayBetweenRequests;
    }
  }
  
  

  if ( button.pressed() ) {
    
    if (!spotify.getPlayerDetails(SPOTIFY_MARKET).isPlaying) {
      if(spotify.play()){
          Serial.println("Playing!");
      }
    } else {
      if(spotify.pause()){
        Serial.println("Paused!");
      }
    }

    isPlaying = !isPlaying;
        
  }
  
  if (isPlaying) {
    digitalWrite(GREENPIN,HIGH);
    digitalWrite(REDPIN,LOW);
  } else {
    digitalWrite(GREENPIN,LOW);
    digitalWrite(REDPIN,HIGH);
  }
}

void handleRoot() {
  char webpage[800];
  sprintf(webpage, webpageTemplate, clientId, callbackURI, scope);
  server.send(200, "text/html", webpage);
}

void handleCallback() {
  String code = "";
  //const char *refreshToken = NULL;
  for (uint8_t i = 0; i < server.args(); i++)
  {
    if (server.argName(i) == "code")
    {
      code = server.arg(i);
      refreshToken = spotify.requestAccessTokens(code.c_str(), callbackURI);
    }
  }

  if ((refreshToken != NULL) && (refreshToken[0] != '\0')) {
    server.send(200, "text/plain", refreshToken);
    if (!spotify.refreshAccessToken()) {
        Serial.println("Failed to get access tokens");
    }
    spotify.setRefreshToken(refreshToken);
  }
  else
  {
    server.send(404, "text/plain", "Failed to load token, check serial monitor");
  }

}

void handleNotFound() {
  String message = "File Not Found\n\n";
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

  Serial.print(message);
  server.send(404, "text/plain", message);
}

void printCurrentlyPlaying(CurrentlyPlaying currentlyPlaying)
{
    if (!currentlyPlaying.error) {
        Serial.print("Track: ");
        Serial.println(currentlyPlaying.trackName);
        Serial.println("------------------------");
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(String(currentlyPlaying.trackName).substring(0,16));
        lcd.setCursor(0, 1);
        lcd.print(String(currentlyPlaying.trackName).substring(16,32));
    }
}
