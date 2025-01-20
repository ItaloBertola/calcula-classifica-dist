// Código para medição e classificação de distância com sensor a laser óptico de alta precisão VL53L0X
// Para a explicação do funcionamento geral do código, ler arquivo Read Me

#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>

#define EEPROM_SIZE 12
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

const int buttonPin1 = 35;       // Botão 1 - Calibração
const int buttonPin2 = 34;       // Botão 2 - Pular calibração
const int buzzer = 18;
const int barraled[] = {13, 12, 14, 27, 26, 25, 33, 32, 19, 23};      // LED 0 (Azul); LEDs 1, 2, 3 e 4 (Verdes); LEDs 5, 6 e 7 (Amarelos); LEDs 8 e 9 (Vermelhos) 

// Variáveis de controle
int address = 0;                    // Endereço de memória da EEPROM, a receber o valor float da distância durante a calibração
bool calibracao = false;            // Iniciando calibração como false

void setup() {
  // Inicializando Monitor Serial (Serial USB)
  Serial.begin(9600);

  // Inicializando os botões
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);

  // Inicializando a barra led
  for (int i = 0; i < 10; i++){
    pinMode(barraled[i], OUTPUT);
    pinMode(buzzer, OUTPUT);

    digitalWrite(barraled[i], LOW);
    digitalWrite(buzzer, LOW);
  }

  // Inicializando o sensor de distância óptico
  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while (1);
  }
}

void loop() {
  while (calibracao == false){                                  // Loop de calibração (Só roda até a calibração ser finalizada e não retorna até o sistema ser reinicializado)
    calibracao = CalibraDistancia(address, calibracao);
  }

  // Iniciando e lendo a EEPROM para pegar o valor salvo durante a calibração
  EEPROM.begin(EEPROM_SIZE);
  float read_dist;                     // Variável para receber a distância calibrada
  EEPROM.get(address, read_dist);      // Recebendo a distância calibrada        
  EEPROM.end();                        // Fechando a memória EEPROM

  // Inicialização da medição do sensor de distância
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);

  // IFs de medição do sensor VL53L0X --> "read_dist" considerada como distância ideal
  if (measure.RangeStatus != 4) {               // MEDIÇÃO VÁLIDA
    float dist_atual = measure.RangeMilliMeter;         // Le o valor de distância do sensor
    dist_atual = dist_atual/10;                         // Alterando o valor da distância de mm para cm

    if (dist_atual >= 40){                              // Acima deste valor de distância, o sensor VL53L0X não foi capaz de garantir a captação adequada dos sinais (Distância muito grande). Portanto, não deve ser utilizado.
      AtivaLEDBuzzer(3, dist_atual, read_dist);
    }
    else {
      if (dist_atual > (read_dist+2) & dist_atual < 40) {          // Intervalo de classificação LONGE          
        AtivaLEDBuzzer(0, dist_atual, read_dist);
      }
      else if (dist_atual < (read_dist-2)){          // Intervalo de classificação PERTO
        AtivaLEDBuzzer(1, dist_atual, read_dist);
      }
      else{          // Intervalo de classificação IDEAL
        AtivaLEDBuzzer(2, dist_atual, read_dist);
      }
    }
  }
  else {                              // MEDIÇÃO INVÁLIDA (geralmente quando a distância é maior do que o sensor é capaz de captar)
    AtivaLEDBuzzer(3, NULL, NULL);            // Chamada da função não precisa dos valores de distância, pois representa um erro em sua medição.
  }
}

