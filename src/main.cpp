#include <Arduino.h>

//Servo Libraries
#include <ESP32Servo.h>
#include <analogWrite.h>
#include <ESP32Tone.h>
#include <ESP32PWM.h>

//PS4 Controller Libraries
#include <ps4.h>
#include <PS4Controller.h>
#include <ps4_int.h>

//Robot states of operation
enum robotStates {
  LOCKED, AUTO, MANUAL, STTOPED
};
robotStates robotState = LOCKED;
bool right = true;
bool desempate = false;

bool optionPressed = false;

//IR Remote library and variables
#include <IRremote.h>
#define irReceiverPin 23
IRrecv irrecv(irReceiverPin);
decode_results results;

//Edge Sensors Pins and Variables
#define leftSensorPin 34
#define rightSensorPin 35

int leftSensorRef = 0;
int rightSensorRef = 0;
int rightSensorTolerance = 500;
int leftSensorTolerance = 500;
bool rightReading = true;
int rightSensor = 35;
int leftSensor = 34;

//Presence Sensor Pins and Variables

#define rightInfSensor 18//17
#define middleInfSensor 5//18
#define leftInfSensor 4 //23

//Auto mode states of operation
enum autoStates {
  STOPPED, READY, RUNNING
};
autoStates autoState = STOPPED;

enum tatics {
  STAR, RADAR
};
tatics tatic = RADAR; //retirei esta linha no codigo novo

//PS4 LED status variables
unsigned long blinkTimer;
bool ledOn = true;
int ledIntensity;

//Define Motors Pins and Instances
#define leftMotorPin 26
#define rightMotorPin 27

Servo MotorEsquerdo;
Servo MotorDireito;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  irrecv.enableIRIn(); //Enable IR Receiver

  PS4.begin("8c:f1:12:a0:c4:84"); //Start Connection between ESP32 and PS4 Controller
  //PS4.begin("60:5b:b4:56:c5:fa");
  //PS4.begin("a8:47:4a:ed:40:64");
  while (!PS4.isConnected()) {
    Serial.println("WatingConnection");
    delay(250);
  }

  pinMode(leftInfSensor, INPUT);
  pinMode(middleInfSensor, INPUT);
  pinMode(rightInfSensor, INPUT);

  PS4.setLed(100, 0, 0);
  PS4.sendToController();

  MotorEsquerdo.attach(leftMotorPin);
  MotorDireito.attach(rightMotorPin);
  MotorEsquerdo.write(90); 
  MotorDireito.write(90);
  
  Serial.println("Ready and LOCKED");
}

void loop() {

  //sensorTest();
  // put your main code here, to run repeatedly:
  if (PS4.isConnected()) {
    Status_Verify();
  } else {
    MotorEsquerdo.write(90);
    MotorDireito.write(90);
  }
}

void sensorTest() {
  //Serial.println("TestintSensors");
  if (digitalRead(leftInfSensor)) {
    Serial.println("Left");
  }
  if (digitalRead(middleInfSensor)) {
    Serial.println("Middle");
  }
  if (digitalRead(rightInfSensor)) {
    Serial.println("Right");
  }

}
//Mudan~ca de status do robo a partir do controle de ps4 e mudan~cas de LED
void Status_Verify() {
  if (PS4.Options()) {
    if (!optionPressed) {
      optionPressed = true;
      if (robotState == LOCKED) {
        robotState = AUTO;
        PS4.setLed(0, 100, 0);
        PS4.sendToController();
        MotorEsquerdo.write(90);
        MotorDireito.write(90);
        Serial.println("AUTO");

      } else if (robotState == AUTO) {
        robotState = MANUAL;
        PS4.setLed(0, 0, 100);
        PS4.sendToController();
        MotorEsquerdo.write(90);
        MotorDireito.write(90);
        autoState = STOPPED;
        Serial.println("MANUAL");

      } else if (robotState == MANUAL) {
        robotState = LOCKED;
        autoState = STOPPED;
        PS4.setLed(100, 0, 0);
        PS4.sendToController();
        MotorEsquerdo.write(90);
        MotorDireito.write(90);
        Serial.println("LOCKED");
      }
    }
  } else {
    optionPressed = false;
  }

  if (robotState == MANUAL) {
    ManualControl();
  } else if (robotState == AUTO) {
    Auto();
  }
}

void MotorWrite(int ppmDireito, int ppmEsquerdo) {
  MotorDireito.write(ppmDireito);
  MotorEsquerdo.write(ppmEsquerdo);
}

void IRRead() {
  String value;
  if (irrecv.decode(&results)) {
    value = String(results.value, HEX);
    Serial.println(value);
    irrecv.resume();
  }
  
  if (value == "10") {
    if (autoState == STOPPED) {
      //Serial.println("ReadyToGo");
      autoState = READY;
      MotorWrite(90, 90);
      CalibrateSensors();
    }
  } else if (value == "810") {
    if (autoState == READY) {
      autoState = RUNNING;
      PS4.setLed(0, 100, 0);
      PS4.sendToController();
      //Serial.println("LET'S GO!!!");
    }
  } else if ( value == "410") {
    if (autoState == RUNNING || autoState == READY) {
      //Serial.println("STOP");
      autoState = STOPPED;
      MotorWrite(90, 90);
    }
  }
}

