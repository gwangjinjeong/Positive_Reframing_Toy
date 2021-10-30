#include <Stepper.h>

#include <EEPROM.h>


// 2048:한바퀴(360도), 1024:반바퀴(180도)...
const int stepsPerRevolution = 2048; 
// 모터 드라이브에 연결된 핀 IN4, IN2, IN3, IN1
Stepper myStepper(stepsPerRevolution,11,9,10,8);           
void setup() {
  myStepper.setSpeed(14); 
  Serial.begin(9600);
  EEPROM.write(0,0);
}
void loop() {

  if (Serial.available())
  {
    char cTemp = Serial.read();

    switch(cTemp)
     {
       case '2':
         myStepper.step(280);
         break;
       case '8':
         myStepper.step(-280);
         break;
//         
//       case 'G':
//         digitalWrite(LED_G,HIGH);
//         break;
//       case 'g':
//         digitalWrite(LED_G,LOW);
//         break;
//         
//       case 'B':
//         digitalWrite(LED_B,HIGH);
//         break;
//       case 'b':
//         digitalWrite(LED_B,LOW);
//         break;
     }
  }
  // 시계 반대 방향으로 한바퀴 회전  
}