// Função para Calibração da Distância Ideal/Desejada
bool CalibraDistancia (int address, bool calibracao){
  int lastButtonState1 = HIGH;         // Estado anterior do Botão1
  int lastButtonState2 = HIGH;         // Estado anterior do Botão2
  
  // Inicialização da medição do sensor de distância
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);

  // Caso o sensor tenha feito uma medição válida
  if (measure.RangeStatus != 4) {                 // Medição Válida
    int reading1 = digitalRead(buttonPin1);                // Lendo o estado do botão 1
    int reading2 = digitalRead(buttonPin2);                // Lendo o estado do botão 2

    // Iniciando EEPROM
    EEPROM.begin(EEPROM_SIZE);
    float dist_desejada = measure.RangeMilliMeter;              // Le o valor de distância do sensor    
    dist_desejada = dist_desejada/10;                           // Alterando o valor da distância de mm para cm

    // Piscando os LEDs verdes da barra LED (indicação de calibração em andamento)
    for (int i = 0; i< 10; i++){
      if (i > 0 & i < 5){
        digitalWrite(barraled[i], HIGH);
      }
      else {
        digitalWrite(barraled[i], LOW);
      }
    }
    delay(150);                                                
    for (int i = 0; i< 10; i++){    
      digitalWrite(barraled[i], LOW);
    }
    delay(150);

    // Checando se algum botão foi pressionado
    if (reading1 != lastButtonState1){                   // Botão 1 (REALIZA CALIBRAÇÃO)--> A calibração é desejada
      // Escrevendo os dados na EEPROM
      EEPROM.writeFloat(address, dist_desejada);
      EEPROM.commit();
      calibracao = true;                                // Botão pressionado, variável booleana recebe true
    }
    else if (reading2 != lastButtonState2){                   // Botão 2 (PULA CALIBRAÇÃO) --> A calibração não é desejada
      // Reading data from EEPROM --> Verificando se já existe alguma distância válida salva na memória EEPROM
      float readDistancia_EEPROM;
      EEPROM.get(address, readDistancia_EEPROM);            

      if (readDistancia_EEPROM < 10) {       // Caso não haja um valor de distância já salvo na EEPROM, ou o valor seja menor que 10cm (considerado muito baixo para o caso específico deste código), usa-se uma distância padrão de 15cm
        // Escrevendo novos dados na EEPROM
        float dist_padrao = 15;
        EEPROM.writeFloat(address, dist_padrao);
        EEPROM.commit();
      }
      else{                      // Caso haja uma distância válida pré calibrada na memória, o valor é mantido
        // Nada precisa ser feito
      }
      calibracao = true;                            // Botão pressionado, variável booleana recebe true
    }
    EEPROM.end();
  }
  return calibracao;
}

// Função para ativar respostas ao usuário (Barra LED e Buzzer)
void AtivaLEDBuzzer (int novaSituacao, float dist_atual, float read_dist){
  if (novaSituacao == 0){                       // Distância muito LONGE do ponto analisado
    // Ativando Barra LED
    for (int i = 0; i < 10; i++){
      if (i >= 0){
        digitalWrite(barraled[i], HIGH);   
      }
    }
    // Ativando buzzer
    digitalWrite(buzzer, HIGH);
    delay(500);
    digitalWrite(buzzer, LOW);
  }
  else if (novaSituacao == 1){                  // Distância muito PERTO do ponto analisado
    // Ativando Barra LED
    for (int i = 0; i < 10; i++){
      digitalWrite(barraled[i], LOW);           
      if (dist_atual < (read_dist-6) || dist_atual < 4){
        if (i >= 9){
          digitalWrite(barraled[i], HIGH);   
        }
      }
      else if (dist_atual < (read_dist-5)){
        if (i >= 8){
          digitalWrite(barraled[i], HIGH);     
        }       
      }
      else if (dist_atual < (read_dist-4)){
        if (i >= 7){
          digitalWrite(barraled[i], HIGH);     
        }      
      }
      else if (dist_atual < (read_dist-3)){
        if (i >= 6){
          digitalWrite(barraled[i], HIGH);     
        }      
      }
      else{    
        if (i >= 5){
          digitalWrite(barraled[i], HIGH);   
        }      
      }
    }
    // Ativando buzzer
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);
    delay(200);
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);
  }
  else if (novaSituacao == 2){                  // Distância IDEAL em relação ao ponto analisado
    // Ativando Barra LED
    for (int i = 0; i < 10; i++){
      digitalWrite(barraled[i], LOW);    
      if (dist_atual < (read_dist-1)){
        if (i >= 4){
          digitalWrite(barraled[i], HIGH);
        } 
      }
      else if (dist_atual < read_dist){
        if (i >= 3){
          digitalWrite(barraled[i], HIGH);
        }      
      }
      else if (dist_atual < (read_dist+1)) {
        if (i >= 2){
          digitalWrite(barraled[i], HIGH);
        }      
      }
      else {
        if (i >= 1){
          digitalWrite(barraled[i], HIGH);
        }   
      }
    }
    delay(500);
  }
  else if(novaSituacao == 3){                  // Medição inválida (geralmente quando a distância é maior do que o sensor é capaz de captar)
    // Ativando Barra LED
    for (int i = 0; i < 10; i++){
      digitalWrite(barraled[i], LOW);     
      if (i == 0){
        digitalWrite(barraled[i], HIGH);
      }
    }
    delay(500);
  }
}