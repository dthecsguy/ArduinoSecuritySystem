#include <ShiftedLCD.h>
#include <Keypad.h> 
#include <SPI.h>

const byte ROWS = 4, COLS = 4;
//define the symbols on the buttons of the keypads
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[COLS] = {9, 8, 7, 6}; //connect to the row pinouts of the keypad
byte colPins[ROWS] = {5, 4, 3, 2}; //connect to the column pinouts of the keypad

Keypad kypd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS);
LiquidCrystal lcd(A1); //sspin 

unsigned int sysperiod;
unsigned int prevtime;
char intrusion, buff[6], H, L, _size, solenoidPin = 10, p[7], temp[7], buzzerPin = A2;
unsigned char keyq, tock, tick;

//for tasks
typedef struct task {
  int state;
  unsigned long period;
  unsigned long elapsedTime;
  int (*TickFct)(int);
} task;

//state declarations
enum RDSWTCH_States{RDS_INIT, RDS_CLOSED, RDS_OPEN};
enum KYPD_States{KYPD_READ};
enum LOGIC_States{LOG_FRESH, LOG_UNL, LOG_LOC, LOG_RES, LOG_VER, LOG_SHOW, LOG_ALARM};
enum RESET_States{RES_WAIT, RES_RES};

//tick functions
int reedSwitchTick(int state){
  byte input = digitalRead(A0);  
  switch(state){  //make transitions
    case RDS_INIT:    if(input) state = RDS_CLOSED;
                      else state = RDS_OPEN;
                      break;
    
    case RDS_CLOSED:  if(input) state = RDS_CLOSED;
                      else state = RDS_OPEN;
                      break;
                      
    case RDS_OPEN:    if(input) state = RDS_CLOSED;
                      else state = RDS_OPEN;
                      break;
                      
    default:          state = RDS_INIT;
  }
  
  switch(state){ //do actions
    case RDS_CLOSED:  intrusion = 0; break;
    case RDS_OPEN:    intrusion = 1; break;
    default: break;
  }
  
  return state;
}

int keypadTick(int state){
  char input = kypd.getKey();

  switch(state){
    case KYPD_READ: state = KYPD_READ; break;
  }

  switch(state){
    case KYPD_READ: if(input == 0) {
                      keyq = 0;
                      break;
                    }
                    else{
                      pushBack(input);
                      keyq = 1;
                      break;
                    }
  }
  return state;
}