void Star() 
{
  //triangulo
}

void Radar() {
  //Serial.println("StarStart");

  //Logica para girar o robo no inicio da rodada de desempate
  if (!desempate) {
    unsigned int timerStart = millis() + 300;
    while (timerStart > millis()) {
      MotorWrite(120, 80);
    }
  }

 
  while (autoState == RUNNING) {
    //Sensor do meio nÃ£o esta lendo e o robo vai girar para o sentido escolhido no inicio
    while (!digitalRead(middleInfSensor) && autoState == RUNNING) {
      //Serial.println("NotFind");
      IRRead();
      //Status_Verify();
      if (right) {
        MotorWrite(110, 80);
      } else {
        MotorWrite(80, 110);
      }
    }

    //Sensor do meio esta lendo e o robo ira seguir reto
    right = !right;
    while (digitalRead(middleInfSensor) && autoState == RUNNING) {
      //Serial.println("Find");
      IRRead();
      //Status_Verify();
      //Valores da velocidade de cada motor - regular caso o robo nÃ£o ande reto
      MotorWrite(150, 150);
    }
  }
}

void Fradar() {
  //Serial.println("StarStart");
  if (!desempate) {
    unsigned int timerStart = millis() + 300;
    while (timerStart > millis()) {
      MotorWrite(120, 80);
    }
  }

  MotorWrite(100,100);
  delay(50);
 
  while (autoState == RUNNING) {
    while (!digitalRead(middleInfSensor) && autoState == RUNNING) {
      //Serial.println("NotFind");
      IRRead();
      //Status_Verify();
      if (right) {
        MotorWrite(80, 110);
      } else {
        MotorWrite(110, 80);
      }
    }
    right = !right;
    while (digitalRead(middleInfSensor) && autoState == RUNNING) {
      //Serial.println("Find");
      IRRead();
      //Status_Verify();
      MotorWrite(150, 150);
    }
  }
}//Fim void radar
  
}

void Auto() {
  IRRead();

  //Bolinha do controle inicia o modo radar - MODO PADRÃƒO
  if (PS4.Circle()) {
    Serial.println("RadarMode");
    tatic = RADAR;
  }

  //Quadrado do controle inicia o modo de leitura enquanto vai para frente
  if (PS4.Square()) {
    Serial.println("ForwardRadarMode");
    tatic = FRADAR;
  }
  
  //Triangulo do controle inicia o modo de leitura estrela - NÃƒO ESTA FUNCIONANDO
  if (PS4.Triangle()) {
    Serial.println("StarMode");
    tatic = STAR;
  }

  //Seta para direita do controle da a preferencia de girar para a direita em qualquer modo
  if (PS4.Right()) {
    Serial.println("Right");
    right = true;
  }
  //Seta para esquerda do controle da a preferencia de girar para a esquerda em qualquer modo
  if (PS4.Left()) {
    Serial.println("Left");
    right = false;
  }
  //Seta para cima do controle desativa o modo de desempate
  if (PS4.Up()) {
    Serial.println("Up");
    desempate = false;
  }
  //Seta para baixo do controle ativa o modo de desempate - O ROBO DEVE INICIAR A PARTIDA DE COSTAS
  if (PS4.Down()) {
    Serial.println("Down");
    desempate = true;
  }

  //Estado do controle - Preparado para a luta ou nÃ£o
  if (autoState == RUNNING) {
    if (tatic == STAR) {
      Star();
    } else if (tatic == RADAR) {
      Radar();
    }
  } else if (autoState == READY) {
    MotorWrite(90, 90);
    if (blinkTimer < millis()) {
      if (ledOn) {
        PS4.setLed(0, 0, 0);
        PS4.sendToController();
      } else {
        PS4.setLed(0, 100, 0);
        PS4.sendToController();
      }
      ledOn = !ledOn;
      blinkTimer = millis() + 200;
    }
  } else if (autoState == STOPPED) {
    MotorWrite(90, 90);
    if (blinkTimer < millis()) {
      if (ledOn) {
        PS4.setLed(0, ledIntensity++, 0);
        PS4.sendToController();
      } else {
        PS4.setLed(0, ledIntensity--, 0);
        PS4.sendToController();
      }
      if (ledIntensity == 0 || ledIntensity == 100) {
        ledOn = !ledOn;
      }
      blinkTimer = millis() + 10;
    }
  }
}
void CalibrateSensors() {
  leftSensorRef = analogRead(leftSensorPin) - leftSensorTolerance;
  rightSensorRef = analogRead(rightSensorPin) - rightSensorTolerance;
}