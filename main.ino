/*
   ============================================================
   ESP8266 Wi-Fi Soccer Bot
   ESP8266 + L298N Motor Driver
   Browser Based Controller
   ============================================================

   Wi-Fi:
   SSID     : SoccerBot
   Password : 12345678

   Open browser and go to:
   192.168.4.1
   ============================================================
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// ============================================================
// Wi-Fi
// ============================================================

const char* ssid = "SoccerBot";
const char* password = "12345678";

ESP8266WebServer server(80);


// ============================================================
// L298N MOTOR PINS
// ============================================================

// Left motor
#define ENA D1
#define IN1 D2
#define IN2 D3

// Right motor
#define ENB D7
#define IN3 D5
#define IN4 D6


// ============================================================
// MOTOR SPEED
// ESP8266 PWM range = 0 - 1023
// ============================================================

int motorSpeed = 700;


// ============================================================
// HTML CONTROL PAGE
// ============================================================

const char MAIN_page[] PROGMEM = R"rawliteral(

<!DOCTYPE html>

<html>

<head>

<meta name="viewport"
      content="width=device-width, initial-scale=1">

<title>Soccer Bot Controller</title>

<style>

body {
    font-family: Arial;
    text-align: center;
    background: #111;
    color: white;
    margin: 0;
    padding: 20px;
}

h1 {
    margin-bottom: 5px;
}

.status {
    margin: 10px;
    font-size: 18px;
    color: #00ff88;
}

.control {
    display: grid;
    grid-template-columns: 100px 100px 100px;
    gap: 12px;
    justify-content: center;
    margin-top: 30px;
}

button {

    width: 100px;
    height: 80px;

    font-size: 22px;
    font-weight: bold;

    border: none;
    border-radius: 15px;

    background: #333;
    color: white;

    box-shadow: 0 4px 8px #000;

    user-select: none;
    -webkit-user-select: none;
}

button:active {
    background: #00aaff;
    transform: scale(0.95);
}

.stop {
    background: #cc2222;
}

.kick {
    background: #ff8800;
}

.speedBox {
    margin-top: 35px;
}

input {
    width: 250px;
}

.speedValue {
    font-size: 20px;
    color: #00ff88;
}

</style>

</head>


<body>

<h1>⚽ Soccer Bot</h1>

<div class="status">
    Status: <span id="status">STOPPED</span>
</div>


<div class="control">

    <div></div>

    <button
        ontouchstart="move('forward')"
        ontouchend="stop()"
        onmousedown="move('forward')"
        onmouseup="stop()">
        ▲
    </button>

    <div></div>


    <button
        ontouchstart="move('left')"
        ontouchend="stop()"
        onmousedown="move('left')"
        onmouseup="stop()">
        ◀
    </button>


    <button
        class="stop"
        onclick="stop()">
        STOP
    </button>


    <button
        ontouchstart="move('right')"
        ontouchend="stop()"
        onmousedown="move('right')"
        onmouseup="stop()">
        ▶
    </button>


    <div></div>


    <button
        ontouchstart="move('backward')"
        ontouchend="stop()"
        onmousedown="move('backward')"
        onmouseup="stop()">
        ▼
    </button>

    <div></div>

</div>


<div class="speedBox">

    <h3>Motor Speed</h3>

    <input
        type="range"
        min="0"
        max="1023"
        value="700"
        id="speed"
        oninput="changeSpeed(this.value)">

    <div class="speedValue">
        <span id="speedValue">700</span>
    </div>

</div>


<script>

function move(direction) {

    fetch("/move?dir=" + direction);

    document.getElementById("status").innerHTML =
        direction.toUpperCase();
}


function stop() {

    fetch("/stop");

    document.getElementById("status").innerHTML =
        "STOPPED";
}


function changeSpeed(value) {

    document.getElementById("speedValue").innerHTML =
        value;

    fetch("/speed?value=" + value);
}

</script>

</body>

</html>

)rawliteral";


// ============================================================
// MOTOR FUNCTIONS
// ============================================================

void leftForward() {

    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

    analogWrite(ENA, motorSpeed);
}


void leftBackward() {

    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);

    analogWrite(ENA, motorSpeed);
}


void rightForward() {

    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);

    analogWrite(ENB, motorSpeed);
}


void rightBackward() {

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);

    analogWrite(ENB, motorSpeed);
}


// ============================================================
// ROBOT MOVEMENT
// ============================================================

void forward() {

    leftForward();
    rightForward();
}


void backward() {

    leftBackward();
    rightBackward();
}


void turnLeft() {

    leftBackward();
    rightForward();
}


void turnRight() {

    leftForward();
    rightBackward();
}


void stopRobot() {

    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);

    analogWrite(ENA, 0);
    analogWrite(ENB, 0);
}


// ============================================================
// MOVE REQUEST
// ============================================================

void handleMove() {

    if (!server.hasArg("dir")) {
        server.send(400, "text/plain", "Missing direction");
        return;
    }

    String direction = server.arg("dir");

    if (direction == "forward") {

        forward();

    }

    else if (direction == "backward") {

        backward();

    }

    else if (direction == "left") {

        turnLeft();

    }

    else if (direction == "right") {

        turnRight();

    }

    else {

        stopRobot();

    }

    server.send(200, "text/plain", "OK");
}


// ============================================================
// STOP REQUEST
// ============================================================

void handleStop() {

    stopRobot();

    server.send(200, "text/plain", "STOPPED");
}


// ============================================================
// SPEED REQUEST
// ============================================================

void handleSpeed() {

    if (server.hasArg("value")) {

        motorSpeed =
            server.arg("value").toInt();

        motorSpeed =
            constrain(motorSpeed, 0, 1023);
    }

    server.send(
        200,
        "text/plain",
        String(motorSpeed)
    );
}


// ============================================================
// SETUP
// ============================================================

void setup() {

    Serial.begin(115200);


    // Motor pins

    pinMode(ENA, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);

    pinMode(ENB, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);


    stopRobot();


    // --------------------------------------------------------
    // Create Wi-Fi Access Point
    // --------------------------------------------------------

    WiFi.mode(WIFI_AP);

    WiFi.softAP(
        ssid,
        password
    );


    Serial.println();
    Serial.println("==============================");
    Serial.println("Soccer Bot Started");
    Serial.println("==============================");

    Serial.print("Wi-Fi Name: ");
    Serial.println(ssid);

    Serial.print("IP Address: ");
    Serial.println(
        WiFi.softAPIP()
    );


    // --------------------------------------------------------
    // Web Server
    // --------------------------------------------------------

    server.on(
        "/",
        []() {
            server.send_P(
                200,
                "text/html",
                MAIN_page
            );
        }
    );


    server.on(
        "/move",
        handleMove
    );


    server.on(
        "/stop",
        handleStop
    );


    server.on(
        "/speed",
        handleSpeed
    );


    server.begin();

    Serial.println(
        "Web server started!"
    );
}


// ============================================================
// LOOP
// ============================================================

void loop() {

    server.handleClient();
}
