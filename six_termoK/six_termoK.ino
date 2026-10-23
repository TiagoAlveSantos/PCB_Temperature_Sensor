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

// --- ADIÇÃO DO SENSOR DE CORRENTE ---
const int pinoSensorCorrente = A13;

// Pinos dos Relés (Conectados nos pinos 23 e 25)
const int pinoReleResistencia = 23;  // CH1 - Relé da Resistência (Fio Laranja)
const int pinoReleVentilador  = 25;  // CH2 - Relé do Ventilador (Fio Azul)

void setup() {
  Serial.begin(9600);

  // Configura os pinos dos relés como saída
  pinMode(pinoReleResistencia, OUTPUT);
  pinMode(pinoReleVentilador, OUTPUT);
  
  // Inicializa os relés desligados (geralmente a maioria dos módulos desliga em HIGH)
  digitalWrite(pinoReleResistencia, HIGH);
  digitalWrite(pinoReleVentilador, HIGH);

  delay(500);
}

bool falhaCorrente = false;

void loop() {
  // Leitura do Sensor de Corrente ACS712 (20A) no pino A13
  int leituraAnalogica = analogRead(pinoSensorCorrente);
  
  // Converte a leitura (0 a 1023) para Tensão (0 a 5V)
  float tensao = (leituraAnalogica * 5.0) / 1023.0;
  
  // Fórmula específica para o ACS712 de 20A (sensibilidade de 100mV/A ou 0.100 V/A)
  float corrente = (tensao - 2.5) / 0.100; 
  if (corrente < 0) corrente = 0; // Remove ruídos negativos leves

  if (corrente >= 3.5) {
    falhaCorrente = true; // Ativa o travamento de segurança!
  }

  float tempResistencia = 0.0;

  // Percorre todos os sensores enviando os valores em uma única linha
  for (int i = 0; i < numSensors; i++) {
    float celsius = sensors[i].readCelsius();

    if (isnan(celsius)) {
      Serial.print("NaN"); // Envia indicativo de erro se falhar
    } else {
      Serial.print(celsius, 2); // Envia o valor com 2 casas decimais
      if (i == 0) {
        tempResistencia = celsius; // O primeiro sensor (índice 0) controla a resistência
      }
    }

    // Adiciona uma vírgula para separar os dados de cada sensor (exceto o último)
    if (i < numSensors - 1) {
      Serial.print(",");
    }
  }
  
  Serial.print(corrente, 2);
  Serial.println();

  // 4. Lógica de Controle Automático da Resistência (Limite de 300°C)
  // Com histerese para evitar oscilações bruscas.
  // Se a corrente ultrapassar 3.5A desarmo o relé.
  if (falhaCorrente) {
    digitalWrite(pinoReleResistencia, HIGH); // Desliga a resistência
  } else {
    if (tempResistencia >= 301.0) {
        digitalWrite(pinoReleResistencia, HIGH); // Desliga a resistência se passar de 300°C
    } else if (tempResistencia <= 295.0) {
      digitalWrite(pinoReleResistencia, LOW);  // Liga a resistência se cair para 295°C ou menos
    }
  }
  // Intervalo de 1 segundo entre as leituras
  delay(1000);
}