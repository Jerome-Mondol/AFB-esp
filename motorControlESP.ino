#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

bool isMovingForward = false;

// Motor control pins
const int IN1 = D1;
const int IN2 = D2;
const int IN3 = D3;
const int IN4 = D4;
const int ENA = D7;
const int ENB = D8;

const int trigPin = D6;
const int echoPin = D5;

ESP8266WebServer server(80);

const char* ssid = "AFB not working";
const char* password = "itisworking";

void forward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  // delay(50);
  Serial.println("Moving forward");
}

void backward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  // delay(50);
  Serial.println("Moving backward");
}

void left() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  // delay(50);
  Serial.println("Turning left");
}

void right() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  // delay(50);
  Serial.println("Turning right");
}

void stop() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  // delay(50);
  // Serial.println("Motors stopped");
}

double obstacleDistance() {
  double readings[5];
  int validCount = 0;

  for (int i = 0; i < 5; i++) {
    long duration;

    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH, 20000);  // Timeout = 20ms

    double dist = duration * 0.034 / 2;
    if (dist >= 2 && dist <= 400) { // valid range
      readings[validCount++] = dist;
    }

    delay(10); // Give sensor time to reset
  }

  if (validCount == 0) return -1.0;  // No valid readings

  // Simple median filter
  for (int i = 0; i < validCount - 1; i++) {
    for (int j = i + 1; j < validCount; j++) {
      if (readings[i] > readings[j]) {
        double temp = readings[i];
        readings[i] = readings[j];
        readings[j] = temp;
      }
    }
  }

  return readings[validCount / 2];  // Median
}

String htmlPage() {
  return R"rawliteral(
    <!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>CAR</title>
</head>
<body>
    <button onmousedown="sendCmd('forward')" onmouseup="sendCmd('stop')">Forward</button>
    <button onmousedown="sendCmd('backward')" onmouseup="sendCmd('stop')">Backward</button>
    <button onmousedown="sendCmd('left')" onmouseup="sendCmd('stop')">Left</button>
    <button onmousedown="sendCmd('right')" onmouseup="sendCmd('stop')">Right</button>
    <button onmousedown="sendCmd('stop')" onmouseup="sendCmd('stop')">Stop</button>




    <script>
  let lastCmdTime = 0;
  function sendCmd(dir) {
    const now = Date.now();
    if (now - lastCmdTime > 100) {  // 100ms debounce
      fetch('/' + dir);
      lastCmdTime = now;
    }
  }
</script>

</body>
</html>

  )rawliteral";
}


void handleRoot() { server.send(200, "text/html", htmlPage()); }


void setup() {
  Serial.begin(115200);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  analogWrite(ENA, 150);
  analogWrite(ENB, 150);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  WiFi.softAP(ssid, password);

  server.on("/", handleRoot);
  // Forward movement
  server.on("/forward", [] () {
    // Obstacle handling for forward movment.
    double dist = obstacleDistance();
    if(dist > 0 && dist < 10) {
      stop();
      isMovingForward = false;
      Serial.print("Obstacle detected :");
      Serial.println(dist);
    } else {
      forward();
      isMovingForward = true;
      server.send(200, "text/html", "Moving forward");
    }
  });
  // Backward Movement
  server.on("/backward", [] () {
    backward();
    server.send(200, "text/html", "Moving backward");
  });
  // Going left
  server.on("/left", [] () {
    left();
    server.send(200, "text/html", "Going left");
  });
  // Going right
  server.on("/right", [] () {
    right();
    server.send(200, "text/html", "Going right");
  });
  server.on("/stop", [] () {
    stop();
    isMovingForward = false;
    server.send(200, "text/html", "Motors stopped");
  });

  
  server.begin();
}
void loop() {
  server.handleClient();
  if(isMovingForward) {
    double dist = obstacleDistance();
      if(dist > 0 && dist < 10) {
        stop();
        isMovingForward = false;
        Serial.print("Obstacle detected :");
        Serial.println(dist);
      }
  }
}
