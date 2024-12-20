#include <ESP8266WiFi.h>
#include "secrets.h"
#include "ThingSpeak.h"

// WiFi
char ssid[] = SECRET_SSID;   // Nombre de la red WiFi
char pass[] = SECRET_PASS;   // Contraseña de la red WiFi
WiFiClient client;

// ThingSpeak
unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

// Pines LEDs
const int ledVerde1 = 13;
const int ledVerde2 = 12;
const int ledAmarillo = 4;
const int ledRojo = 14;
const int ledReferencia = 3; 

// Variables
unsigned long ultimaActualizacion = 0; // Para llevar control del tiempo de envío
const unsigned long intervaloEnvio = 20000; 

void setup() {
  Serial.begin(115200);  
  WiFi.mode(WIFI_STA); 
  ThingSpeak.begin(client);  //ThingSpeak

  pinMode(ledVerde1, OUTPUT);
  pinMode(ledVerde2, OUTPUT);
  pinMode(ledAmarillo, OUTPUT);
  pinMode(ledRojo, OUTPUT);
  pinMode(ledReferencia, OUTPUT); 

  digitalWrite(ledVerde1, LOW);
  digitalWrite(ledVerde2, LOW);
  digitalWrite(ledAmarillo, LOW);
  digitalWrite(ledRojo, LOW);
  digitalWrite(ledReferencia, LOW); 
}

void loop() {
  // Conexión WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Intentando conectar a SSID: ");
    Serial.println(SECRET_SSID);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass); 
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nConectado a WiFi.");
  }

  
  int valorAnalogico = analogRead(A0);             
  float voltaje = valorAnalogico * (3.3 / 1023.0); 
  float indiceUV = voltaje / (3.3 / 60.0);                 

  
  Serial.print("Voltaje: ");
  Serial.print(voltaje);
  Serial.print(" V, Indice UV: ");
  Serial.println(indiceUV);

  
  if (indiceUV >= 0 && indiceUV <= 1) {
    digitalWrite(ledVerde1, HIGH);
    digitalWrite(ledVerde2, LOW); 
    digitalWrite(ledAmarillo, LOW); 
    digitalWrite(ledRojo, LOW);
  } else if (indiceUV >= 2 && indiceUV <= 3) {
    digitalWrite(ledVerde1, LOW); 
    digitalWrite(ledVerde2, HIGH);
    digitalWrite(ledAmarillo, LOW); 
    digitalWrite(ledRojo, LOW);
  } else if (indiceUV >= 4) {
    digitalWrite(ledVerde1, LOW); 
    digitalWrite(ledVerde2, LOW); 
    digitalWrite(ledAmarillo, HIGH);
    digitalWrite(ledRojo, LOW);
  }

  
  int estadoLedReferencia = ThingSpeak.readLongField(myChannelNumber, 6, SECRET_READ_APIKEY);
  if (estadoLedReferencia == 1) {
    digitalWrite(ledReferencia, HIGH); 
  } else if (estadoLedReferencia == 0) {
    digitalWrite(ledReferencia, LOW);  
  }

  
  if (millis() - ultimaActualizacion >= intervaloEnvio) {
    ThingSpeak.setField(1, indiceUV);      
    ThingSpeak.setField(2, digitalRead(ledVerde1)); 
    ThingSpeak.setField(3, digitalRead(ledVerde2)); 
    ThingSpeak.setField(4, digitalRead(ledAmarillo)); 
    ThingSpeak.setField(5, digitalRead(ledRojo));    

    int httpCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if (httpCode == 200) {
      Serial.println("Actualización exitosa en ThingSpeak.");
    } else {
      Serial.println("Error al actualizar en ThingSpeak. Código HTTP: " + String(httpCode));
    }

    ultimaActualizacion = millis(); 
  }

  delay(300); 
}
