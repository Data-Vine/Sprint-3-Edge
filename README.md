# Sprint-3-Edge
📌 Descrição

Este projeto simula um sistema de monitoramento remoto utilizando ESP32, DHT22 e MPU6050, com envio de dados via MQTT. Ele captura:

BPM (batimentos por minuto) e SpO₂ (oxigenação do sangue) através de simulação do DHT22.

Aceleração nos eixos X, Y, Z com o MPU6050, calculando a velocidade integrando a aceleração.

Os dados são publicados em tópicos MQTT distintos para monitoramento em tempo real.

🛠 Componentes Utilizados

ESP32 – microcontrolador principal.

DHT22 – sensor de temperatura e umidade (usado para simular BPM e SpO₂).

MPU6050 – sensor de aceleração e giroscópio.

WiFi – conexão à rede local.

PubSubClient – biblioteca para comunicação MQTT.

🔌 Conexões
DHT22
Pino	Conexão
VCC	3.3V
GND	GND
DATA	GPIO 1
MPU6050
Pino	Conexão
VCC	3.3V
GND	GND
SDA	GPIO 5
SCL	GPIO 6
INT	GPIO 4
⚙️ Configurações
WiFi
const char* default_SSID = "Wokwi-GUEST";
const char* default_PASSWORD = "";

MQTT
const char* default_BROKER_MQTT = "20.246.40.8";
const int default_BROKER_PORT = 1883;
const char* TOPICO_BPM_SPO2 = "/TEF/des001/attrs";
const char* TOPICO_VELOCIDADE = "/TEF/des001/attrs/velocidade";

📦 Bibliotecas Necessárias

Wire.h

MPU6050.h

WiFi.h

PubSubClient.h

DHT.h

⚡ Funcionalidades

Leitura de sensores DHT22: simula BPM (60–100 bpm) e SpO₂ (90–100%).

Leitura do MPU6050: captura aceleração nos eixos X, Y, Z.

Cálculo de velocidade: integração simples da aceleração em cada eixo.

Publicação MQTT:

/TEF/des001/attrs → BPM e SpO₂.

/TEF/des001/attrs/velocidade → velocidade X/Y/Z.

Monitoramento Serial: exibe todos os valores em tempo real.

📡 Fluxo de Dados

ESP32 lê sensores a cada 1 segundo.

Calcula média de BPM/SpO₂ (10 leituras).

Integra aceleração para estimativa de velocidade.

Publica valores nos tópicos MQTT correspondentes.

Pode ser consumido via broker MQTT ou plataformas como FIWARE/NGSI-LD ou STH-Comet.

🚀 Como Testar

Abra o projeto no Wokwi
. https://wokwi.com/projects/442194358480221185

Clique em Start Simulation.

Abra o Serial Monitor para verificar leituras de BPM, SpO₂ e velocidades.

Confira os dados publicados no broker MQTT, caso tenha acesso a um.

🧑‍💻 Integrantes

Alexandre Wesley – 561622

João Stellare – 565813

Kauê de Almeida Pena – 564211
