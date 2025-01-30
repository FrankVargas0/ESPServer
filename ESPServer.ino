#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>
#include <WebSocketsServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESPmDNS.h>
// Credenciales WiFi
const char *ssid = "Nombre de red wifi";
const char *password = "Conntraseña de la red";
// DHT11
const uint8_t dhtPin = 4; // Pin de datos DHT11
DHT dht(dhtPin, DHT11);
float temperature = 0.0;
float humidity = 0.0;
// DS18B20
const uint8_t oneWireBus = 2; // Pin de datos DS18B20
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);
float temperature2 = 0.0;
// Sensor de Flama YG1006 (Salida digital)
const uint8_t flamePin = 23; // Pin GPIO para el sensor de flama
String flameStatus = "OFF";
// Sensor de Movimiento HC-SR501
const uint8_t motionPin = 32; // Pin GPIO para el sensor de movimiento
String motionStatus = "NO DETECTADO";
// Sensor de Distancia HC-SR04
const uint8_t trigPin = 12; // Pin TRIG
const uint8_t echoPin = 14; // Pin ECHO
float distance = 0.0;
// Definir el pin donde se conecta el sensor de flujo YF-S201
const int flowPin = 5; // Pin GPIO 6 del ESP32, puedes cambiarlo según tu configuración
volatile int flowCount = 0; // Contador de pulsos
float flowRate = 0.0;
unsigned long lastTime = 0;
float totalFlow = 0.0;
// Interrupción para contar los pulsos del sensor
void IRAM_ATTR pulseCounter(){flowCount++;}

// Servidores
AsyncWebServer server(80);
WebSocketsServer webSocket(81);
const unsigned long sendInterval = 2000;

// HTML embebido
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Indicadores de sensores</title>
    <link rel="shortcut icon" href="https://cdn-icons-png.flaticon.com/512/8899/8899687.png" type="image/x-icon">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.1/css/all.min.css">
    <style>
        :root {
    --background-color: #F4F6FA;
    --primary-color: #4A90E2;
    --text-color: #333333;
    --card-bg: #FFFFFF;
    --shadow-color: rgba(0, 0, 0, 0.1);
    }
    *{
        box-sizing: border-box;
        margin: 0;
        padding: 0;
    }
    body {
        font-family: 'Arial', sans-serif;
        background-color: var(--background-color);
        color: var(--text-color);
        margin: 0;
        padding: 0;
        display: flex;
        flex-direction: column;
        align-items: center;
        justify-content: center;
        min-height: 100vh;
        width: 100vw;
    }
    header {
        margin-bottom: 20px;
    }
    #title {
        font-size: 2em;
        color: var(--primary-color);
        text-align: center;
    }
    #logo{
        display: inline;
        position: absolute;
        top:10px;
        left:-15px;
        width: 130px;
    }
    #logo2{
        display: inline;
        position: absolute;
        top:0px;
        right:0px;
        width: 230px;
        padding: 10px;
        border-radius: 25px;
    }
    main{
        display: flex;
        flex-direction: column;
        justify-content: space-evenly;
        height: 80vh;
    }
    .gauge-container {
        display: flex;
        flex-wrap: wrap;
        gap: 20px;
        justify-content: space-evenly;
        align-items: center;
        width: 70vw;
        padding-bottom:20px ;
    }
    .card {
        background: var(--card-bg);
        padding: 20px;
        box-shadow: 0px 4px 8px var(--shadow-color);
        position: relative;
        display: flex;
        flex-direction: column;
        align-items: center;
        text-align: center;
        overflow: hidden;
        border-radius: 20px;
        width: 190px;
        height: 254px;
    }
    .card::before {
        content: '';
        position: absolute;
        width: 100px;
        background-image: linear-gradient(180deg, rgba(104, 111, 114, 0.1), rgba(104, 111, 114, 0.1));
        height: 130%;
        animation: rotBGimg 3s linear infinite;
        transition: all 0.2s linear;
    }
    @keyframes rotBGimg {
        from {
        transform: rotate(0deg);
        }
        to {
        transform: rotate(360deg);
        }
    }
    .card::after {
        content: '';
        position: absolute;
        background: var(--card-bg);
        inset: 5px;
        border-radius: 15px;
    }  
    .gauge {
        width: 200px;
        height: 160px;
    }
    .sensor-label {
        margin-bottom: 10px;
        font-size: 1.2em;
        font-weight: bold;
    }
    .gauge, .sensor-label{
        z-index: 1;
    }
    /* Responsive */
    @media (max-width: 768px) {
        .gauge-container {
            flex-direction: column;
            gap: 15px;
        }
        #titles{
        margin-top: 13vh;
        }
    }
    #titles{
        display: flex;
        flex-direction: column;
        align-items: center;
    }
    .fire{
        position: absolute;
        left: calc(50% - 30px);
        top: calc(50% - 70px);
        font-size: 70px;
    }
    .people{
        position: absolute;
        width:0;
        left: calc(50% - 40px);
        top: calc(50% - 60px);
        font-size: 70px;
    }
    .hidden {
        display: none;
    }
    </style>
