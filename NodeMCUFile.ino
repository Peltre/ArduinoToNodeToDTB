//Reciever nodemcu
//Sending Data from Arduino to NodeMCU Via Serial Communication
//NodeMCU code

//Include Lib for Arduino to Nodemcu
#include <ESP8266WiFi.h> 
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <Firebase_ESP_Client.h>

#include "addons/RTDBHelper.h"
#include "addons/TokenHelper.h"

#define WIFI_SSID "NOMBRE_DE_LA_RED"
#define WIFI_PASSWORD "CONTRASEÑA"
#define API_KEY "APY KEY DE LA BASE DE DATOS"
#define DATABASE_URL "URL DE LA BASE DE DATOS"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

//D6 = Rx & D5 = Tx
SoftwareSerial nodemcu(D5, D4);


void setup() {
  // Initialize Serial port
  Serial.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando al internet");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(" . ");
    delay(300);
  }
  
  Serial.println();
  Serial.print("Conectado");
  nodemcu.begin(9600);
  while (!Serial) continue;

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if(Firebase.signUp(&config, &auth, "", "")){
    Serial.println("signUp OK");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {

  delay(1000);

    // Read data from SoftwareSerial
  String receivedData = nodemcu.readString();

  // Print received data for debugging
  Serial.println("Received Data:");
  Serial.println(receivedData);

  
  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& data = jsonBuffer.parseObject(nodemcu);

  // Clear buffer after reading
  nodemcu.readString();

  if (data == JsonObject::invalid()) {
    //Serial.println("Invalid Json Object");
    Serial.println("Failed to parse JSON");
    jsonBuffer.clear();
    return;
  }

  Serial.println("JSON Object Recieved");
  Serial.print("Recieved Humidity:  ");
  float hum = data["humidity"];
  Serial.println(hum);
  Serial.print("Recieved Temperature:  ");
  float temp = data["temperature"];
  Serial.println(temp);
  Serial.println("-----------------------------------------");
  Serial.print("Recieved Luminosity:  ");
  int valorLDR = data["luminosity"];
  Serial.println(valorLDR);
  Serial.println("-----------------------------------------");
  Serial.print("Recieved RawGasValue:  ");
  int raw_adc = data["gasraw"];
  Serial.println(raw_adc);
  Serial.print("Recieved GasValue:  ");
  float value_adc = data["gasvalor"];
  Serial.println(value_adc);
  Serial.println("-----------------------------------------");
  Serial.print("Recieved Weight:  ");
  int pesog = data["weight"];
  Serial.println(pesog);
  Serial.println("-----------------------------------------");
  Serial.print("Recieved movement:  ");
  bool pirStatus = data["movement"];
  Serial.println(pirStatus);
  Serial.println("-----------------------------------------");

  if(Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    //Guardamos los valores recaudados a nuestra base de datos
    //Guardamos el valor de Temperatura primero
    if(Firebase.RTDB.setFloat(&fbdo, "SensorDHT/valores/temperatura",temp)){
      Serial.println();
      Serial.print("Guardado exitosamente en: " + fbdo.dataPath());
    } else {
      Serial.print("Hubo un error" + fbdo.errorReason());
    }

    //Guardamos el valor de temperatura
    if (Firebase.RTDB.setFloat(&fbdo, "SensorDHT/valores/humedad",hum)) {
      Serial.println();
      Serial.print("Guardado exitosamente en: " + fbdo.dataPath());
    } else {
      Serial.print("Hubo un error" + fbdo.errorReason());
    }

    //Guardamos el valor de la Fotorresistencia
    if (Firebase.RTDB.setInt(&fbdo, "SensorLDR/Valor/Luminosidad",valorLDR)) {
      Serial.println();
      Serial.print("Guardado exitosamente en: " + fbdo.dataPath());
    } else {
      Serial.print("Hubo un error" + fbdo.errorReason());
    }

//Guardamos el valor del sensor de GAS
    if (Firebase.RTDB.setInt(&fbdo, "SensorMQ/Valores/GasRaw",raw_adc)) {
      Serial.println();
      Serial.print("Guardado exitosamente en: " + fbdo.dataPath());
    } else {
      Serial.print("Hubo un error" + fbdo.errorReason());
    }

//Guardamos el valor del sensor de gas, pero la tension
    if (Firebase.RTDB.setFloat(&fbdo, "SensorMQ/Valores/GasProcesado",value_adc)) {
      Serial.println();
      Serial.print("Guardado exitosamente en: " + fbdo.dataPath());
    } else {
      Serial.print("Hubo un error" + fbdo.errorReason());
    }

    //Guardamos el valor del sensor de peso
    if (Firebase.RTDB.setInt(&fbdo, "SensorHX711/Valor/Peso",pesog)) {
      Serial.println();
      Serial.print("Guardado exitosamente en: " + fbdo.dataPath());
    } else {
      Serial.print("Hubo un error" + fbdo.errorReason());
    }

    //Guardamos el valor del sensor de movimiento
    if (Firebase.RTDB.setInt(&fbdo, "SensorPIR/Valor/Movimiento",pirStatus)) {
      Serial.println();
      Serial.print("Guardado exitosamente en: " + fbdo.dataPath());
    } else {
      Serial.print("Hubo un error" + fbdo.errorReason());
    }
  }
}