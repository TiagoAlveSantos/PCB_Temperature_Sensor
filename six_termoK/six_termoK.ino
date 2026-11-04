#include "max6675.h"

// Pinos independentes dos 5 sensores
const int sckPins[] = {4,  8,  11, 32, 40, 52};  
const int csPins[]  = {2,  9,  12, 33, 42, 50};  
const int soPins[]  = {3,  10, 13, 31, 44, 48};  

const int numSensors = sizeof(sckPins) / sizeof(sckPins[0]);

MAX6675 sensors[] = {
  MAX6675(sckPins[0], csPins[0], soPins[0]),
  MAX6675(sckPins[1], csPins[1], soPins[1]),
  MAX6675(sckPins[2], csPins[2], soPins[2]),
  MAX6675(sckPins[3], csPins[3], soPins[3]),
  MAX6675(sckPins[4], csPins[4], soPins[4]),
  MAX6675(sckPins[5], csPins[5], soPins[5])
};

// --- ADIÇÃO DO SENSOR DE CORRENTE ---
const int pinoSensorCorrente = A13;

// Pinos dos Relés (Conectados nos pinos 23 e 25)
const int pinoReleResistencia = 23;  // CH1 - Relé da Resistência (Fio Laranja)
const int pinoReleVentilador  = 25;  // CH2 - Relé do Ventilador (Fio Azul)
const int pinoBotao           = 30;  // Botão Liga/Desliga (INPUT_PULLUP)
const int pinoLedVermelho     = 49;  // LED Vermelho
const int pinoLedVerde        = 51;  // LED Verde
const int pinoLedAzul         = 53;  // LED Azul

bool falhaCorrente = false;
bool sistemaLigado = false;
bool estadoBotaoAnterior = HIGH;
bool leituraBotao = HIGH;
float tempResistencia = 0.0;
float celsius = 0.0;
float somaLeituras = 0;
int numeroAmostras = 10;

// Variáveis para controle do tempo sem travamento (millis)
//unsigned long tempoAnterior = 0;
//const long intervaloLeitura = 1000;

void setup() {
  Serial.begin(9600);
   // Configura os pinos dos relés como saída
  pinMode(pinoReleResistencia, OUTPUT);
  pinMode(pinoReleVentilador, OUTPUT);
  pinMode(pinoBotao, INPUT_PULLUP);    // Ativa o resistor interno do Arduino
  pinMode(pinoLedVermelho, OUTPUT);
  pinMode(pinoLedVerde, OUTPUT);
  pinMode(pinoLedAzul, OUTPUT);
  
  // Inicializa os relés desligados
  digitalWrite(pinoReleResistencia, LOW);
  digitalWrite(pinoReleVentilador, LOW);

  digitalWrite(pinoLedVermelho, HIGH); 
  digitalWrite(pinoLedVerde, LOW);
  digitalWrite(pinoLedAzul, LOW);

  delay(500);
}

void loop() {

  acionamentoBotao();
  atualizaLED();
  lerSensores();
  controleSegurancaRele();
  delay(1000);

}

void acionamentoBotao() {
  leituraBotao = digitalRead(pinoBotao);
  if (leituraBotao == LOW && estadoBotaoAnterior == HIGH) {
    sistemaLigado = !sistemaLigado; // Inverte o estado (liga <-> desliga)
    if (sistemaLigado) {
      falhaCorrente = false; // Reseta a falha de corrente ao religar
    }
    delay(200); // Debounce
  }
  estadoBotaoAnterior = leituraBotao;
}

void atualizaLED(){
  // Atualiza o LED de status (aceso se o sistema estiver ligado)
  if (falhaCorrente) {
    // Cenário 3: Falha de corrente ativa (Prioridade Máxima)
    digitalWrite(pinoLedVerde, LOW);
    digitalWrite(pinoLedVermelho, LOW);
    digitalWrite(pinoLedAzul, HIGH);   // Apenas o Azul aceso
  } 
  else if (sistemaLigado) {
    // Cenário 2: Sistema Ligado e Operando normalmente
    digitalWrite(pinoLedVerde, HIGH);  // Apenas o Verde aceso
    digitalWrite(pinoLedVermelho, LOW);
    digitalWrite(pinoLedAzul, LOW);
  } 
  else {
    // Cenário 1: Sistema Desligado (Standby)
    digitalWrite(pinoLedVerde, LOW);
    digitalWrite(pinoLedVermelho, HIGH); // Apenas o Vermelho aceso
    digitalWrite(pinoLedAzul, LOW);
  }
}

void lerSensores() {
  somaLeituras = 0;
  delay(2);

  // Leitura do Sensor de Corrente ACS712 (20A) no pino A13
  for (int i = 0; i < numeroAmostras; i++) {
    somaLeituras += analogRead(pinoSensorCorrente);
    delay(3); // Pequeno atraso de 2ms para estabilizar o conversor AD
  }

  // Tira a média aritmética das 10 leituras
  float mediaAnalogica = somaLeituras / (float)numeroAmostras;

  // Converte a leitura (0 a 1023) para Tensão (0 a 5V)
  float tensao = (mediaAnalogica * 5.0) / 1023.0;
  
  // Fórmula específica para o ACS712 de 20A (sensibilidade de 100mV/A ou 0.100 V/A)
  float corrente = (tensao - 2.5) / 0.100; 
  if (corrente < 0) corrente = 0; // Remove ruídos negativos leves

  if (corrente >= 2.5) {
    falhaCorrente = true; // Ativa o travamento de segurança!
    sistemaLigado = false;
  }

  // Percorre todos os sensores enviando os valores em uma única linha
  for (int i = 0; i < numSensors; i++) {
    celsius = sensors[i].readCelsius();

    if (isnan(celsius)) {
      Serial.print("NaN"); // Envia indicativo de erro se falhar
    } else {
      Serial.print(celsius, 2); // Envia o valor com 2 casas decimais
      if (i == 0) {
        tempResistencia = celsius; // O primeiro sensor (índice 0) controla a resistência
      }
    }
    // Adiciona uma vírgula para separar os dados de cada sensor (exceto o último)
    if (i < numSensors) {
      Serial.print(",");
    }
  }
  
  Serial.print(corrente, 2);
  Serial.println();
}

void controleSegurancaRele() {
  // Lógica de Controle Automático da Resistência (Limite de 300°C)
  // Com histerese para evitar oscilações bruscas.
  // Se a corrente ultrapassar 2.5A desarmo o relé.
  if (sistemaLigado && !falhaCorrente) { 
    if (tempResistencia >= 100) {
        digitalWrite(pinoReleResistencia, LOW); // Desliga a resistência se passar de 301°C
    } else if (tempResistencia <=85) {
      digitalWrite(pinoReleResistencia, HIGH);  // Liga a resistência se cair para 295°C ou menos
    }
    digitalWrite(pinoReleVentilador, HIGH);
  } else {
    digitalWrite(pinoReleResistencia, LOW);
    // Se a temperatura ainda estiver alta (>= 30°C), mantém o ventilador ligado!
    if (tempResistencia >= 30) {
      digitalWrite(pinoReleVentilador, HIGH); // Mantém ventilador LIGADO
    } else {
      digitalWrite(pinoReleVentilador, LOW); // Abaixo de 30°C, pode desligar o ventilador
    }
  }
}