//Sender arduino
//Sending Data from Arduino to NodeMCU Via Serial Communication
//Arduino code

//DHT11 Lib
#include <DHT.h>
#include "HX711.h"
#include <Servo.h>

//Arduino to NodeMCU Lib
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

//Initialise Arduino to NodeMCU (5=Rx & 6=Tx)
SoftwareSerial nodemcu(4, 5);

//Initialisation of DHT11 Sensor
#define DHTPIN 7
#define DHTTYPE DHT11
#define DT A2 // DT de HX711 a pin analogo 2
#define SCK A3 // SCK de HX711 a pin analogo 3

Servo myservo;
DHT dht(DHTPIN, DHTTYPE);
HX711 celda; // crea objeto con nombre celda
const int LEDHX = 10;
  const int LEDR = 13;
  const int LEDB = 12;
  const int LEDG = 8;
const int ldrPin = A1;
const int MQ_PIN = A0;
const int Venti = 2;
const int pirPin = 6;
const int LEDPIN = 3;

//Definimos las variables en donde guardaremos los valores de los sensores
float temp;
float hum;
int valorLDR;
int umbral = 750;
bool pirStatus;
int raw_adc;
float value_adc;
int pesog;

void setup() {
  nodemcu.begin(9600);
  Serial.begin(9600);
  delay(1000);

  Serial.println("Program started");

  pinMode(ldrPin, INPUT);
    myservo.attach(9);
    myservo.write(90);   
    pinMode(LEDR,OUTPUT);
    pinMode(LEDG,OUTPUT);
    pinMode(LEDB,OUTPUT);
    pinMode(Venti,OUTPUT);
    pinMode(LEDPIN,OUTPUT);
    pinMode(pirPin, INPUT);
    Serial.println(F("Initialize System"));
    pinMode(LEDHX, OUTPUT);

  dht.begin();

  celda.begin(DT, SCK);   // inicializa objeto con los pines a utilizar
  celda.set_scale(466.f);  // establece el factor de escala obtenido del primer programa
  celda.tare();     // realiza la tara o puesta a cero
}

void loop() {

  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();

  //Obtain Temp and Hum data FUNCIONES REC DATOS
  dht11_func();
  //Obtener el valor de la Luminosidad
  LDR_func();
  //Obtener el valor del sensor de gas
  adc_func();
  //Obtener el valor del sensor de peso
  peso_func();
  //Obtener el valor del sensor PIR
  pir_func();


  //Assign collected data to JSON Object
  data["humidity"] = hum;
  data["temperature"] = temp; 
  data["luminosity"] = valorLDR;
  data["gasraw"] = raw_adc;
  data["gasvalor"] = value_adc;
  data["weight"] = pesog;
  data["movement"] = pirStatus;

  //Send data to NodeMCU
  data.printTo(nodemcu);
  jsonBuffer.clear();

  if (!isnan(temp) && !isnan(hum)) {

    //Serial.print("Temperatura: ");
   // Serial.print(temp);
    //Serial.print(" °C\tHumedad: ");
    //Serial.print(hum);
    //Serial.println(" %");
    //Serial.print("Luminosidad: ");
    //Serial.print(valorLDR);  
    //Serial.print("\nRaw:");
    //Serial.print(raw_adc);
    //Serial.print("    Tension:");
    //Serial.println(value_adc);
    
  } else {
    Serial.println("Error al leer el sensor");
  }
  if (hum >= 70){
   digitalWrite(LEDR, LOW);
   digitalWrite(LEDG, LOW);
   digitalWrite(LEDB, HIGH);
  } else {
   digitalWrite(LEDR, LOW);
   digitalWrite(LEDG, HIGH);
   digitalWrite(LEDB, LOW);
    }
    
  if (valorLDR < umbral) {
   myservo.write(5);
  } else {
   myservo.write(90);
  }
  
  if (raw_adc > 160){
   digitalWrite(LEDR, LOW);
   digitalWrite(LEDG, LOW);
   digitalWrite(LEDB, LOW);
  
  }
  
  if (temp >= 30){  
    digitalWrite(Venti, HIGH);
  } else{
    digitalWrite(Venti, LOW);
    }

  if (pirStatus)
  {
    //Serial.print("DATA,TIME:");
    //Serial.println(F("Sensor status"));
    
    digitalWrite(LEDPIN, HIGH);
   
  }else{
    //Serial.print("DATA,TIME:");
    //Serial.print(F("Sensor status")); Serial.println(pirStatus);
    digitalWrite(LEDPIN, LOW);
    
    }

   if (pesog >= 300){
    //Serial.print("Valor (gramos): ");   // texto descriptivo
    //Serial.println(pesog); // muestra el valor obtenido promedio de 10 lecturas
    celda.power_down();       // apaga el modulo HX711
    delay(50);          // demora de .005 segundos
    celda.power_up();       // despierta al modulo 
    digitalWrite (LEDHX, HIGH);
    }else{
    //Serial.print("Valor (gramos): ");   // texto descriptivo
    //Serial.println(pesog); // muestra el valor obtenido promedio de 10 lecturas
    digitalWrite (LEDHX, LOW);  
      }

  delay(2000);
}

//Definimos las funciones para recibir e imprimir los datos
void dht11_func() {

  hum = dht.readHumidity();
  temp = dht.readTemperature();
  Serial.print("Humedad: ");
  Serial.println(hum);
  Serial.print("Temperatura: ");
  Serial.println(temp);

}

void LDR_func() {

  valorLDR = analogRead(ldrPin);
  Serial.print("Luminosidad: ");
  Serial.println(valorLDR); 
}

void adc_func() {

  raw_adc = analogRead(MQ_PIN);
  value_adc = raw_adc * (5.0 / 1023.0);
  Serial.print("Valor de Gas Raw: ");
  Serial.println(raw_adc); 
  Serial.print("Valor de Gas Procesado: ");
  Serial.println(value_adc); 
}

void peso_func() {

  pesog = (celda.get_units(10));
  Serial.print("Peso en gramos: ");
  Serial.println(pesog); 
}

void pir_func() {

  pirStatus = digitalRead(pirPin);
  Serial.print("Movimiento: ");
  Serial.println(pirStatus);
  Serial.print("\n"); 
}


