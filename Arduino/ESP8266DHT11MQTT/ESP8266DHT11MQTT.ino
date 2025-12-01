// Librerías necesarias para la funcionalidad del ESP8266 y los sensores
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// --- Configuración del hardware ---

// Define el tipo de sensor DHT que estás usando (DHT11 o DHT22)
#define DHTTYPE DHT11 
// Define el pin donde está conectado el sensor DHT. D4 en la mayoría de las placas ESP8266 es el pin 2.
#define DHTPin D3 

// Inicializa una instancia del sensor DHT
DHT dht(DHTPin, DHTTYPE); 

// --- Configuración de la red y el servidor MQTT ---

// Reemplaza con el nombre de tu red WiFi
const char* ssid = "MOVISTAR WIFI4497";
// Reemplaza con la contraseña de tu red WiFi
const char* password = "689ms4283kxz7ge";
// Reemplaza con la dirección IP o dominio de tu servidor MQTT
const char* mqtt_server = "test.mosquitto.org"; 

// Cliente WiFi para la conexión del ESP8266
WiFiClient espClient;
// Cliente MQTT que utiliza el cliente WiFi
PubSubClient client(espClient);

// --- Variables para el control de tiempo no bloqueante ---

// Guarda el último tiempo de publicación de datos
unsigned long lastMsg = 0;
// Intervalo de tiempo para enviar datos (en milisegundos), 10 segundos.
const unsigned long publishInterval = 10000; 

// Guarda el último tiempo de intento de reconexión a MQTT
unsigned long lastReconnectAttempt = 0;

// --- Funciones principales ---

// Maneja los mensajes entrantes del servidor MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido en el topico [");
  Serial.print(topic);
  Serial.print("]: ");
  
  // Imprime el contenido del mensaje
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Se ejecuta si la conexión MQTT se pierde para intentar reconectar
void reconnect() {
  // No bloquear el loop. Intentar reconectar cada 5 segundos.
  if (!client.connected()) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      Serial.println("Conexion MQTT perdida. Intentando reconectar...");
      
    // Genera un ID de cliente único usando la dirección MAC del ESP8266
    String clientId = "ESP8266Client-";
    clientId += String(WiFi.macAddress());

    Serial.print("Intentando conectar al broker MQTT con el ID de cliente: ");
    Serial.println(clientId);

    if (client.connect(clientId.c_str())) {
        lastReconnectAttempt = 0; // Resetea para la próxima desconexión
        Serial.println("¡Reconectado a MQTT!");
      // Publica un mensaje de confirmación al conectar
      client.publish("event", "ESP8266 conectado y publicando datos");
    } else {
        Serial.print("Fallo en la reconexion. Codigo de estado: ");
      Serial.print(client.state());
        Serial.println(". Reintentando en 5 segundos...");
    }
    }
  }
}

// Configura y establece la conexión WiFi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a la red ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("¡Conectado a la red WiFi!");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

// --- Rutinas de configuración (setup) y bucle principal (loop) ---

void setup() {
  // Inicia la comunicación serial para depuración
  Serial.begin(115200);
  // Inicia el sensor DHT
  dht.begin();
  
  // Llama a la función para conectar a WiFi
  setup_wifi();
  
  // Establece la dirección del servidor MQTT y el puerto (1883 es el puerto por defecto)
  client.setServer(mqtt_server, 1883);
  // Asigna la función de callback para manejar los mensajes MQTT
  client.setCallback(callback);
}

void loop() {
  // Si se pierde la conexión WiFi, intentar reconectar.
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }

  // Solo intentar conectar a MQTT si hay conexión WiFi
  reconnect(); // Esta función ya comprueba si client.connected() es falso
  // Permite que el cliente MQTT procese mensajes entrantes y mantenga la conexión
  client.loop();

  // Obtiene el tiempo actual
  unsigned long now = millis();
  
  // Condición para publicar datos cada 10 segundos de forma no bloqueante
  if (now - lastMsg >= publishInterval) {
    lastMsg = now;

    // Leer la humedad y la temperatura del sensor DHT11
    float humedad = dht.readHumidity();
    float temperaturaC = dht.readTemperature();

    // Verificación de errores en la lectura del sensor
    if (isnan(humedad) || isnan(temperaturaC)) {
      Serial.println("¡Error al leer del sensor DHT!");
      return;
    }

    // Convertir los valores a cadena de texto para publicar
    String tempPayload = String(temperaturaC, 2); // 2 decimales para la temperatura
    String humPayload = String(humedad, 2);   // 2 decimales para la humedad

    // Publicar valores de temperatura y humedad en sus respectivos tópicos
    client.publish("sensor/dht11/temperatura", tempPayload.c_str());
    client.publish("sensor/dht11/humedad", humPayload.c_str());
    
    // Imprime los valores en el monitor serial para depuración
    Serial.print("Humedad: ");
    Serial.print(humedad);
    Serial.println(" %");
    Serial.print("Temperatura: ");
    Serial.print(temperaturaC);
    Serial.println(" °C");
  }
}