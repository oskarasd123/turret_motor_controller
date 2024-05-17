

#define X_STEP_PIN 2
#define X_DIR_PIN 3
#define Y_STEP_PIN 11
#define Y_DIR_PIN 12

#define LASER_SWITCH_PIN 5

#define HIGH_TIME 20 //us

#define Y_MAX_STEP 2500
#define Y_STARTING_POS -3000
#define Y_MIN_STEP -3000
#define X_MAX_STEP 30000
#define X_MIN_STEP -30000

uint8_t laser_setting = 0;

double x_target = 0; // actual target
double x_speeding_target = 0; // moves with a constant speed
double x_smoothing_target = 0; // moves with a speed proportional to distance from speeding_target
double x_smooth_step = 0; // moves with a speed proportional to distance from smoothing_target
int64_t x_step = 0;

double y_target = Y_STARTING_POS;
double y_speeding_target = Y_STARTING_POS;
double y_smoothing_target = Y_STARTING_POS;
double y_smooth_step = Y_STARTING_POS;
int64_t y_step = Y_STARTING_POS;

double move_roughness = 0.01; // smaller value means smoother
double move_speed = 0.8; // must be less than 1

const uint8_t max_command_length = 32;
char recieved_command[max_command_length];
char temp_string[max_command_length];
char recieved_character;
uint8_t command_pointer = 0;
char end_marker = '\n';
char buf[20];

#define X_TARGET_COMMAND 'x'
#define Y_TARGET_COMMAND 'y'
#define X_ZERO_COMMAND 'X'
#define Y_ZERO_COMMAND 'Y'
#define QUERRY_COMMAND 'q'
#define MOVE_ROUGHNESS_COMMAND 'a'
#define MOVE_SPEED_COMMAND 'b'
#define LASER_SETTING_COMMAND 'l'


void doStepX(int direction){
  if (direction > 0){
    digitalWrite(X_DIR_PIN, HIGH);
    x_step++;
  }
  if (direction < 0){
    digitalWrite(X_DIR_PIN, LOW);
    x_step--;
  }
  digitalWrite(X_STEP_PIN, HIGH);
  delayMicroseconds(HIGH_TIME);
  digitalWrite(X_STEP_PIN, LOW);
}

void doStepY(int direction){
  if (direction > 0){
    digitalWrite(Y_DIR_PIN, HIGH);
    y_step++;
  }
  if (direction < 0){
    digitalWrite(Y_DIR_PIN, LOW);
    y_step--;
  }
  digitalWrite(Y_STEP_PIN, HIGH);
  delayMicroseconds(HIGH_TIME);
  digitalWrite(Y_STEP_PIN, LOW);
}

void parseCommand(){
  char * strtokIndx;
  char buf[20];

  // command identifier
  strcpy(temp_string, recieved_command);
  strtokIndx = strtok(temp_string," ");
  strcpy(buf, strtokIndx);

  switch (buf[0]){
    case (X_TARGET_COMMAND):{
      strtokIndx = strtok(NULL," ");
      strcpy(buf, strtokIndx);
      x_target = atol(buf);
      Serial.print("x ");
      Serial.println((long)x_target);
    }break;
    case (Y_TARGET_COMMAND):{
      strtokIndx = strtok(NULL," ");
      strcpy(buf, strtokIndx);
      y_target = atol(buf);
      Serial.print("y ");
      Serial.println((long)y_target);
    }break;
    case (X_ZERO_COMMAND):{
      strtokIndx = strtok(NULL," ");
      strcpy(buf, strtokIndx);
      if (buf[0] == 'c'){
        x_step = 0;
        x_target = 0;
        x_speeding_target = 0;
        x_smoothing_target = 0;
        x_smooth_step = 0;
        Serial.print("X");
      }
    }break;
    case (Y_ZERO_COMMAND):{
      strtokIndx = strtok(NULL," ");
      strcpy(buf, strtokIndx);
      if (buf[0] == 'c'){
        y_step = 0;
        y_target = 0;
        y_speeding_target = 0;
        y_smoothing_target = 0;
        y_smooth_step = 0;
        Serial.print("Y");
      }
    }break;
    case (QUERRY_COMMAND):{
      Serial.print("q ");
      Serial.print((long)x_step);
      Serial.print(" ");
      Serial.println((long)y_step);
    }break;
    case (MOVE_ROUGHNESS_COMMAND):{
      strtokIndx = strtok(NULL," ");
      strcpy(buf, strtokIndx);
      move_roughness = atof(buf);
      move_roughness = move_roughness > 1 ? 1 : move_roughness;
      move_roughness = move_roughness < 0.0001 ? 0.0001 : move_roughness;
      Serial.print("a ");
      Serial.println(move_roughness);
    }break;
    case (MOVE_SPEED_COMMAND):{
      strtokIndx = strtok(NULL," ");
      strcpy(buf, strtokIndx);
      move_speed = atof(buf);
      move_speed = move_speed < 0.01 ? 0.01 : move_speed;
      move_speed = move_speed > 1 ? 1 : move_speed;
      Serial.print("b ");
      Serial.println(move_speed);
    }break;
    case (LASER_SETTING_COMMAND):{
      strtokIndx = strtok(NULL," ");
      strcpy(buf, strtokIndx);
      laser_setting = atof(buf);
      Serial.print("l ");
      Serial.println(laser_setting);
    }break;
  }
  
}

