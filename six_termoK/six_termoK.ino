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
float corrente = 0.0;

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
  float celsius = 0.0;
  delay (1);
  lerCorrente();

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

void lerCorrente() {
  float somaQuadrados = 0;
  int numeroAmostras = 40;
  // Variável estática para manter o histórico da última corrente e suavizar a leitura (Filtro)
  static float correnteSuavizada = 0.0;
  delay(1);

  for (int i = 0; i < numeroAmostras; i++) {
    int leituraBruta = analogRead(pinoSensorCorrente);

    // Converte a leitura (0 a 1023) para Tensão (0 a 5V)
    float tensao = (leituraBruta * 5.0) / 1023.0;

    // Subtrai o offset central de 2.5V para isolar apenas a onda AC oscilante
    float tensaoAC = tensao - 2.5;

    // Ignora micro-ruídos abaixo de um limiar muito baixo perto do zero
    if (abs(tensaoAC) < 0.03) {
      tensaoAC = 0;
    }

    // Eleva ao quadrado para acumular no cálculo RMS
    somaQuadrados += (tensaoAC * tensaoAC);
    
    delay(1); // Pequeno atraso entre as amostras
  }
    
  // Calcula a tensão RMS (raiz da média dos quadrados)
  float tensaoRMS = sqrt(somaQuadrados / (float)numeroAmostras);
  
  // Converte a Tensão RMS para Corrente RMS usando a sensibilidade do ACS712-20A (0.100 V/A)
  float correnteInstantanea = tensaoRMS / 0.100;

  // --- FILTRO DE SUAVIZAÇÃO (Média Exponencial) ---
  // O valor 0.3 controla a velocidade da resposta. Quanto menor (ex: 0.1 ou 0.2), 
  // mais estável fica o número, ignorando picos rápidos de ruído.
  correnteSuavizada = (correnteInstantanea * 0.3) + (correnteSuavizada * 0.7);

  if (correnteSuavizada < 0.1) {
    corrente = 0; 
  } else {
    corrente = correnteSuavizada;
  }

  if (corrente >= 4.0) {
    falhaCorrente = true; // Ativa o travamento de segurança!
    sistemaLigado = false;
  }
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