#include <NewPing.h>
#include <Servo.h>

#define VCC_PIN 13
#define TRIGGER_PIN 12    
#define ECHO_PIN 11       
#define GROUND_PIN 10
#define MAX_DISTANCE 200  
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); 

#define SERVO_PIN 9 // Renamed for clarity
Servo myservo;

#define SERVO_OPEN 0
#define SERVO_CLOSE 90
#define SERVO_THRESHOLD 40
#define RESET_THRESHOLD 65
#define SERVO_SPEED 30
#define INTERVAL 1500
#define RETRY_TIMER 3000

// *** NEW SETTING: HOW LONG TO IGNORE SENSOR AFTER GRAB ***
#define POST_GRAB_LOCKOUT 2000 

#define SONAR_INTERVAL 50

int isOpen = 1;
int isReady = 1;
unsigned long grabTimer = 0;
unsigned long groundTimer = 0;
unsigned long liftTimer = 0; // Renamed for clarity (was groundTimer in State 2)
int isDetecting = 0;

// *** NEW VARIABLE ***
unsigned long lockoutStartTime = 0; 

void setup() {
  Serial.begin(9600);
  pinMode(ECHO_PIN, INPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(GROUND_PIN, OUTPUT);
  pinMode(VCC_PIN, OUTPUT);
  digitalWrite(GROUND_PIN,LOW);
  digitalWrite(VCC_PIN, HIGH);

  myservo.attach(SERVO_PIN);
  myservo.write(SERVO_OPEN);

  Serial.println("System initialized");
}

void loop() {
  delay(SONAR_INTERVAL);

  // *** CRITICAL FIX: SENSOR LOCKOUT ***
  // If we recently grabbed, do NOT read the sensor or run logic
  if (millis() - lockoutStartTime < POST_GRAB_LOCKOUT) {
     Serial.println("Stabilizing power...");
     return; // Skip the rest of the loop!
  }

  int distance = readDistance();
  // Filter out 0 readings (standard sonar glitch)
  if (distance == 0) distance = 200; 
  
  Serial.println(distance);

  // --- STATE 1: READY TO GRAB ---
  if(isOpen == 1 && isReady == 1) {
    if(distance > 0 && distance < SERVO_THRESHOLD) {

      if(isDetecting == 0) {
        groundTimer = millis();
        isDetecting = 1;
        Serial.println("1.1 Ground detected");
      } 
      else if (millis() - groundTimer >= INTERVAL) {
        Serial.println("1.2 Closing slowly");
        slowClose();
        
        // UPDATE STATE
        grabTimer = millis();
        isOpen = 0;
        isReady = 0;
        isDetecting = 0;
        
        // *** START THE BLIND TIMER ***
        lockoutStartTime = millis(); 
      }

    } else {
      if(isDetecting == 1) {
        isDetecting = 0;
        Serial.println("1.3 Claw moved away");
      }
    }
  }

  // --- STATE 2: LIFTING / HOLDING ---
  // (I fixed the curly braces here so the logic is contained properly)
  if(isOpen == 0 && isReady == 0) {
    
    // A. CHECK FOR SUCCESSFUL LIFT (With debounce)
    if(distance > RESET_THRESHOLD) {
      if(isDetecting == 0) {
        liftTimer = millis();
        isDetecting = 1;
        Serial.println("2.1 High height detected...");
      } 
      else if (millis() - liftTimer >= INTERVAL) {
        isReady = 1;
        Serial.println("2.2 Lift Confirmed! Ready to drop");
        isDetecting = 0;
      }
    } else {
      // If we drop below threshold, reset the lift timer
      if(isDetecting == 1) {
        isDetecting = 0;
        Serial.println("2.3 False Alarm (Height unstable)");
      }
    }

    // B. CHECK FOR FUMBLE (Must be OUTSIDE the 'if > threshold' block)
    if(distance < SERVO_THRESHOLD) {
      if(millis() - grabTimer >= RETRY_TIMER) {
        myservo.write(SERVO_OPEN);
        isOpen = 1;
        isReady = 1;
        Serial.println("1.4 Fumble Detected. Restarting.");
        delay(1000); // Wait a second so we don't grab immediately
      }
    }
  }

  // --- STATE 3: DROP ---
  if(isOpen == 0 && isReady == 1) {
    if(distance < SERVO_THRESHOLD) {
      myservo.write(SERVO_OPEN);
      isOpen = 1;
      isReady = 0;
      Serial.println("3. Object dropped");
      delay(500);
      
      // Start lockout here too, just in case servo opening causes noise
      lockoutStartTime = millis();
    }
  }

  // --- STATE 4: RESET ---
  if(isOpen == 1 && isReady == 0) {
    if(distance > RESET_THRESHOLD) {
      isReady = 1;
      Serial.println("4. Reset complete. Ready to grab");
    }
  }
}

int readDistance() {
  return sonar.ping_cm();
}

int slowClose () {
  if(SERVO_OPEN > SERVO_CLOSE) {
    for(int pos = SERVO_OPEN; pos >= SERVO_CLOSE; pos--) {
      myservo.write(pos);
      delay(SERVO_SPEED);
    }
  } else {
    for(int pos = SERVO_OPEN; pos <= SERVO_CLOSE; pos++) {
      myservo.write(pos);
      delay(SERVO_SPEED);
    }
  }
  return 1;
}
