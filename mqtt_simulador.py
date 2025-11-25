# Importa las librerías necesarias
import paho.mqtt.client as mqtt
import time
import random

# --- Configuración del broker MQTT público ---
MQTT_SERVER = "test.mosquitto.org"  # Broker público online
MQTT_PORT = 1883                      # Puerto MQTT normal
MQTT_TOPIC = "luz/sensorldr"         # Tópico donde se publicarán los datos
CLIENT_ID = "PythonSimLDROnline"      # ID del cliente

# Crea una instancia del cliente MQTT
client = mqtt.Client(client_id=CLIENT_ID)

# --- Funciones del cliente MQTT ---

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Conectado exitosamente al broker MQTT público.")
    else:
        print(f"Fallo en la conexión. Código de error: {rc}")

def on_disconnect(client, userdata, rc):
    print("Desconectado del broker MQTT.")

client.on_connect = on_connect
client.on_disconnect = on_disconnect

# --- Bucle de simulación ---
try:
    print(f"Intentando conectar a {MQTT_SERVER}:{MQTT_PORT}...")
    client.connect(MQTT_SERVER, MQTT_PORT, 60)
    client.loop_start()

    while True:
        # Simula lectura de un sensor LDR
        valor_ldr = random.randint(25, 900)
        payload = str(valor_ldr)

        # Publica el valor en el tópico
        result = client.publish(MQTT_TOPIC, payload, qos=1)
        print(f"Publicando en '{MQTT_TOPIC}': {payload}")
        if result.rc == mqtt.MQTT_ERR_SUCCESS:
            print("Mensaje publicado con éxito.")
        else:
            print(f"Error al publicar mensaje. Código: {result.rc}")

        time.sleep(2)

except KeyboardInterrupt:
    print("Simulación terminada por el usuario.")
except Exception as e:
    print(f"Ocurrió un error: {e}")
finally:
    client.loop_stop()
    client.disconnect()
