const int PIR = 2;
const int RELE = 3;
const int HALL = A0;

const int MAIN_DELAY = 50;
const int ONE_SECOND = 1000 / MAIN_DELAY; //Константа, которая определяет количество циклов в одной секнде
const int TIME_TO_PRINT_DEBUG = 500 / MAIN_DELAY; //Константа определят задержку между дебажной информацией.

int sensorValue = 0;
int timeToOff = 5 * ONE_SECOND; //Единица измерения - пауза в основном цикле
bool doorOpen = false;
bool personInside = false;
bool lightOn = false;
int debugTime = TIME_TO_PRINT_DEBUG;

void setup() {
  pinMode( PIR, INPUT);
  pinMode( RELE, OUTPUT);
  
  Serial.begin(9600, SERIAL_8N1);
  while (!Serial) {}

  //Когда подали напряжение включаем свет на 10 секунд, дальше он погаснет, если не будет шевеления.
  digitalWrite(RELE, LOW);
  lightOn = true;

  // Установим прерывание на датчик 
  attachInterrupt(0, onMove, HIGH);
  
  //Определяем состояние двери
  sensorValue = analogRead(HALL);
  doorOpen = (sensorValue < 524) && (sensorValue > 500);
}


void onDoorOpen(){
  // Если дверь открывается, то свет нужно включить сразу.
  digitalWrite(RELE, LOW);
  doorOpen = true;
  lightOn = true;
  timeToOff = 5 * 60 * ONE_SECOND;
}

void onDoorClose(){
  // Человек входит в комнату. Датчик движения срабатывает до того, как дверь закроется,
  // на повторное срабатывание потребуется какое-то время, поэтому устанавливаем задержку,
  // чтобы свет не выключился, как только человек закроет дверь.
  timeToOff = 10 * ONE_SECOND;
  personInside = false;
  doorOpen = false;
}

void onMove(){
  if (!doorOpen){
    personInside = true;
    
    // если же там действительно человек, то за 20 минут он хоть раз да шевельнется, и счетчик начнет тикать заново.
    timeToOff = 20 * 60 * ONE_SECOND;
  
  } else {

    // Если дверь открыта, то либо это сделано потому, что там в шкафу роются, либо потому что забыли закрыть.
    // Если последнее, то через 5 минут свет погаснет, иначе человек там будет постоянно шевелиться, и будет сбрасываться счетчик.
    timeToOff = 5 * 60 * ONE_SECOND;
  }

  // Включаем свет, если он почему-то выключен, а кто-то внутри шевелится.
  if (!lightOn) {
    digitalWrite(RELE, LOW);
    lightOn = true;
  }
    
}

void loop() {

  // Проверяем, открыта ли дверь, есди состояние двери изменилось, то вызываем соответсвующие обработчики событий.
  sensorValue = analogRead(HALL);
  if ((sensorValue < 612) && (sensorValue > 412) && !doorOpen){
    onDoorOpen();
  } else if (((sensorValue > 612) || (sensorValue < 412)) && doorOpen){
    onDoorClose();
  }

  // Если свет включен, то отнимаем у счетчика единицу, ели при этом становится 0, то гасим свет.
  if(lightOn && (timeToOff-- == 0) ){
    digitalWrite(RELE, HIGH);
    lightOn = false;
  }

  // Печать отладочной информации в порт.
  if(debugTime-- == 0){
    //Печатаем отладочную информацию в консоль.
    if(doorOpen){
      Serial.print("Door opened (");
    } else{
      Serial.print("Door closed (");
    }
    Serial.print(sensorValue);
    Serial.print("), Movement ");
    if(digitalRead(PIR) == HIGH) {
      Serial.print("detected, ");
    } else {
      Serial.print("not detected, ");
    }
    Serial.print("Light ");
    if(lightOn) {
      Serial.print("on, cicles left ");
      Serial.print(timeToOff);
      Serial.println(".");
    } else {
      Serial.println("off.");
    }
    
    Serial.flush();
    debugTime = TIME_TO_PRINT_DEBUG;
  }
  
  delay(MAIN_DELAY); 
}