void setup() {
  // put your setup code here, to run once:
  pinMode(X_STEP_PIN, OUTPUT);
  pinMode(X_DIR_PIN, OUTPUT);
  pinMode(Y_STEP_PIN, OUTPUT);
  pinMode(Y_DIR_PIN, OUTPUT);

  //https://arduinoinfo.mywikis.net/wiki/Arduino-PWM-Frequency
  //TCCR0B = TCCR0B & B11111000 | B00000010;    // set timer 0 divisor to     8 for PWM frequency of  7812.50 Hz on pins 5 and 6

  Serial.begin(19200);
  while (!Serial){}
  Serial.print("startup\n");
  delay(100);
}

void loop() {
  if (Serial.available() > 0){
    recieved_character = Serial.read();
    if (recieved_character != end_marker){
      recieved_command[command_pointer] = recieved_character;
      command_pointer++;
      if (command_pointer >= max_command_length){
        command_pointer = max_command_length -1;
      }
    }
    else {
      for (int i = command_pointer; i < max_command_length; i++){
        recieved_command[i] = 0;
      }
      parseCommand();
      command_pointer = 0;
    }
  }

  analogWrite(LASER_SWITCH_PIN, laser_setting);

  x_target = (x_target > X_MAX_STEP) ? X_MAX_STEP : x_target;
  x_target = (x_target < X_MIN_STEP) ? X_MIN_STEP : x_target;
  y_target = (y_target > Y_MAX_STEP) ? Y_MAX_STEP : y_target;
  y_target = (y_target < Y_MIN_STEP) ? Y_MIN_STEP : y_target;
  
  x_speeding_target += (x_target > x_speeding_target ? move_speed : -move_speed);

  x_smoothing_target = x_smoothing_target*(1-move_roughness) + x_speeding_target*move_roughness;

  x_smooth_step = x_smooth_step*(1-move_roughness) + x_smoothing_target*move_roughness;
  /*char buf[30];
  sprintf(buf, "loop %.06d %.06d",target_step, step);
  Serial.println(buf);*/
  if (x_step > x_smooth_step+0.5){
    doStepX(-1);
  }
  else if (x_step < x_smooth_step-0.5){
    doStepX(1);
  }

  y_speeding_target += (y_target > y_speeding_target ? move_speed : -move_speed);

  y_smoothing_target = y_smoothing_target*(1-move_roughness) + y_speeding_target*move_roughness;
  
  y_smooth_step = y_smooth_step*(1-move_roughness) + y_smoothing_target*move_roughness;

  if (y_step > y_smooth_step+0.5){
    doStepY(-1);
  }
  else if (y_step < y_smooth_step-0.5){
    doStepY(1);
  }
  delayMicroseconds(20);
}
