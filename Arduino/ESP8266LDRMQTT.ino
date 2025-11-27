// Librerías necesarias para la funcionalidad del ESP8266
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// --- Configuración del hardware ---

// Define el pin analógico donde está conectado el sensor LDR.
// El ESP8266 tiene un solo pin analógico, A0.
#define LDR_PIN A0

// --- Configuración de la red y el servidor MQTT ---

// Reemplaza con el nombre de tu red WiFi
const char* ssid = "NOMBRE_DE_TU_WIFI";
// Reemplaza con la contraseña de tu red WiFi
const char* password = "PASSWORD_DE_TU_WIFI";
// Reemplaza con la dirección IP o dominio de tu servidor MQTT
const char* mqtt_server = "IP_DEL_SERVIDOR_MQTT";

// Cliente WiFi para la conexión del ESP8266
WiFiClient espClient;
// Cliente MQTT que utiliza el cliente WiFi
PubSubClient client(espClient);

// --- Variables para el control de tiempo no bloqueante ---

// Guarda el último tiempo de publicación de datos
unsigned long lastMsg = 0;
// Intervalo de tiempo para enviar datos (en milisegundos), 5 segundos
const unsigned long publishInterval = 5000;

// --- Funciones principales ---

// Maneja los mensajes entrantes del servidor MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido en el tópico [");
  Serial.print(topic);
  Serial.print("]: ");
  
  // Imprime el contenido del mensaje
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
}

// Se ejecuta si la conexión MQTT se pierde para intentar reconectar
void reconnect() {
  while (!client.connected()) {
    Serial.println("Conexión MQTT perdida. Intentando reconectar...");
    
    // Intenta conectar con un ID de cliente único.
    if (client.connect("ESP8266_LDR_Client")) {
      Serial.println("¡Reconectado a MQTT!");
      // Publica un mensaje de confirmación al conectar
      client.publish("status/ldr", "ESP8266 LDR conectado");
      // Puedes suscribirte a un tópico para recibir comandos, si es necesario
      // client.subscribe("cmd/ldr");
    } else {
      Serial.print("Fallo en la reconexión. Código de estado: ");
      Serial.print(client.state());
      Serial.println(". Reintentando en 5 segundos...");
      // Espera 5 segundos antes de reintentar
      delay(5000);
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
  
  // Llama a la función para conectar a WiFi
  setup_wifi();
  
  // Establece la dirección del servidor MQTT y el puerto (1883 es el por defecto)
  client.setServer(mqtt_server, 1883);
  // Asigna la función de callback para manejar los mensajes MQTT
  client.setCallback(callback);
}

void loop() {
  // Asegura que el cliente MQTT se mantenga conectado
  if (!client.connected()) {
    reconnect();
  }
  // Permite que el cliente MQTT procese mensajes entrantes y mantenga la conexión
  client.loop();

  // Obtiene el tiempo actual
  unsigned long now = millis();
  
  // Condición para publicar datos cada 'publishInterval' milisegundos
  if (now - lastMsg >= publishInterval) {
    lastMsg = now;

    // Leer el valor del sensor LDR (devuelve un valor entre 0 y 1023)
    int ldrValue = analogRead(LDR_PIN);

    // Opcional: Convertir el valor a un porcentaje de luz (0-100%)
    // Invertimos el valor porque un LDR típico da valores altos en oscuridad y bajos con luz.
    float lightPercent = (1023 - ldrValue) * 100.0 / 1023.0;

    // Convertir el valor a cadena de texto para publicar
    String payload = String(lightPercent, 1); // 1 decimal

    // Publicar el valor en el tópico MQTT
    client.publish("ldr/light_percent", payload.c_str());
    
    // Imprime los valores en el monitor serial para depuración
    Serial.print("Valor LDR crudo: ");
    Serial.println(ldrValue);
    Serial.print("Nivel de luz: ");
    Serial.print(payload);
    Serial.println(" %");
  }
}