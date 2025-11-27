# Importa las librerías necesarias
import paho.mqtt.client as mqtt
import time
import random

# --- Configuración del broker MQTT ---
MQTT_SERVER = "test.mosquitto.org"  # Broker público para pruebas
MQTT_PORT = 1883                      # Puerto MQTT estándar
MQTT_TOPIC = "sensor/mq2/gas_level"   # Tópico para el nivel de gas
CLIENT_ID = "PythonSimMQ2"            # ID único para el cliente

# Crea una instancia del cliente MQTT
client = mqtt.Client(client_id=CLIENT_ID)

# --- Funciones de callback del cliente MQTT ---

def on_connect(client, userdata, flags, rc):
    """Callback que se ejecuta al conectar al broker."""
    if rc == 0:
        print("Conectado exitosamente al broker MQTT.")
    else:
        print(f"Fallo en la conexión. Código de error: {rc}")

def on_disconnect(client, userdata, rc):
    """Callback que se ejecuta al desconectar del broker."""
    print("Desconectado del broker MQTT.")

client.on_connect = on_connect
client.on_disconnect = on_disconnect

# --- Bucle principal de simulación ---
try:
    print(f"Intentando conectar a {MQTT_SERVER}:{MQTT_PORT}...")
    client.connect(MQTT_SERVER, MQTT_PORT, 60)
    client.loop_start() # Inicia el bucle de red en segundo plano

    while True:
        # Simula la lectura de un sensor MQ2 (valor entre 200 y 1000, por ejemplo)
        gas_level = random.randint(200, 1000)
        payload = str(gas_level)

        # Publica el valor en el tópico MQTT
        client.publish(MQTT_TOPIC, payload, qos=1)
        print(f"Publicado -> Nivel de gas: {gas_level} en '{MQTT_TOPIC}'")

        time.sleep(5) # Espera 5 segundos antes de la siguiente lectura

except KeyboardInterrupt:
    print("\nSimulación terminada por el usuario.")
except Exception as e:
    print(f"Ocurrió un error inesperado: {e}")
finally:
    client.loop_stop()
    client.disconnect()
    print("Cliente MQTT desconectado y programa finalizado.")