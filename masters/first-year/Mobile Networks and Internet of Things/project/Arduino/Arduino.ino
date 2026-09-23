#include <MKRNB.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHTPIN A2
#define DHTTYPE DHT11

// --- PINOS ---
const int MQ135_PIN = A1;
const int TRIG_PIN = 3;
const int ECHO_PIN = 4;
const int BUZZER_PIN = 5;
const int FAN_PIN = 2;
const int LED_GREEN = A6;
const int LED_RED = A5;

DHT dht(DHTPIN, DHTTYPE);

// --- REDE E MQTT ---
const char PINNUMBER[] = "";
NB nbAccess;
GPRS gprs;
NBClient nbClient;
PubSubClient client(nbClient);

const char* mqtt_server = "test.mosquitto.org";
const char* topicDados = "/cave_monitorizacao/dados";
const char* topicComandos = "/cave_monitorizacao/ventoinha"; 

// --- LIMITES ---
float maxTemp = 26.0;
float maxHum = 70.0;
int gasThreshold = 550;

// --- CONFIGURAÇÃO DA CAIXA DE SAPATOS ---
float distanciaCheio = 2.0;    
float distanciaVazio = 15.0;   
int capacidadeMaxLitros = 5;   

// Variável que guarda o comando manual vindo da App
bool ventoinhaForcadaApp = false;

float readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1;
  return duration * 0.0343 / 2.0;
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  
  Serial.print(">>> COMANDO RECEBIDO DA APP: ");
  Serial.println(msg);

  if (String(topic) == topicComandos) {
    ventoinhaForcadaApp = (msg == "true");
    // Atualiza o pino imediatamente para resposta instantânea
    bool t_temp = dht.readTemperature();
    int g_temp = analogRead(MQ135_PIN);
    bool ligarAgora = (t_temp > maxTemp || g_temp > gasThreshold || ventoinhaForcadaApp);
    digitalWrite(FAN_PIN, ligarAgora ? HIGH : LOW);
  }
}

void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(FAN_PIN, OUTPUT); pinMode(LED_GREEN, OUTPUT); pinMode(LED_RED, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT); pinMode(ECHO_PIN, INPUT);

  Serial.println("A ligar rede...");
  while (nbAccess.begin(PINNUMBER) != NB_READY || gprs.attachGPRS() != GPRS_READY) {
    delay(2000);
  }
  Serial.println("Ligado!");
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  // 1. Verifica ligação ao MQTT (Essencial para receber comandos da App)
  if (!client.connected()) {
    if (client.connect("arduino_cave_diogo_final")) {
        client.subscribe(topicComandos);
    }
  }
  
  // 2. Ouve o MQTT constantemente (Sem isto a ventoinha não liga na hora)
  client.loop(); 

  // 3. Temporizador inteligente: Só lê sensores e envia dados a cada 5 segundos
  // Substitui o delay(5000) por esta lógica:
  static unsigned long ultimaVez = 0;
  if (millis() - ultimaVez >= 5000) {
    ultimaVez = millis();

    float t = dht.readTemperature();
    float h = dht.readHumidity();
    int gas = analogRead(MQ135_PIN);
    float dist = readDistanceCM();

    // --- CÁLCULO DE LÍQUIDO ---
    int nivelPercentagem = 0;
    int nivelLitros = 0;
    if (dist > 0) {
      nivelPercentagem = map((int)(dist * 10), (int)(distanciaVazio * 10), (int)(distanciaCheio * 10), 0, 100);
      nivelPercentagem = constrain(nivelPercentagem, 0, 100);
      nivelLitros = (nivelPercentagem * capacidadeMaxLitros) / 100;
    }

    // --- LÓGICA DE AVISOS ---
    String avisos = "";
    bool alertaAtivo = false;
    if (!isnan(t) && t > maxTemp) { avisos += "Temp. Alta! "; alertaAtivo = true; }
    if (!isnan(h) && h > maxHum) { avisos += "Humidade Alta! "; alertaAtivo = true; }
    if (gas > gasThreshold) { avisos += "Gas Detetado! "; alertaAtivo = true; }
    if (nivelPercentagem < 20) { avisos += "Reserva Baixa! "; alertaAtivo = true; }
    if (avisos == "") avisos = "Tudo normal na cave.";

    // --- LÓGICA VENTOINHA ---
    bool condicaoAuto = (t > maxTemp || gas > gasThreshold);
    bool ligarVentoinha = (condicaoAuto || ventoinhaForcadaApp);
    digitalWrite(FAN_PIN, ligarVentoinha ? HIGH : LOW);

    // --- MONTAGEM E ENVIO DO JSON ---
    String payload = "{";
    payload += "\"Temperatura\":" + String(isnan(t) ? 0 : t, 1) + ",";
    payload += "\"Humidade\":" + String(isnan(h) ? 0 : h, 1) + ",";
    payload += "\"Nivel_Cuba_Litros\":" + String(nivelLitros) + ",";
    payload += "\"Nivel_Cuba_Percentagem\":" + String(nivelPercentagem) + ",";
    payload += "\"Qualidade_Ar_Segura\":" + String(gas <= gasThreshold ? "true" : "false") + ",";
    payload += "\"Ventoinha_Estado\":\"" + String(ligarVentoinha ? "Ventoinha em andamento" : "Ventoinha parada") + "\",";
    payload += "\"Avisos\":\"" + avisos + "\"";
    payload += "}";

    Serial.println("Dados enviados: " + payload);
    client.publish(topicDados, payload.c_str());

    digitalWrite(LED_RED, alertaAtivo ? HIGH : LOW);
    digitalWrite(LED_GREEN, alertaAtivo ? LOW : HIGH);
  }
}