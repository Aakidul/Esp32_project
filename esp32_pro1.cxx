#include <Servo.h>

Servo myservo;  // Create servo object

// Define pins for the ultrasonic sensor and buzzer:
const int trigPin = 7;   // Trigger pin of HC-SR04
const int echoPin = 6;   // Echo pin of HC-SR04
const int buzzerPin = 8; // Buzzer control pin
const int servoPin = 9;  // Servo signal pin

// Parameters for servo sweep
int pos = 0;
const int sweepDelay = 15;  // Delay in milliseconds for smooth movement

// Function to measure distance using HC-SR04
float getDistance() {
  // Clear trigger pin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // Send a 10µs HIGH pulse to trigger the sensor
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Read the echo time (in microseconds)
  long duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance in centimeters:
  // Speed of sound = 343 m/s, so distance = (duration / 2) * 0.0343 cm/µs
  float distance = (duration * 0.0343) / 2;
  return distance;
}

void setup() {
  Serial.begin(9600);
  
  // Attach the servo to the servoPin
  myservo.attach(servoPin);
  
  // Set pin modes
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  // Sweep servo from 0° to 180°
  for (pos = 0; pos <= 180; pos++) {
    myservo.write(pos);
    delay(sweepDelay);
    float dist = getDistance();
    
    // Debug info on Serial Monitor
    Serial.print("Angle: ");
    Serial.print(pos);
    Serial.print("   Distance: ");
    Serial.println(dist);
    
    // If object is closer than 100 cm, turn on buzzer
    if (dist > 0 && dist < 100) {
      digitalWrite(buzzerPin, HIGH);
    } else {
      digitalWrite(buzzerPin, LOW);
    }
  }
  
  // Sweep servo from 180° back to 0°
  for (pos = 180; pos >= 0; pos--) {
    myservo.write(pos);
    delay(sweepDelay);
    float dist = getDistance();
    
    Serial.print("Angle: ");
    Serial.print(pos);
    Serial.print("   Distance: ");
    Serial.println(dist);
    
    if (dist > 0 && dist < 100) {
      digitalWrite(buzzerPin, HIGH);
    } else {
      digitalWrite(buzzerPin, LOW);
    }
  }
}