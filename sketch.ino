// Integrantes:
/*
Alexandre Wesley - 561622
João Stellare - 565813
Kauê de Almeida Pena - 564211
*/
// Grupo: Data-Vine
// Turma: 1ESPF

#include <Wire.h>
#include <MPU6050.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ===================== CONFIGURAÇÕES =====================
const char* default_SSID = "Wokwi-GUEST"; 
const char* default_PASSWORD = "";        
const char* default_BROKER_MQTT = "20.49.4.108"; 
const int   default_BROKER_PORT = 1883;         

// Tópicos MQTT
const char* TOPICO_SUBSCRIBE = "/TEF/des001/cmd"; 
const char* TOPICO_BPM = "/TEF/des001/attrs/bpm";
const char* TOPICO_SPO2 = "/TEF/des001/attrs/spo2";
const char* TOPICO_VELOCIDADEX = "/TEF/des001/attrs/velocidadeX";
const char* TOPICO_VELOCIDADEY = "/TEF/des001/attrs/velocidadeY";
const char* TOPICO_VELOCIDADEZ = "/TEF/des001/attrs/velocidadeZ";
const char* ID_MQTT = "des001"; 

// ===================== PINOS =====================
#define DHTPIN 1
#define DHTTYPE DHT22
#define SDA_PIN 5  
#define SCL_PIN 6   
#define MPU_INT_PIN 4

// ===================== SENSORES =====================
DHT dht(DHTPIN, DHTTYPE);
MPU6050 mpu;

// ===================== MQTT =====================
WiFiClient espClient;
PubSubClient MQTT(espClient);

// ===================== VARIÁVEIS =====================
#define periodoDeRegistro 1000

uint32_t ultimoRegistro = 0;
int mediaBPM = 0;
int mediaSPO2 = 0;

// Variáveis MPU
int16_t ax, ay, az;
int16_t gx, gy, gz;
float velocidadeX = 0, velocidadeY = 0, velocidadeZ = 0;
unsigned long ultimoTempoMPU = 0;


// ===================== SETUP =====================
void setup() {
  Serial.begin(115200);

  // Inicializa WiFi
  WiFi.begin(default_SSID, default_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { delay(200); Serial.print("."); }
  Serial.println("\nWiFi conectado!");
  Serial.print("IP: "); Serial.println(WiFi.localIP());

  // Inicializa MQTT
  MQTT.setServer(default_BROKER_MQTT, default_BROKER_PORT);
  reconnectMQTT();

  // Inicializa sensores
  dht.begin();
  Wire.begin(SDA_PIN, SCL_PIN);

  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("Falha ao conectar MPU6050!");
    while(1);
  }
  Serial.println("MPU6050 conectado com sucesso!");

  ultimoTempoMPU = millis();
}

// ===================== LOOP =====================
void loop() {
  if (!MQTT.connected()){
    reconnectMQTT();
  }
  MQTT.loop();

  leituraSensor(); // Simula BPM/SpO2
  leituraMPU();    // Lê aceleração e calcula velocidade
}

// ===================== FUNÇÃO AUXILIAR =====================
void reconnectMQTT() {
  while (!MQTT.connected()) {
    Serial.print("* Tentando se conectar ao Broker MQTT: ");
    Serial.println(default_BROKER_MQTT);

    if (MQTT.connect(ID_MQTT)) {
      Serial.println("Conectado ao broker!");
      MQTT.subscribe(TOPICO_SUBSCRIBE);
    } else {
      Serial.println("Falha na conexão, tentando em 2s...");
      delay(2000);
    }
  }
}

// ===================== LEITURA SENSOR DHT (Simulado) =====================
void leituraSensor() {
  if (millis() - ultimoRegistro > periodoDeRegistro) {
    int somaBPM = 0, somaSpO2 = 0;

    for (int i = 0; i < 10; i++) {
      somaBPM  += random(60, 100);  // Simula BPM realista
      somaSpO2 += random(90, 100);  // Simula SpO2 realista
      delay(100);
    }

    mediaBPM = somaBPM / 10;
    mediaSPO2 = somaSpO2 / 10;

    // Exibe no Serial
    Serial.print("BPM: "); Serial.print(mediaBPM);
    Serial.print(" / SpO2: "); Serial.println(mediaSPO2);
    delay(1000);

    // Publica no MQTT
    char payload[100];
    snprintf(payload, sizeof(payload), "{\"mediaBPM\":%d,\"mediaSPO2\":%d}", mediaBPM, mediaSPO2);
    String sp = String(mediaSPO2);
    String bp = String(mediaBPM);
    MQTT.publish(TOPICO_BPM, sp.c_str());
    MQTT.publish(TOPICO_SPO2, bp.c_str());

    ultimoRegistro = millis();
  }
}

// ===================== LEITURA MPU =====================
void leituraMPU() {
  unsigned long agora = millis();
  float dt = (agora - ultimoTempoMPU) / 1000.0;
  ultimoTempoMPU = agora;

  mpu.getAcceleration(&ax, &ay, &az);

  // Converte para m/s²
  float accelX = ax / 16384.0 * 9.8;
  float accelY = ay / 16384.0 * 9.8;
  float accelZ = az / 16384.0 * 9.8;

  // Integração simples para velocidade
  velocidadeX += accelX * dt;
  velocidadeY += accelY * dt;
  velocidadeZ += accelZ * dt;

  // Exibe no Serial
  Serial.print("Velocidade X: "); Serial.println(velocidadeX);
  Serial.print("Velocidade Y: "); Serial.println(velocidadeY);
  Serial.print("Velocidade Z: "); Serial.println(velocidadeZ);

  delay(1000);

  // Publica no MQTT
  char payload[150];
  snprintf(payload, sizeof(payload),
           "{\"velocidadeX\":%.2f,\"velocidadeY\":%.2f,\"velocidadeZ\":%.2f}",
           velocidadeX, velocidadeY, velocidadeZ);
  String velX = String(velocidadeX);
  String velY = String(velocidadeY);
  String velZ = String(velocidadeZ);
  MQTT.publish(TOPICO_VELOCIDADEX, velX.c_str());
  MQTT.publish(TOPICO_VELOCIDADEY, velY.c_str());
  MQTT.publish(TOPICO_VELOCIDADEZ, velZ.c_str());
  velocidadeZ = 0;
  velocidadeY = 0;
  velocidadeX = 0;
}
