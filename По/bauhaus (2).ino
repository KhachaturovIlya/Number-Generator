//барабан
#define IN1_BAR 2
#define IN2_BAR 3
#define IN3_BAR 4
#define IN4_BAR 5

//система подъёма
#define DIR_LIFT 6
#define STEP_LIFT 7

//конвейер
#define IN3_CONV 11
#define IN4_CONV 10

#define LIM 8 //концевик
#define OPTIC 9 //оптопара

//состояния системы
#define WAITING_ST 0
#define PLUR_START_ST 1
#define SIN_START_ST 2
#define INF_START_ST 3

//команды с Raspberry Pi
#define INIT_COMMAND 0 //запуск системы
#define SIN_START_COMMAND 1 //однократное выполнение итерации
#define RETURN_COMMAND 2 //возврат системы в начальное положение
#define ROLLING_COMMAND 3 //запуск барабана
#define SCANNING_COMMAND 4 //ручной запуск СТЗ
#define LOADING_COMMAND 5 //загрузка кубиков в барабан
#define PLUR_START_COMMAND 6 //многократное выполнение итераций
#define INF_START_COMMAND 7 //бесконечное выполнение итераций
#define UP_COMMAND 8 //подъём конвейера
#define DOWN_COMMAND 9 //спуск конвейера
#define STOP_COMMAND 10 //прерыв повтора циклов
#define NO_COMMAND 255//команды не поступило
#define MAX_COMMAND NO_COMMAND //если поступила неизвестная команда, случай приравнивается к предыдущему(с отсутствием команды)

#define TOWER_SPEED 5000 //время работы башни
#define SCAN_SPEED 7000 //время работы СТЗ
#define INIT_TIMEOUT 5000 //время загрузки Raspberry Pi

#define LIFT_NUM 10 //количество шагов для полного выполнения цикла подъёма-спуска
#define LIFT_SPEED 10 //длительность шага

#define CONV_NUM 10 //количесиво шагов для
#define CONV_SPEED 10//время работы 

#define BAR_NUM 10 //число шагов для оборота барабана
#define BAR_SPEED 10 //время произведения шага

int state=0;//состояние системы

int getComd(){
  int comd=NO_COMMAND;
  if (Serial.available()){
    comd=Serial.read();//считывание команды из порта последовательного интерфейса
    if (comd>MAX_COMMAND){
      comd=NO_COMMAND;
    }
    else{
      return comd;
    }
  }
  else{
    comd=NO_COMMAND;
  }
}
int PLUR_START(int count){
  int i=0;
  while (true){
    int comd=getComd();//получение команды с Raspberry Pi
    if (comd==STOP_COMMAND){
      state=WAITING_ST;
      return;
    }
    else{
      if (i>=count){
        state=WAITING_ST;
        return;
      }
      else{
         cycle();
         i++;
      }
    }
  }
}
void baraban_stop(){
  //прекращение подачи питания на шаговый двигатель
  digitalWrite(IN1_BAR, LOW); 
  digitalWrite(IN2_BAR, LOW); 
  digitalWrite(IN3_BAR, LOW); 
  digitalWrite(IN4_BAR, LOW);
}
void baraban_step1(){
  digitalWrite(IN1_BAR, HIGH); 
  digitalWrite(IN2_BAR, LOW); 
  digitalWrite(IN3_BAR, LOW); 
  digitalWrite(IN4_BAR, HIGH);
  delay(BAR_SPEED/4);
}
void baraban_step2(){
  digitalWrite(IN1_BAR, HIGH); 
  digitalWrite(IN2_BAR, HIGH); 
  digitalWrite(IN3_BAR, LOW); 
  digitalWrite(IN4_BAR, LOW);
  delay(BAR_SPEED/4);
}
void baraban_step3(){
  digitalWrite(IN1_BAR, LOW); 
  digitalWrite(IN2_BAR, HIGH); 
  digitalWrite(IN3_BAR, HIGH); 
  digitalWrite(IN4_BAR, LOW);
  delay(BAR_SPEED/4);
}
void baraban_step4(){
  digitalWrite(IN1_BAR, LOW); 
  digitalWrite(IN2_BAR, LOW); 
  digitalWrite(IN3_BAR, HIGH); 
  digitalWrite(IN4_BAR, HIGH);
  delay(BAR_SPEED/4);
}

