#include "max6675.h"

// Pinos independentes para cada um dos 5 sensores
const int sckPins[] = {4,  8,  11, 32, 40};  
const int csPins[]  = {2,  9,  12, 33, 42};  
const int soPins[]  = {3,  10, 13, 31, 44};  

// Quantidade de sensores calculada automaticamente
const int numSensors = sizeof(sckPins) / sizeof(sckPins[0]);

// Inicializa os sensores usando os arrays de pinos independentes
MAX6675 sensors[] = {
  MAX6675(sckPins[0], csPins[0], soPins[0]),
  MAX6675(sckPins[1], csPins[1], soPins[1]),
  MAX6675(sckPins[2], csPins[2], soPins[2]),
  MAX6675(sckPins[3], csPins[3], soPins[3]),
  MAX6675(sckPins[4], csPins[4], soPins[4])
};

void setup() {
  Serial.begin(9600);
  Serial.println("Iniciando leitura dos sensores MAX6675...");
  delay(500);
}

void loop() {
  // Lê e exibe a temperatura de todos os sensores usando um loop
  for (int i = 0; i < numSensors; i++) {
    float celsius = sensors[i].readCelsius();

    Serial.print("Temperatura ");
    Serial.print(i + 1);
    Serial.print(": ");

    if (isnan(celsius)) {
      Serial.println("Erro: Verifique as conexões do termopar tipo K!");
    } else {
      Serial.print(celsius);
      Serial.println(" °C");
    }
  }

  Serial.println("-----------------------------------");
  
  // Aguarda 5 segundos antes da próxima leitura
  delay(5000);
}