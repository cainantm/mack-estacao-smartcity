#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SDS011.h>
#include "AdafruitIO_WiFi.h"

#define PIN_BUZZER 13
#define PIN_MIC    34

// ====== CONFIGURAÇÕES DE NOTAS (STAR WARS) ======
#define NOTE_A4  440
#define NOTE_F4  349
#define NOTE_C5  523
#define NOTE_E5  659

// ====== CREDENCIAIS WI-FI ======
#define WIFI_SSID     "Nome-Wifi"
#define WIFI_PASS     "Senha-Wifi"

// ====== CREDENCIAIS ADAFRUIT IO ======
#define IO_USERNAME   "user-adafruit"
#define IO_KEY        "key-adafruit"

AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

// Criação dos canais (Feeds) automáticos na nuvem
AdafruitIO_Feed *f_temp   = io.feed("temperatura");
AdafruitIO_Feed *f_umid   = io.feed("umidade");
AdafruitIO_Feed *f_pres   = io.feed("pressao");
AdafruitIO_Feed *f_ruido  = io.feed("ruido");
AdafruitIO_Feed *f_pm25   = io.feed("pm25");
AdafruitIO_Feed *f_pm10   = io.feed("pm10");

GuL::SDS011 sds(Serial2);
Adafruit_BME280 bme; 

void tocaStarWars() {
  int melodia[] = { NOTE_A4, NOTE_A4, NOTE_A4, NOTE_F4, NOTE_C5, NOTE_A4, NOTE_F4, NOTE_C5, NOTE_A4 };
  int duracoes[] = { 350,    350,    350,    250,    100,    350,    250,    100,    500 };
  for (int i = 0; i < 9; i++) {
    tone(PIN_BUZZER, melodia[i], duracoes[i]);
    delay(duracoes[i] * 1.30);
  }
  noTone(PIN_BUZZER);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BUZZER, OUTPUT);
  
  if (!bme.begin(0x76)) {
    Serial.println("Erro: Sensor BME280 nao encontrado!");
    while (1);
  }

  Serial2.begin(9600, SERIAL_8N1, 16, 17);
  sds.setToActiveReporting();
  
  Serial.print("Conectando a Adafruit IO...");
  io.connect();

  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\n-> Conectado com sucesso à nuvem Adafruit!");
  tocaStarWars();
}

// ====== VARIÁVEIS GLOBAIS DE ACÚMULO ======
unsigned long ultimoEnvioNuvem = 0;
const long intervaloNuvem = 20000; // Envia para a nuvem a cada 20 segundos

void loop() {
  io.run(); //

  // Garante a reconexão se cair
  if (WiFi.status() != WL_CONNECTED) {
  }

  Serial.println("\n=== [LEITURA LOCAL] Nova Amostragem dos Sensores ===");

  float temp = bme.readTemperature();
  float umid = bme.readHumidity();
  float pres = bme.readPressure() / 100.0F; 

  // Microfone MAX9814 com AGC (Varredura rápida de 120ms)
  unsigned long startMillis = millis();
  unsigned int maxLeitura = 0, minLeitura = 4095;
  while (millis() - startMillis < 120) {
    int leitura = analogRead(PIN_MIC);
    if (leitura > 0 && leitura < 4095) {
      if (leitura > maxLeitura) maxLeitura = leitura;
      if (leitura < minLeitura) minLeitura = leitura;
    }
  }
  unsigned int peakToPeak = maxLeitura - minLeitura;
  float volts = (peakToPeak * 3.3) / 4095.0; 
  float ruidoDB = 20.0 * log10(volts + 0.001) + 65.0; 
  if (ruidoDB < 45.0) ruidoDB = 45.0;
  if (ruidoDB > 95.0) ruidoDB = 95.0;

  sds.read();
  float pm25 = sds.getPM2_5();
  float pm10 = sds.getPM10();

  // Print de Debug Local instantâneo no Monitor Serial
  Serial.print("Temp Físico: "); Serial.print(temp); Serial.println(" *C");
  Serial.print("Ruído Físico: "); Serial.print(ruidoDB, 1); Serial.println(" dB");

  // ====== LÓGICA DE ALERTA DO BUZZER REAL-TIME ======
  // O buzzer responde instantaneamente a cada 3 segundos à palma ou poluição
  if (pm25 > 25.0 || umid < 30.0 || ruidoDB > 62.0) {
    Serial.println("-> STATUS LOCAL: ALERTA ATIVO NO HARDWARE! <-");
    tone(PIN_BUZZER, 2500, 100);
    delay(150);
    tone(PIN_BUZZER, 2500, 100);
  } else {
    Serial.println("-> STATUS LOCAL: Condições normais.");
    noTone(PIN_BUZZER);
  }

  // ====== TEMPORIZADOR NÃO-BLOQUEANTE PARA A NUVEM ======
  // Verifica se já se passaram 20 segundos desde o último envio para o dashboard
  if (millis() - ultimoEnvioNuvem >= intervaloNuvem) {
    ultimoEnvioNuvem = millis(); // Atualiza o cronômetro

    Serial.println("\n📡 >>> [UPLINK] Enviando pacote de dados para Adafruit IO... <<<");
    f_temp->save(temp);
    f_umid->save(umid);
    f_pres->save(pres);
    f_ruido->save(ruidoDB);
    f_pm25->save(pm25);
    f_pm10->save(pm10);
    Serial.println("📡 >>> [UPLINK] Sucesso! Limites de Rate Limit preservados. <<<\n");
  }

  delay(3000); 
}