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
const int ledVerde1 = 5;
const int ledVerde2 = 4;
const int ledAmarillo = 0;
const int ledRojo = 2;

void setup() {
  Serial.begin(115200);  
  WiFi.mode(WIFI_STA); 
  ThingSpeak.begin(client);  // Inicializa ThingSpeak

  
  pinMode(ledVerde1, OUTPUT);
  pinMode(ledVerde2, OUTPUT);
  pinMode(ledAmarillo, OUTPUT);
  pinMode(ledRojo, OUTPUT);

  
  digitalWrite(ledVerde1, LOW);
  digitalWrite(ledVerde2, LOW);
  digitalWrite(ledAmarillo, LOW);
  digitalWrite(ledRojo, LOW);
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

  // Lectura del sensor UV
  int valorAnalogico = analogRead(A0);             
  float voltaje = valorAnalogico * (3.3 / 1023.0); 
  float indiceUV = voltaje * 11.0;                 

  // Muestra los valores en la consola
  Serial.print("Voltaje: ");
  Serial.print(voltaje);
  Serial.print(" V, Indice UV: ");
  Serial.println(indiceUV);

  // Control de los LEDs según el índice UV
  int estadoVerde1 = 0, estadoVerde2 = 0, estadoAmarillo = 0, estadoRojo = 0;

  if (indiceUV >= 0 && indiceUV <= 2) {
    digitalWrite(ledVerde1, HIGH); estadoVerde1 = 1;
    digitalWrite(ledVerde2, LOW); 
    digitalWrite(ledAmarillo, LOW); 
    digitalWrite(ledRojo, LOW);
  } else if (indiceUV >= 3 && indiceUV <= 5) {
    digitalWrite(ledVerde1, LOW); 
    digitalWrite(ledVerde2, HIGH); estadoVerde2 = 1;
    digitalWrite(ledAmarillo, LOW); 
    digitalWrite(ledRojo, LOW);
  } else if (indiceUV >= 6 && indiceUV <= 7) {
    digitalWrite(ledVerde1, LOW); 
    digitalWrite(ledVerde2, LOW); 
    digitalWrite(ledAmarillo, HIGH); estadoAmarillo = 1;
    digitalWrite(ledRojo, LOW);
  } else if (indiceUV >= 8) {
    digitalWrite(ledVerde1, LOW); 
    digitalWrite(ledVerde2, LOW); 
    digitalWrite(ledAmarillo, LOW); 
    digitalWrite(ledRojo, HIGH); estadoRojo = 1;
  }

  // Envía los datos a ThingSpeak
  ThingSpeak.setField(1, indiceUV);      
  ThingSpeak.setField(2, estadoVerde1); 
  ThingSpeak.setField(3, estadoVerde2); 
  ThingSpeak.setField(4, estadoAmarillo); 
  ThingSpeak.setField(5, estadoRojo);    

  int httpCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (httpCode == 200) {
    Serial.println("Actualización exitosa en ThingSpeak.");
  } else {
    Serial.println("Error al actualizar en ThingSpeak. Código HTTP: " + String(httpCode));
  }

  delay(20000); // Espera 20 segundos antes de la próxima actualización
}