int logicTick(int state){
  char input[7];

  Serial.print("Password : ");
  Serial.println(p);
  

  copyS(input, buff);
  input[_size] = '\0';
  
  switch(state){
      case LOG_FRESH:   if (equalS(input, "#C")) {
                          state = LOG_RES;
                          tick = 0;
                        }
                        else state = LOG_FRESH;
                        break;
                        
      case LOG_UNL:     if (equalS(input, "#C")) {
                          tick = 0;
                          state = LOG_RES;
                          _size = 0;
                        }
                        else if(equalS(input, "*")) state = LOG_LOC;
                        else state = LOG_UNL;
                        break;
                        
      case LOG_LOC:     if (intrusion == 1) state = LOG_ALARM;
                        else if(equalS(input, p)) state = LOG_UNL;
                        else state = LOG_LOC;
                        break;
                        
      case LOG_RES:     if (input[_size - 1] == '#') {
                          state = LOG_VER;
                          copyS(temp, input);
                          temp[_size - 1] = '\0';
                          tick = 0; _size = 0;
                        }
                        else if (tick >= 40) state = LOG_UNL;
                        else state = LOG_RES;
                        break;

      case LOG_VER:     if (input[_size - 1] == '#') {
                          input[_size - 1] = '\0';
                          if (equalS(temp, input)){
                            state = LOG_SHOW;
                            copyS(p, temp);
                            tick = 0;
                          }
                        }
                        else if (tick >= 40) state = LOG_UNL;
                        else state = LOG_VER;
                        break;
                        
      case LOG_SHOW:    if (tick >= 10)
                          state = LOG_UNL;
                        else state = LOG_SHOW;
                        break;
                        
      case LOG_ALARM:   if (equalS(input, p)) state = LOG_UNL;
                        else state = LOG_ALARM;
                        break;  
  }

  switch(state){
      case LOG_FRESH:   lcd.clear();
                        lcd.setCursor(0, 0);
                        lcd.print("Fresh");
                        Serial.print("Fresh\n");
                        digitalWrite(solenoidPin, HIGH);
                        lcd.setCursor(0, 1);
                        lcd.print(input);
                        break;
                        
      case LOG_UNL:     Serial.print("Unlocked\n");
                        digitalWrite(solenoidPin, HIGH);
                        lcd.clear();
                        lcd.setCursor(0, 0);
                        lcd.print("Unlocked");
                        lcd.setCursor(0, 1);
                        lcd.print(input);
                        digitalWrite(buzzerPin, LOW);
                        break;
                        
      case LOG_LOC:     Serial.print("Locked\n");
                        digitalWrite(solenoidPin, LOW);
                        lcd.clear();
                        lcd.setCursor(0, 0);
                        lcd.print("Locked");
                        lcd.setCursor(0, 1);
                        lcd.print(input);
                        break;
                         
      case LOG_RES:     Serial.print("Enter Code\n");
                        lcd.clear();
                        lcd.setCursor(0, 0);
                        lcd.print("Enter Code");
                        lcd.setCursor(0, 1);
                        lcd.print(input);
                        tick++;
                        break;

      case LOG_VER:     Serial.print("Re-Enter Code\n");
                        lcd.clear();
                        lcd.setCursor(0, 0);
                        lcd.print("Re-Enter Code");
                        lcd.setCursor(0, 1);
                        lcd.print(input);
                        tick++;
                        break;
                        
      case LOG_SHOW:    Serial.print(p);
                        Serial.print("\n");
                        lcd.clear();
                        lcd.setCursor(0, 0);
                        lcd.print("New Password:");
                        lcd.setCursor(0, 1);
                        lcd.print(p);
                        tick++;
                        break;
                        
      case LOG_ALARM:   Serial.print("Intruder!!\n");
                        digitalWrite(solenoidPin, LOW);
                        lcd.clear();
                        lcd.setCursor(0, 0);
                        lcd.print("Intruder!!");
                        digitalWrite(buzzerPin, HIGH);
                        break;
  }

   Serial.print("Input: ");
   Serial.println(input);
  
  return state;
}

int resetTick(int state){  
  switch(state){  //make transitions
    case RES_WAIT:  if(tock > 15) state = RES_RES;
                      else state = RES_WAIT;
                      break;
                      
    case RES_RES:    state = RES_WAIT;
                      break;
                      
  }

  switch(state){  //make transitions
    case RES_WAIT:  if (!keyq) {
                        tock++;
                    }
                      else {
                        tock = 0;
                      }
                    break;
                      
    case RES_RES:    _size = 0;
                      tock = 0;
                      break;
  }

  return state;
}

const unsigned short tasksNum = 4;
task tasks[tasksNum];

void setup()
{
  Serial.begin(9600);

  pinMode(buzzerPin, OUTPUT);

  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  lcd.print("Starting..");
  delay(2000);
  
  pinMode(A0, INPUT);
  pinMode(solenoidPin, OUTPUT);
  
  sysperiod = 25;
  unsigned char i = 0;
  tock = 0;
  _size = 0;
    
  tasks[i].state = RDS_INIT;
  tasks[i].period = 100;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &reedSwitchTick;
  i++;
  tasks[i].state = KYPD_READ;
  tasks[i].period = 50;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &keypadTick;
  i++;
  tasks[i].state = LOG_FRESH;
  tasks[i].period = 300;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &logicTick;
  i++;
  tasks[i].state = RES_WAIT;
  tasks[i].period = 200;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &resetTick;
  
}

void loop() {	
  unsigned char i;
  if ((millis() - prevtime) >= sysperiod){
    for (i = 0; i < tasksNum; ++i) {
      if ( (millis() - tasks[i].elapsedTime) >= tasks[i].period) {
          tasks[i].state = tasks[i].TickFct(tasks[i].state);
          tasks[i].elapsedTime = millis(); // Last time this task was ran
      }
    }
  }
}

void pushBack(char c){
    if(_size >= 6){
      return;
    }
    else{
      buff[_size] = c;
      _size++;
    }

}

int equalS(const char* a, const char* b){
  for (char i = 0; i < 6; i++){
    if(a[i] != b[i]){
      return 0;
    }
    if (a[i] == '\0')
      break;
  }

  return 69;
}

void copyS(char* canv, const char* b){
  for(char i = 0; i < _size; i++){
    canv[i] = b[i];
  }
}