</head>
<body>
    <header>
        <div id="titles">
            <h1 id="title">Indicadores de sensores</h1>
            <h3 id="subtitle1"> Elaborado por: Frank Vargas </h3>
            <h3 id="subtitle1"> Revisado  por: Ing. Edgar Quezada </h3>
        </div>
        <img id="logo" src="https://redi.cedia.edu.ec/members/UCSG.png" alt="UCSG_iconlogo">
        <img id="logo2" src="https://www.ucsg.edu.ec/wp-content/uploads/2018/11/fac-tec.jpg" alt="FEDT_logo">
    </header>
    <main>
        <!-- primer contenedor -->
        <div class="gauge-container">
            <div class="card">
                <div id="HumidityGauge" class="gauge"></div>
                <label class="sensor-label">Humedad relativa DHT11</label>
            </div>
            <div class="card">
                <div id="TemperatureGauge" class="gauge"></div>
                <label class="sensor-label">Tempatura ambiente DHT11</label>
            </div>
            <div class="card">
                <div id="TemperatureGauge2" class="gauge"></div>
                <label class="sensor-label">Temperatura sonda</label>
            </div>
            <div class="card">
                <div id="FlowGauge" class="gauge"></div>
                <label class="sensor-label">Flujo</label>
            </div>
        </div>
        <!-- segundo contenedor -->
        <div class="gauge-container">
            <div class="card">
                <div id="FlameGauge" class="gauge">
                    <i class="fa-solid fa-fire fireStatic fire"></i>
                    <i class="fa-solid fa-fire fireAlert fire fa-beat-fade hidden" style="color: #d88f21;"></i>
                </div>
                <label class="sensor-label">Flama</label>
            </div>
            <div class="card">
                <div id="DistanceGauge" class="gauge"></div>
                <label class="sensor-label">Distancia</label>
            </div>
            <div class="card">
                <div id="MotionGauge" class="gauge">
                    <i class="fa-solid fa-users motionStatic people"></i>
                    <i class="fa-solid fa-users motionAlert people fa-fade hidden" style="color: #9d2121;"></i>
                </div>
                <label class="sensor-label">Movimiento</label>
            </div>
        </div>
    </main>
    <script src="https://cdn.jsdelivr.net/npm/raphael"></script>
    <script src="https://cdn.jsdelivr.net/npm/justgage@1.3.5/justgage.min.js"></script>
    <script>
        const ws = new WebSocket('ws://' + window.location.hostname + ':81/');
        ws.onmessage = function(event) {
            //Just getting data through JSON file gotten with websockets
            let data = JSON.parse(event.data);
            let fireDetected = data.flame;
            let motionDetected = data.motion;
            //Captura de elementos segun su clase
            const fireStatic = document.querySelector('.fireStatic');
            const fireAlert = document.querySelector('.fireAlert');
            const motionStatic = document.querySelector('.motionStatic');
            const motionAlert = document.querySelector('.motionAlert');
            //Actualizacion de medidores
            HumidityGauge.refresh(data.humidity);
            TemperatureGauge.refresh(data.temperature);
            TemperatureGauge2.refresh(data.temperature2);
            DistanceGauge.refresh(data.distance);
            FlowGauge.refresh(data.flowRate);
            console.log("Flama: " + fireDetected);
            console.log("Movimiento: " + motionDetected);
            console.log("Flujo:" + data.flowRate);
            console.log("Flujo:" + data.totalFlow);
            // Control de Flama
            if (fireDetected === "ON") {
                fireStatic.classList.remove('hidden');
                fireAlert.classList.add('hidden');
            } else {
                fireStatic.classList.add('hidden');
                fireAlert.classList.remove('hidden');
            }
            //Control de presencia
            if (motionDetected === "DETECTADO") {
                motionStatic.classList.add('hidden');
                motionAlert.classList.remove('hidden');
            } else {
                motionStatic.classList.remove('hidden');
                motionAlert.classList.add('hidden');
            }
        };
        let HumidityGauge = new JustGage({
            id: "HumidityGauge",
            gaugeWidthScale: .4,
            decimals:2,
            value: 0,
            min: 0,
            max: 100,
            label: "%",
            labelFontColor: "#000000",
            levelColors: ["#00ff00", "#ffdd00", "#ff0000"],
            labelMinFontSize:20,
            valueMinFontSize:20,
        });
        let TemperatureGauge = new JustGage({
            id: "TemperatureGauge",
            gaugeWidthScale: .4,
            decimals:2,
            value: 0,
            min: 0,
            max: 80,
            label: "ºC",
            labelFontColor: "#000000",
            levelColors: ["#00ff00", "#ffdd00", "#ff0000"],
            labelMinFontSize:20,
            valueMinFontSize:20,
        });
        let TemperatureGauge2 = new JustGage({
            id: "TemperatureGauge2",
            gaugeWidthScale: .4,
            decimals:2,
            value: 0,
            min: -10,
            max: 80,
            label: "ºC",
            labelFontColor: "#000000",
            levelColors: ["#00ff00", "#ffdd00", "#ff0000"],
            labelMinFontSize:20,
            valueMinFontSize:20,
        });
        let FlowGauge = new JustGage({
            id: "FlowGauge",
            gaugeWidthScale: .4,
            decimals:2,
            value: 0,
            min: 0,
            max: 40,
            label: "L/min",
            labelFontColor: "#000000",
            levelColors: ["#00ff00", "#ffdd00", "#ff0000"],
            labelMinFontSize:20,
            valueMinFontSize:20,
        });
        let DistanceGauge = new JustGage({
            id: "DistanceGauge",
            gaugeWidthScale: .4,
            decimals:2,
            value: 0,
            min: 0,
            max: 400,
            label: "cm",
            labelFontColor: "#000000",
            levelColors: ["#00ff00", "#ffdd00", "#ff0000"],
            labelMinFontSize:20,
            valueMinFontSize:20,
        });
    </script>
