# Estação de Monitoramento Ambiental SmartCity 🏙️🌱

Este projeto consiste em uma estação de monitoramento de qualidade do ar e estresse acústico baseada no ecossistema de Internet das Coisas (IoT). O protótipo utiliza o microcontrolador ESP32 para coletar dados ambientais em tempo real e transmiti-los via protocolo MQTT para a plataforma em nuvem Adafruit IO, alinhando-se às diretrizes do Objetivo de Desenvolvimento Sustentável 11 (ODS 11) da ONU: Cidades e Comunidades Sustentáveis.

---

## 🛠️ Arquitetura de Hardware e Sensores

O protótipo é composto pelos seguintes componentes industriais e de bancada:

* **Microcontrolador:** ESP32 (SoC com conectividade Wi-Fi de 2.4 GHz integrada).
* **Sensor Climático (BME280):** Responsável pela leitura precisa de temperatura (°C), umidade relativa do ar (%) e pressão atmosférica (convertida e calibrada para hPa). Comunicação via barramento I2C.
* **Sensor de Material Particulado (SDS011):** Utiliza o princípio de espelhamento laser (laser scattering) para mensurar a concentração de poeira e poluentes finos (PM2.5) e grossos (PM10) em ug/m³. Comunicação via Hardware Serial (UART2).
* **Sensor de Poluição Sonora (MAX9814):** Microfone de eletreto integrado a um circuito de Controle Automático de Ganho (AGC), utilizado para amostragem pico a pico de pressão acústica e conversão matemática logarítmica para Decibéis (dB).
* **Atuador de Segurança (Buzzer Ativo Piezoelétrico):** Dispositivo sonoro de alerta local acionado instantaneamente caso os limites de segurança biológica sejam ultrapassados.

---

## 🚀 Estratégia de Firmware (Garantia de Qualidade - QA)

Para mitigar problemas de bloqueio por Rate Limiting (estouro de requisições) impostos pela camada gratuita do broker Adafruit IO, o algoritmo foi desenhado sob duas cadências temporais assíncronas através da função não-bloqueante millis():

1. Ciclo de Varredura e Atuação Local (3 segundos): O hardware atualiza as leituras físicas e avalia os limiares de segurança imediatamente. Se o material particulado estiver acima de 25 ug/m³, a umidade abaixo de 30% ou o ruído acima de 62 dB, o alarme acústico apita em tempo real na bancada.
2. Ciclo de Telemetria (20 segundos): O envio do pacote de dados unificado (uplink) para os feeds do painel em nuvem ocorre estritamente a cada 20 segundos, mantendo a estabilidade da conexão e o canal livre de perdas de pacotes.

---

## 💻 Configuração e Setup do Ambiente

### 1. Pré-requisitos de Software
Certifique-se de ter a Arduino IDE instalada e configurada com o pacote de placas da família ESP32. É necessário instalar as seguintes bibliotecas através do Gerenciador de Bibliotecas (Ctrl + Shift + I):

* Wire e SPI (Nativas)
* Adafruit BME280 Library (por Adafruit)
* Adafruit Unified Sensor (por Adafruit)
* Nova Fitness Sds dust sensors library (por Team Nova Fitness / GuL)
* Adafruit IO Arduino (por Adafruit)

### 2. Configuração das Credenciais do Código
Abra o arquivo principal do firmware e localize as diretrizes de definição (#define) para atualizar com os dados da sua infraestrutura local e da sua conta na nuvem:

````cpp
// ====== CREDENCIAIS WI-FI ======
#define WIFI_SSID     "INSIRA_O_NOME_DO_SEU_WIFI"
#define WIFI_PASS     "INSIRA_A_SENHA_DO_SEU_WIFI"

// ====== CREDENCIAIS ADAFRUIT IO ======
#define IO_USERNAME   "INSIRA_SEU_USER_ADAFRUIT"
#define IO_KEY        "INSIRA_SUA_AIO_KEY_DA_ADAFRUIT"
````

### 3. Execução
Conecte o ESP32 ao computador via cabo USB estável.
Selecione a placa correta em Ferramentas > Placa > ESP32 Dev Module.
Selecione a porta COM correspondente à sua placa.
Clique em Carregar (Upload).
Abra o Monitor Serial (Ctrl + Shift + M) na velocidade de 115200 baud para acompanhar a inicialização, a conexão Wi-Fi/MQTT e as amostragens do sistema.

📊 Estratégia de Tópicos (Feeds) na Nuvem
O firmware publica os dados automaticamente nos seguintes canais mapeados dentro do painel da Adafruit IO:
temperatura: Medição climática em graus Celsius.
umidade: Percentual de umidade relativa do ar.
pressao: Pressão atmosférica convertida e normalizada para Hectopascals (hPa).
ruido: Nível de pressão sonora mensurado de forma logarítmica em Decibéis (dB).
pm25: Concentração de partículas inaláveis finas de poeira (ug/m³).
pm10: Concentração de partículas inaláveis grossas suspensas no ar (ug/m³).

Ao iniciar com sucesso, a estação emitirá um sinal sonoro de confirmação (com os tons clássicos de Star Wars no buzzer) indicando que o circuito local e a conexão com o broker MQTT estão totalmente funcionais.
