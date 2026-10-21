#include "max6675.h"

// Pinos independentes dos 5 sensores
const int sckPins[] = {4,  8,  11, 32, 40};  
const int csPins[]  = {2,  9,  12, 33, 42};  
const int soPins[]  = {3,  10, 13, 31, 44};  

const int numSensors = sizeof(sckPins) / sizeof(sckPins[0]);

MAX6675 sensors[] = {
  MAX6675(sckPins[0], csPins[0], soPins[0]),
  MAX6675(sckPins[1], csPins[1], soPins[1]),
  MAX6675(sckPins[2], csPins[2], soPins[2]),
  MAX6675(sckPins[3], csPins[3], soPins[3]),
  MAX6675(sckPins[4], csPins[4], soPins[4])
};

void setup() {
  // A taxa de transmissão (Baud Rate) deve ser idêntica no Keysight VEE
  Serial.begin(9600);
  delay(500);
}

void loop() {
  // Percorre todos os sensores enviando os valores em uma única linha
  for (int i = 0; i < numSensors; i++) {
    float celsius = sensors[i].readCelsius();

    if (isnan(celsius)) {
      Serial.print("NaN"); // Envia indicativo de erro se falhar
    } else {
      Serial.print(celsius, 2); // Envia o valor com 2 casas decimais
    }

    // Adiciona uma vírgula para separar os dados de cada sensor (exceto o último)
    if (i < numSensors - 1) {
      Serial.print(",");
    }
  }
  
  // O println final quebra a linha, sinalizando o fim do pacote de leitura para o VEE
  Serial.println();

  // Intervalo de 1 segundo entre as leituras
  delay(1000);
}