</body>
</html>
)rawliteral";

void sendData();
void handleWebSocketMessage(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
void notFound(AsyncWebServerRequest *request);
float readDistance();
float leerFlujo();

void setup() {
    Serial.begin(115200);
    pinMode(flamePin, INPUT);// Configurar el pin del sensor de flama como entrada
    pinMode(motionPin, INPUT); // Configurar sensor de movimiento como entrada
    pinMode(trigPin, OUTPUT);// Configurar sensor de distancia ultrasonico
    pinMode(echoPin, INPUT);
    pinMode(flowPin, INPUT_PULLUP);// Configurar el pin del sensor de flujo
    attachInterrupt(digitalPinToInterrupt(flowPin), pulseCounter, FALLING);// Configurar interrupción para contar los pulsos
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    Serial.println("Conectado a la red, direccion IP:");
    Serial.println(WiFi.localIP());

    // Configurar mDNS con un nombre personalizado
    if (!MDNS.begin("dashboard-ucsg")) { // Cambia "mi-sensor" al nombre que prefieras
        Serial.println("Error iniciando mDNS");
    } else {
        Serial.println("mDNS iniciado. Puedes acceder en: http://mi-sensor.local");
    }
    dht.begin();
    // Servidor Web
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", htmlPage);});
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "¡Página no encontrada!");});
    server.begin();
    // WebSocket
    webSocket.begin();
    webSocket.onEvent([](uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
        switch (type) {
            case WStype_DISCONNECTED:
                Serial.printf("[%u] Desconectado!\n", num);
                break;
            case WStype_CONNECTED: {
                IPAddress ip = webSocket.remoteIP(num);
                Serial.printf("[%u] Conectado desde %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
                break;}}
    });
}
void loop() {
    webSocket.loop();
    static uint32_t prevMillis = 0;
    if (millis() - prevMillis >= sendInterval) {
        prevMillis = millis();
        //lectura DHT11
        temperature = dht.readTemperature();
        humidity = dht.readHumidity();
        // Lectura DS18B20
        sensors.requestTemperatures();
        temperature2 = sensors.getTempCByIndex(0);
        // Leer el estado del sensor de flama
        flameStatus = digitalRead(flamePin) == LOW ? "ON" : "OFF";
        // Leer el estado del sensor de movimiento
        motionStatus = digitalRead(motionPin) == HIGH ? "DETECTADO" : "NO DETECTADO";
        // Leer distancia HC-SR04
        distance = readDistance();
        leerFlujo();
        // Enviar JSON
        sendData();
    }
}
// Función para leer distancia del HC-SR04
float readDistance() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    float duration = pulseIn(echoPin, HIGH);
    float distanceCm = duration * 0.0343 / 2; // Conversión a cm
    if (distanceCm >= 400 || distanceCm <= 2) {
        return -1; // Valor fuera de rango
    }
    return distanceCm;
}
// Función para leer el flujo
float leerFlujo() {
  // Calcular el flujo cada segundo
  unsigned long currentTime = millis();
  if (currentTime - lastTime >= 1000) {
    noInterrupts();// Deshabilitar interrupciones para leer el contador de pulsos
    int pulses = flowCount;
    flowCount = 0;
    interrupts();
    // Calcular la tasa de flujo (en litros por minuto)
    flowRate = pulses / 7.5; // 7.5 pulsos por litro según el sensor YF-S201
    totalFlow += flowRate / 60.0; // Acumulamos el total de litros
    lastTime = currentTime;}
  return flowRate;
}
// Enviar datos al cliente WebSocket
void sendData() {
    String data = "{\"temperature\":" + String(temperature) + 
                  ",\"humidity\":" + String(humidity) + 
                  ",\"temperature2\":" + String(temperature2) + 
                  ",\"flame\":\"" + flameStatus + 
                  "\",\"motion\":\"" + motionStatus + 
                  "\",\"distance\":" + String(distance) + 
                  ",\"flowRate\":" + String(flowRate) + 
                  ",\"totalFlow\":" + String(totalFlow) + "}";
    webSocket.broadcastTXT(data);
}