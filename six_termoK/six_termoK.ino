#include "max6675.h"

//Ele esta no pino TX0, não é bom, pode dar interferência no serial
int thermoSCK = 4;  
int thermoCS  = 2;
int thermoSO = 3;

int thermoSCK2 = 8;  
int thermoCS2  = 9;
int thermoSO2 = 10;

int thermoSCK3 = 11;  
int thermoCS3  = 12;
int thermoSO3 = 13;

int thermoSCK4 = 32;  
int thermoCS4  = 33;
int thermoSO4 = 31;

int thermoSCK5 = 40;  
int thermoCS5  = 42;
int thermoSO5 = 44;

// Inicializa o sensor na ordem exata da biblioteca: (SCLK, CS, MISO/SO)
MAX6675 thermocouple1(thermoSCK, thermoCS, thermoSO);
MAX6675 thermocouple2(thermoSCK2, thermoCS2, thermoSO2);
MAX6675 thermocouple3(thermoSCK3, thermoCS3, thermoSO3);
MAX6675 thermocouple4(thermoSCK4, thermoCS4, thermoSO4);
MAX6675 thermocouple5(thermoSCK5, thermoCS5, thermoSO5);

void setup() {
  Serial.begin(9600);
  Serial.println("Iniciando leitura do sensor MAX6675...");
  delay(500);
}

void loop() {
  // Faz a leitura da temperatura em graus Celsius
  float celsius1 = thermocouple1.readCelsius();
  float celsius2 = thermocouple2.readCelsius();
  float celsius3 = thermocouple3.readCelsius();
  float celsius4 = thermocouple4.readCelsius();
  float celsius5 = thermocouple5.readCelsius();

  // Verifica se houve falha na leitura (ex: termopar desconectado)
  if (isnan(celsius1)) {
    Serial.println("Erro: Verifique as conexões do termopar tipo K!");
  } else {
    Serial.print("Temperatura 1: ");
    Serial.print(celsius1);
    Serial.println(" °C");
  }
  if (isnan(celsius2)) {
    Serial.println("Erro: Verifique as conexões do termopar tipo K!");
  } else {
    Serial.print("Temperatura 2: ");
    Serial.print(celsius2);
    Serial.println(" °C");
  }
  if (isnan(celsius3)) {
    Serial.println("Erro: Verifique as conexões do termopar tipo K!");
  } else {
    Serial.print("Temperatura 3: ");
    Serial.print(celsius3);
    Serial.println(" °C");
  }
  if (isnan(celsius4)) {
    Serial.println("Erro: Verifique as conexões do termopar tipo K!");
  } else {
    Serial.print("Temperatura 4: ");
    Serial.print(celsius4);
    Serial.println(" °C");
  }
  if (isnan(celsius5)) {
    Serial.println("Erro: Verifique as conexões do termopar tipo K!");
  } else {
    Serial.print("Temperatura 5: ");
    Serial.print(celsius5);
    Serial.println(" °C");
  }

  Serial.println("-----------------------------------");
  // Aguarda 5 segundos antes da próxima leitura
  delay(5000);
}
