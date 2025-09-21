// ===================== INFORMAÇÕES DO PROJETO =====================
/*
Integrantes:
- Alexandre Wesley - 561622
- João Stellare - 565813
- Kauê de Almeida Pena - 564211

Grupo: Data-Vine
Turma: 1ESPF
*/ 

#include <Wire.h>
#include <MPU6050.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ===================== CONFIGURAÇÕES =====================
// Credenciais de rede WiFi
const char* default_SSID = "Wokwi-GUEST"; 
const char* default_PASSWORD = "";        

// Configuração do Broker MQTT
const char* default_BROKER_MQTT = "20.49.4.108"; 
const int   default_BROKER_PORT = 1883;         

// Definição dos tópicos MQTT para publicação e assinatura
const char* default_TOPICO_SUBSCRIBE = "/TEF/des001/cmd"; 
const char* default_TOPICO_BPM = "/TEF/des001/attrs/bpm";
const char* default_TOPICO_SPO2 = "/TEF/des001/attrs/spo2";
const char* default_TOPICO_VELOCIDADEX = "/TEF/des001/attrs/velocidadeX";
const char* default_TOPICO_VELOCIDADEY = "/TEF/des001/attrs/velocidadeY";
const char* default_TOPICO_VELOCIDADEZ = "/TEF/des001/attrs/velocidadeZ";
const char* default_ID_MQTT = "des001";

// Conversão para variáveis mutáveis
char* SSID = const_cast<char*>(default_SSID);
char* PASSWORD = const_cast<char*>(default_PASSWORD);
char* BROKER_MQTT = const_cast<char*>(default_BROKER_MQTT);
int BROKER_PORT = default_BROKER_PORT;
char* TOPICO_SUBSCRIBE = const_cast<char*>(default_TOPICO_SUBSCRIBE);
char* TOPICO_BPM = const_cast<char*>(default_TOPICO_BPM);
char* TOPICO_SPO2 = const_cast<char*>(default_TOPICO_SPO2);
char* TOPICO_VELOCIDADEX = const_cast<char*>(default_TOPICO_VELOCIDADEX);
char* TOPICO_VELOCIDADEY = const_cast<char*>(default_TOPICO_VELOCIDADEY);
char* TOPICO_VELOCIDADEZ = const_cast<char*>(default_TOPICO_VELOCIDADEZ);
char* ID_MQTT = const_cast<char*>(default_ID_MQTT);

// ===================== PINOS =====================
#define DHTPIN 1       // Pino conectado ao sensor DHT22
#define DHTTYPE DHT22  // Tipo do sensor
#define SDA_PIN 5      // Pino SDA do MPU6050
#define SCL_PIN 6      // Pino SCL do MPU6050
#define MPU_INT_PIN 4  // Pino de interrupção (não usado diretamente)

// ===================== SENSORES =====================
DHT dht(DHTPIN, DHTTYPE); // Objeto para o sensor DHT22
MPU6050 mpu;              // Objeto para o acelerômetro MPU6050

// ===================== MQTT =====================
WiFiClient espClient;
PubSubClient MQTT(espClient); // Cliente MQTT

// ===================== VARIÁVEIS =====================
#define periodoDeRegistro 1000 // Intervalo de publicação em ms

uint32_t ultimoRegistro = 0;
int mediaBPM = 0;
int mediaSPO2 = 0;

// Variáveis do MPU para cálculos de aceleração/velocidade
int16_t ax, ay, az;
int16_t gx, gy, gz;
float velocidadeX = 0, velocidadeY = 0, velocidadeZ = 0;
unsigned long ultimoTempoMPU = 0;

// ===================== SETUP =====================
void setup() {
  Serial.begin(115200);

  // Conexão com o WiFi
  WiFi.begin(default_SSID, default_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { delay(200); Serial.print("."); }
  Serial.println("\nWiFi conectado!");
  Serial.print("IP: "); Serial.println(WiFi.localIP());

  // Conexão com o Broker MQTT
  MQTT.setServer(default_BROKER_MQTT, default_BROKER_PORT);
  reconnectMQTT();

  // Inicialização dos sensores
  dht.begin();
  Wire.begin(SDA_PIN, SCL_PIN);

  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("Falha ao conectar MPU6050!");
    while(1); // trava se falhar
  }
  Serial.println("MPU6050 conectado com sucesso!");

  ultimoTempoMPU = millis(); // Marca o tempo inicial
}

