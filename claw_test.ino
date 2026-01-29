#include <NewPing.h>      // include the NewPing library for this program
#include <Servo.h>
#define VCC_PIN 13
#define TRIGGER_PIN 12    // sonar trigger pin will be attached to Arduino pin 12
#define ECHO_PIN 11       // sonar echo pint will be attached to Arduino pin 11
#define GROUND_PIN 10
#define MAX_DISTANCE 200  // fmaximum distance set to 200 cm
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // initialize NewPing

#define SERVO 9
Servo myservo;

#define SERVO_OPEN 0
#define SERVO_CLOSE 100
#define SERVO_THRESHOLD 27
#define RESET_THRESHOLD 40
#define SERVO_SPEED 30
#define INTERVAL 1500
#define RETRY_TIMER 3000


#define SONAR_INTERVAL 50

int isOpen = 1;
int isReady = 1;
unsigned long grabTimer = 0;
unsigned long groundTimer = 0;
int isDetecting = 0;

void setup() //do the following things once

{
  Serial. begin(9600);            // set data transmission rate to communicate with computer
  pinMode(ECHO_PIN, INPUT) ;  
  pinMode(TRIGGER_PIN, OUTPUT) ;
  pinMode(GROUND_PIN, OUTPUT);    // tell pin 10 it is going to be an output
  pinMode(VCC_PIN, OUTPUT);       // tell pin 13 it is going to be an output
  digitalWrite(GROUND_PIN,LOW);   // tell pin 10 to output LOW (OV, or ground)
  digitalWrite(VCC_PIN, HIGH) ;   // tell pin 13 to output HIGH (+5V)

  myservo.attach(SERVO);
  myservo.write(SERVO_OPEN);

  Serial.println("System initialized");
}

void loop() // do the following things forever

{
  delay(SONAR_INTERVAL);

  int distance = readDistance();

  if(isOpen == 1 && isReady == 1) {
    if(distance > 0 && distance < SERVO_THRESHOLD) {

      // delay(INTERVAL);

      if(isDetecting == 0) {
        groundTimer = millis();
        isDetecting = 1;
        Serial.println("1.1 Ground detected");
      } 
      else if (millis()-groundTimer >= INTERVAL) {
        Serial.println("1.2 Closing slowly");
        slowClose();
        grabTimer = millis();
        isOpen = 0;
        isReady = 0;
        isDetecting = 0;
      }

    } else {
      if(isDetecting == 1) {
        isDetecting = 0;
        Serial.println("1.3 Claw moved away");
      }
    }
  }

  if(isOpen == 0 && isReady == 0) {
    if(distance > RESET_THRESHOLD) {
      isReady = 1;
      Serial.println("2. Ready to drop");
    }

    if(distance > 0 && distance < SERVO_THRESHOLD) {
      if(millis() - grabTimer >= RETRY_TIMER) {
        myservo.write(SERVO_OPEN);
        isOpen = 1;
        isReady = 1;
        Serial.println("1.4 Restart, try again");
      }
    }
  }

  if(isOpen == 0 && isReady == 1) {
    if(distance > 0 && distance < SERVO_THRESHOLD) {
      myservo.write(SERVO_OPEN);
      isOpen = 1;
      isReady = 0;
      Serial.println("3. Object dropped");
      delay(500);
    }
  }

  if(isOpen == 1 && isReady == 0) {
    if(distance > RESET_THRESHOLD) {
      isReady = 1;
      Serial.println("4. Reset complete. Ready to grab");
    }
  }
}


int readDistance() {
  int DISTANCE_IN_CM = sonar.ping_cm();   // read the sonar sensor, using a variable
  // Serial.print("Ping: ");                 //print â€œPing:" on the computer display
  // Serial.print(DISTANCE_IN_CM);           //print the value of the variable next
  // Serial.println("cm"); 

  return DISTANCE_IN_CM;

}


int slowClose () {
  if(SERVO_OPEN>SERVO_CLOSE) {
    for(int pos=SERVO_OPEN; pos >= SERVO_CLOSE; pos--) {
      myservo.write(pos);
      delay(SERVO_SPEED);
    }
  } else {
    for(int pos=SERVO_CLOSE; pos <= SERVO_OPEN; pos++) {
      myservo.write(pos);
      delay(SERVO_SPEED);
    }
  }
}
