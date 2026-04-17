#include <avr/io.h>
#include <Encoder.h>

#define Button1 PC7 
#define Button0 PC6
#define Dir_Motor1 PB0
#define Speed_Motor1 PB7
#define Dir_Motor2 PE6
#define Speed_Motor2 PD0 

#define ENCODER_PIN_A 0
#define ENCODER_PIN_B 1
#define COUNTS_PER_ROTATION 12*50

#define PWM_M1 OCR0A
#define PWM_M2 OCR0B

bool goForward = true;
bool Moving = true;
bool Prev_SW1_State = false; 
bool Prev_SW2_State = false; 

void motorControl();

Encoder encoder_A(ENCODER_PIN_A, ENCODER_PIN_B);

int clamp(int value, int minVal, int maxVal) {
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
}

void setup() {
  DDRB |= (1 << Dir_Motor1); // (Direction) Sets Register B Bitmask to 00000001 enabling Pin0 as a output
  DDRB |= (1 << Speed_Motor1); // (Speed) Sets Register B Bitmask to 10000000 enabling Pin7 as a ouput
  DDRE |= (1 << Dir_Motor2); // (Driection)
  DDRD |= (1 << Speed_Motor2);  // (Speed)

  // ENCODER SET UP 
  DDRB |= (1 << PB5); // Encoder detector for 1st motor
  DDRB |= (1 << PB6); // Encoder detector for 2nd motor  

  // Button Set up
  DDRC &= ~(1 << Button1); // Sets Register C Bitmask to 01111111 enabling Pin7 as a input
  DDRC &= ~(1 << Button0);
  
  // IR Sensor Array set up
  DDRB |= (1 << PB3); // sets PINB3 (LED_ON) as an output 
  PORTB |= (1 << PB3); // Turn on the IR LED's frrr

  //8 - bit timer set up 
  TCCR0A = (1 << COM0A1) | (1 << COM0B1) | (1 << WGM01) | (1 << WGM00); // 8-bit timer section on data sheet explains this 
  TCCR0B = (1 << CS00); // no prescaling on the clock p.g. 108

  // Motor PWM Set up
  PWM_M1 = 100; // number we are compairing too for the 50% duty cycle on the PWM signal (127/256 = 0.5) // motor a
  PWM_M2 = 200; // motor b (max is 256)
  
  //analogWrite(9, count/COUNTS_PER_ROTATION);

  Serial.begin(9600);  // Let me talk to the serial monitor fam
}

void loop() {

// Encoder Setup
  static unsigned long previous_time = micros();
  unsigned long current_time = micros();

  unsigned long dt_us = current_time - previous_time;
  double dt_s = double(dt_us) / 1000000; 

  static double previous_position = 0.0;

//ENCODER LOGIC 
  int32_t count = encoder_A.read();
  double current_position = count / COUNTS_PER_ROTATION;
  double dp = current_position - previous_position;

  double current_speed = dp / dt_s;

  //double ep = target_speed - current_speed; 

// PUSH BUTTON LOGIC 
  bool Current_SW1_State = (PINC & (1 << Button1));
  bool Current_SW2_State = (PINC & (1 << Button0));



  // on/off logic
  if (Current_SW1_State && !Prev_SW1_State) {
    Moving = !Moving;
    delay(200);
  }
  if (Moving == true) {
    motorControl();
  }
  else {
    PWM_M1 = 0;
    PWM_M2 = 0;
  }

  Prev_SW1_State = Current_SW1_State;

  // direction logic
  if (Current_SW2_State && !Prev_SW2_State) {
    goForward = !goForward;
    delay(500);
  }
  if (goForward == true) {
    PORTB |= (1 << Dir_Motor1);
    PORTE &= ~(1 << Dir_Motor2);
  }
  else {
    PORTB &= ~(1 << Dir_Motor1);
    PORTE |= (1 << Dir_Motor2);
  }
  Prev_SW2_State = Current_SW2_State;


// Update encoder logic
  previous_time = current_time;  
  previous_position = current_position;

}





 // el feedbacko loop for la control of el motors on el lineo 

 void motorControl() {

  int s1Value = analogRead(A3);
  int s2Value = analogRead(A2);
  int s3Value = analogRead(A1);
  int s4Value = analogRead(A0);
  int s5Value = analogRead(8);
  int s6Value = analogRead(7);
  int s7Value = analogRead(11);
  int s8Value = analogRead(6);

  //encoder feedback


  float ideal = 4.5;


  // find postion on IR array 
  float position = (float)(s4Value*4 + s5Value*5 + s6Value*6 + s7Value*7 + s8Value*8) / (float)(s4Value + s5Value + s6Value + s7Value + s8Value);
  Serial.print(position);
  Serial.print("\n");

  float error = ideal - position;
  Serial.print(error);
  Serial.print("\n");

  float Kp = 6.0;
  int basePWM = 150;

  //may be wrong here need to test 
  int LeftMotor = basePWM - (Kp * error);
  int RightMotor = basePWM + (Kp * error);

  PWM_M1 = clamp(LeftMotor, 50, 255);
  PWM_M2 = clamp(RightMotor, 50, 255);



 }