// ===================== LOOP =====================
void loop() {
  // Mantém conexão com o MQTT
  if (!MQTT.connected()){
    reconnectMQTT();
  }
  MQTT.loop();

  // Faz leituras dos sensores
  leituraSensor(); // BPM/SpO2
  leituraMPU();    // Velocidade (X, Y, Z)
}

// ===================== FUNÇÃO AUXILIAR =====================
// Reconecta ao broker caso a conexão caia
void reconnectMQTT() {
  while (!MQTT.connected()) {
    Serial.print("* Tentando se conectar ao Broker MQTT: ");
    Serial.println(default_BROKER_MQTT);

    if (MQTT.connect(ID_MQTT)) {
      Serial.println("Conectado ao broker!");
      MQTT.subscribe(TOPICO_SUBSCRIBE); // Inscreve no tópico de comando
    } else {
      Serial.println("Falha na conexão, tentando em 2s...");
      delay(2000);
    }
  }
}

// ===================== LEITURA SENSOR DHT (Simulado) =====================
// Gera valores aleatórios para simular leitura de BPM e SpO2
void leituraSensor() {
  if (millis() - ultimoRegistro > periodoDeRegistro) {
    int somaBPM = 0, somaSpO2 = 0;

    // Média de 10 leituras simuladas
    for (int i = 0; i < 10; i++) {
      somaBPM  += random(60, 100);  // BPM realista
      somaSpO2 += random(90, 100);  // SpO2 realista
      delay(100);
    }

    mediaBPM = somaBPM / 10;
    mediaSPO2 = somaSpO2 / 10;

    // Mostra no Serial Monitor
    Serial.print("BPM: "); Serial.print(mediaBPM);
    Serial.print(" / SpO2: "); Serial.println(mediaSPO2);
    delay(1000);

    // Publica os dados em tópicos separados
    String sp = String(mediaSPO2);
    String bp = String(mediaBPM);
    MQTT.publish(TOPICO_BPM, sp.c_str());   // Publica SpO2
    MQTT.publish(TOPICO_SPO2, bp.c_str());  // Publica BPM

    ultimoRegistro = millis();
  }
}

// ===================== LEITURA MPU =====================
// Lê aceleração e calcula velocidade em X, Y e Z
void leituraMPU() {
  unsigned long agora = millis();
  float dt = (agora - ultimoTempoMPU) / 1000.0; // Tempo em segundos
  ultimoTempoMPU = agora;

  mpu.getAcceleration(&ax, &ay, &az);

  // Converte para m/s²
  float accelX = ax / 16384.0 * 9.8;
  float accelY = ay / 16384.0 * 9.8;
  float accelZ = az / 16384.0 * 9.8;

  // Integração simples (v = v0 + a*dt)
  velocidadeX += accelX * dt;
  velocidadeY += accelY * dt;
  velocidadeZ += accelZ * dt;

  // Exibe valores no Serial Monitor
  Serial.print("Velocidade X: "); Serial.println(velocidadeX);
  Serial.print("Velocidade Y: "); Serial.println(velocidadeY);
  Serial.print("Velocidade Z: "); Serial.println(velocidadeZ);

  delay(1000);

  // Publica velocidades em tópicos separados
  String velX = String(velocidadeX);
  String velY = String(velocidadeY);
  String velZ = String(velocidadeZ);
  MQTT.publish(TOPICO_VELOCIDADEX, velX.c_str());
  MQTT.publish(TOPICO_VELOCIDADEY, velY.c_str());
  MQTT.publish(TOPICO_VELOCIDADEZ, velZ.c_str());

  // Zera velocidades (reset para próxima medição)
  velocidadeZ = 0;
  velocidadeY = 0;
  velocidadeX = 0;
}
