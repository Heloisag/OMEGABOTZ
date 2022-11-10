void IRRead() {
  String value;
  if (irrecv.decode(&results))
  {
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

void Star() {

  /*rightSensor = analogRead(rightSensorPin);
  leftSensor = analogRead(leftSensorPin);

  if (digitalRead(middleInfSensor)) {
    MotorWrite(130, 130);
  } else if (digitalRead(leftInfSensor)) {
    MotorWrite(80, 95);
    delay(100);
  } else if (digitalRead(rightInfSensor)) {
    MotorWrite(100, 80);
    delay(100);
  }*/

  /*if (leftSensor < leftSensorRef && rightSensor < rightSensorRef) {

    MotorWrite(80, 80);
    delay(200);
    MotorWrite(100, 80);
    delay(200);
  } else if (leftSensor < leftSensorRef) {
    MotorWrite(100, 80);
  } else  if (rightSensor < rightSensorRef) {
    MotorWrite(80, 95);
  } else {
    MotorWrite(110, 110);
  }*/
}

void Radar() {
  //Serial.println("StarStart");
  if (!desempate) {
    unsigned int timerStart = millis() + 300;
    while (timerStart > millis()) {
      MotorWrite(120, 80);
    }
  }

 
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
  if (PS4.Circle()) {
    Serial.println("RadarMode");
    tatic = RADAR;
  }

  if (PS4.Square()) {
    Serial.println("ForwardRadarMode");
    tatic = FRADAR;
  }
  
  if (PS4.Triangle()) {
    Serial.println("StarMode");
    tatic = STAR;
  }
  if (PS4.Right()) {
    Serial.println("Right");
    right = true;
  }
  if (PS4.Left()) {
    Serial.println("Left");
    right = false;
  }
  if (PS4.Up()) {
    Serial.println("Up");
    desempate = false;
  }
  if (PS4.Down()) {
    Serial.println("Down");
    desempate = true;
  }

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
