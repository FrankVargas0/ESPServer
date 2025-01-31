Documentación del Sistema de Monitoreo con ESP32 y Sensores

1. Introducción
Este sistema permite la monitorización de diferentes variables ambientales mediante sensores conectados a un ESP32. Los datos capturados se muestran en una página web alojada en el propio ESP32, utilizando WebSockets para una actualización en tiempo real.

2. Componentes Utilizados

ESP32: Microcontrolador con WiFi integrado.
DHT11: Sensor de temperatura y humedad.
DS18B20: Sensor de temperatura digital.
YG1006: Sensor de flama.
HC-SR501: Sensor de movimiento PIR.
HC-SR04: Sensor ultrasónico de distancia.
YF-S201: Sensor de flujo de agua.

3. Configuración de Hardware
Cada sensor está conectado a un pin GPIO específico del ESP32:

DHT11 => 4
DS18B20 => 2
YG1006 (Flama) => 23
HC-SR501 => 32
HC-SR04 (Trig) => 12
HC-SR04 (Echo) => 14
YF-S201 => 5

4. Lógica en el ESP32
4.1 Conexión WiFi

El ESP32 se conecta a una red WiFi utilizando las credenciales predefinidas:

WiFi.begin(ssid, password);
while (WiFi.status() != WL_CONNECTED) {
    delay(500);
}

Una vez conectado, se imprime la dirección IP asignada.

4.2 Inicialización de Sensores
Cada sensor es inicializado en el setup() antes de comenzar la captura de datos.

4.3 Adquisición de Datos
Los datos de los sensores se capturan en intervalos definidos y se envían a los clientes WebSockets.
Ejemplo de lectura de temperatura:

temperature = dht.readTemperature();
humidity = dht.readHumidity();

Ejemplo de lectura de distancia con el sensor ultrasónico:

float readDistance() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    return pulseIn(echoPin, HIGH) * 0.034 / 2;
}

5. Página Web y Visualización de Datos
5.1 WebSockets
El ESP32 envía datos en tiempo real mediante WebSockets:

webSocket.sendTXT(num, jsonData);

5.2 HTML y CSS
El código HTML y CSS embebido en el ESP32 genera la interfaz gráfica con indicadores de los sensores.

Para añadir espacios para los nuevos sensores en el código HTML de la página web embebida en el ESP32, sigue estos pasos:
-Agrega nuevos elementos HTML: Debes añadir etiquetas <div> o cualquier otro elemento que desees usar para mostrar los valores de los sensores.
-Asigna un id único a cada nuevo sensor para que pueda ser actualizado mediante JavaScript.

Ejemplo de HTML modificado:

<div id="sensorContainer">
    <h2>Monitoreo de Sensores</h2>
    <div>
        <p>Temperatura: <span id="temperature">--</span> °C</p>
        <p>Humedad: <span id="humidity">--</span> %</p>
        <p>Flama: <span id="flame">--</span></p>
        <p>Movimiento: <span id="motion">--</span></p>
        <p>Distancia: <span id="distance">--</span> cm</p>
        <p>Flujo de Agua: <span id="flow">--</span> L/min</p>
        <!-- NUEVOS SENSORES -->
        <p>Nuevo Sensor 1: <span id="newSensor1">--</span></p>
        <p>Nuevo Sensor 2: <span id="newSensor2">--</span></p>
    </div>
</div>

5.3 JavaScript
Se utilizan JustGage y WebSockets para actualizar los valores en la interfaz.
Modifica el JavaScript: Asegúrate de actualizar los valores de los nuevos sensores cuando recibas datos a través de WebSockets.
Ejemplo de JavaScript:

document.addEventListener("DOMContentLoaded", function() {
    var socket = new WebSocket("ws://your_esp32_ip:81/");
    
    socket.onmessage = function(event) {
        var data = JSON.parse(event.data);
        document.getElementById("temperature").innerText = data.temperature;
        document.getElementById("humidity").innerText = data.humidity;
        document.getElementById("flame").innerText = data.flame;
        document.getElementById("motion").innerText = data.motion;
        document.getElementById("distance").innerText = data.distance;
        document.getElementById("flow").innerText = data.flow;
        // Actualización de nuevos sensores
        document.getElementById("newSensor1").innerText = data.newSensor1;
        document.getElementById("newSensor2").innerText = data.newSensor2;
    };
});


6. Agregar Nuevos Sensores
6.1 En el ESP32
Definir el nuevo sensor y su pin en el código.
Inicializar el sensor en setup().
Implementar la función de lectura de datos.
Incluir la variable del sensor en el JSON enviado a WebSockets.

Ejemplo:

DynamicJsonDocument doc(256);
doc["temperature"] = temperature;
doc["humidity"] = humidity;
doc["flame"] = flameDetected;
doc["motion"] = motionDetected;
doc["distance"] = distance;
doc["flow"] = waterFlow;

// Nuevos sensores
doc["newSensor1"] = newSensor1Value;
doc["newSensor2"] = newSensor2Value;

String jsonData;
serializeJson(doc, jsonData);
webSocket.sendTXT(clientNum, jsonData);