void baraban_throw(){
  //оборот в обратную сторону
  baraban_step4();
  baraban_step3();
  baraban_step2();
  baraban_step1();
}
void launch(){
  baraban();
  baraban_throw();
  baraban_moveload();
  delay(TOWER_SPEED);
  SCAN_launch();
  delay(SCAN_SPEED);
  while (digitalRead(LIM)==LOW){
    up(1);
  }
}
void SCAN_launch(){
  //подача команды на Raspberry о запуске СТЗ
  Serial.write("1");
  delay(INIT_TIMEOUT);
}
void baraban_moveload(){
  if (digitalRead(OPTIC)==LOW){
  baraban_step1();
  }
  if (digitalRead(OPTIC)==LOW){
  baraban_step2();
  }
  if (digitalRead(OPTIC)==LOW){
  baraban_step3();
  }
  if (digitalRead(OPTIC)==LOW){
  baraban_step4();
  }
}
void baraban_diceload(){
  //запуск конвейера
  for (int j=0; j<CONV_NUM;j++){
    digitalWrite(IN3_CONV,HIGH);
    delay(CONV_SPEED);
    digitalWrite(IN3_CONV,LOW);
  }
}
void up(int steps){
  digitalWrite(DIR_LIFT,HIGH);//смена направления движения подъёмника
  for (int j=0; j<steps;j++){
    digitalWrite(STEP_LIFT,HIGH);
    delay(LIFT_SPEED);
    digitalWrite(STEP_LIFT,LOW);
  }
}
void down(){
    digitalWrite(DIR_LIFT,LOW);//смена направления движения подъёмника
    for (int j=0; j<LIFT_NUM;j++){
      digitalWrite(STEP_LIFT,HIGH);
      delay(LIFT_SPEED);
      digitalWrite(STEP_LIFT,LOW);
    }
}
void INF_START(){
  int comd=getComd();
  while (true){
  if (comd==STOP_COMMAND){
    state=WAITING_ST;
    return;
  }
  else{
    cycle();
  }
  }
}
void RETURN(){
  while (LIM==LOW){
    up(1);
  }
  baraban_moveload();
  SCAN_move();
}
void SIN_START(){
  cycle();
  state=WAITING_ST;
}
void waiting(){
  int comd=getComd();//получение команды с Raspberry Pi
  switch(comd){
    case DOWN_COMMAND:
      down();
      break;
    case UP_COMMAND:
      while(digitalRead(LIM)==LOW){//подъём до касания концевика
        up(1);
      }
      break;
    case INF_START_COMMAND:
      state=INF_START_ST;
      break;
    case NO_COMMAND:
      break;
    case PLUR_START_COMMAND:
      state=PLUR_START_ST;
      break;
    case LOADING_COMMAND:
      baraban_moveload();
      baraban_diceload();
      break;
    case SCANNING_COMMAND:
      SCAN_launch();
      break;
    case ROLLING_COMMAND:
      baraban();
      baraban_throw();
      break;
    case RETURN_COMMAND:
      RETURN();
      break;
    case SIN_START_COMMAND:
      state=SIN_START_ST;
      break;                   
  }
}
void baraban(){
  baraban_step1();
  baraban_step2();
  baraban_step3();
  baraban_step4();
}
void SCAN_move(){
  for (int j=0; j<CONV_NUM;j++){
    digitalWrite(IN4_CONV,HIGH);
    delay(CONV_SPEED);
    digitalWrite(IN4_CONV,LOW);
  }
}
//автоматическое выполнение цикла
void cycle(){
  baraban_moveload();
  baraban_diceload();
  baraban();
  digitalWrite(DIR_LIFT,HIGH);//переключение подъёмника в режим спуска
  down();
  baraban_throw();
  up(LIFT_NUM/2);
  SCAN_move();
  SCAN_launch();
  delay(SCAN_SPEED);
  up(LIFT_NUM/2);
}
void setup(){
  pinMode(IN1_BAR,OUTPUT);
  pinMode(IN2_BAR, OUTPUT);
  pinMode(IN3_BAR, OUTPUT);
  pinMode(IN4_BAR, OUTPUT);

  pinMode(STEP_LIFT,OUTPUT);
  pinMode(DIR_LIFT,OUTPUT);

  pinMode(IN3_CONV,OUTPUT);
  pinMode(IN4_CONV,OUTPUT);

  pinMode(LIM,INPUT);
  pinMode(OPTIC,INPUT);

  Serial.begin(9600);
  while(true){
    int comd=getComd();
    if (comd=='INIT_COMMAND'){
      delay(INIT_TIMEOUT);//ждём, пока запустится Raspberry Pi
      launch();// запуск и инициализация Arduino
      break;
    }
  }
}

void loop(){
  int comd=getComd(); //получение команды с Raspberry Pi
  switch(state){
    case WAITING_ST:
      waiting();
      break;
    case PLUR_START_ST:
      int ITER_NUM=getComd();
      PLUR_START(ITER_NUM);
      break;
    case INF_START_ST:
      INF_START();
      break;
    case SIN_START_ST:
      SIN_START();
      break;
  }
}
