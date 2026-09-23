import json
import requests
import paho.mqtt.client as mqtt
import threading
import time

# Configurações
FIREBASE_URL_BASE = "https://monitorizacao-cave-default-rtdb.europe-west1.firebasedatabase.app/"
MQTT_BROKER = "test.mosquitto.org"
MQTT_TOPIC_DADOS = "/cave_monitorizacao/dados"
MQTT_TOPIC_COMANDOS = "/cave_monitorizacao/ventoinha"

def on_connect(client, userdata, flags, rc):
    print(f"Ligado ao MQTT (Código: {rc})")
    client.subscribe(MQTT_TOPIC_DADOS)

def on_message(client, userdata, msg):
    try:
        payload = msg.payload.decode().strip()
        if not payload: return
        
        dados = json.loads(payload)
        
        # Blindagem: Nunca deixa o Arduino mexer na chave da App
        if "Ventoinha_Forcada" in dados:
            del dados["Ventoinha_Forcada"]
            
        # PATCH na raiz para garantir que as chaves batem certo com a App
        requests.patch(FIREBASE_URL_BASE + ".json", json=dados)
        print(f"Sensores atualizados: T={dados.get('Temperatura')} H={dados.get('Humidade')}")
    except Exception as e:
        print(f"Erro ao processar MQTT: {e}")

def watch_firebase():
    last_state = None
    print("Vigilância do Firebase ativa...")
    while True:
        try:
            # Lemos apenas o valor do botão manual
            r = requests.get(FIREBASE_URL_BASE + "Ventoinha_Forcada.json")
            if r.status_code == 200:
                current_state = r.json()
                
                # Se o estado for diferente do último conhecido (e não for nulo)
                if current_state is not None and current_state != last_state:
                    msg = "true" if current_state else "false"
                    client.publish(MQTT_TOPIC_COMANDOS, msg)
                    print(f">>> COMANDO APP ENVIADO: {msg}")
                    last_state = current_state
        except Exception as e:
            print(f"Erro na vigilância: {e}")
        
        time.sleep(0.5) # Resposta mais rápida (meio segundo)

# Configuração do Cliente MQTT
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

print("A iniciar ponte bidirecional...")
try:
    client.connect(MQTT_BROKER, 1883, 60)
except:
    print("Erro ao ligar ao Broker MQTT. Verifica a internet.")

# Inicia a vigilância do Firebase numa thread separada
t = threading.Thread(target=watch_firebase, daemon=True)
t.start()

# Loop principal do MQTT
client.loop_forever()