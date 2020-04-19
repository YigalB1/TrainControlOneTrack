// Supporting 2 slaves, based on #CONST value
// HW Atduino NANO based
// I2C: SDA-A4, SCL-A5, 10K ohm pullups
// LEDs: red - pin 10, green-11,blue-12

// 10 March 2018: moving the read from sensor to the I2C interrupt routine
// 11 March 2020: adding debug messages


#include <Wire.h> // for the I2C
const int I2Caddr =10; // can be 9 or 10 in this design. to change between Left and right.
const int trigPin = 2;  // defines Ultrasonic pins numbers
const int echoPin = 3;  // defines Ultrasonic pins numbers
const int redLed = 10;
const int greenLed = 11;
const int blueLed = 12;

const int buzzer = 7;
const int FAR = 40;
const int CLOSE = 20;
const int TIGHT = 5;

const int Sample_Time = 250;  // Sample delay
const int Max_dist_delta = 20; // allowed max of reading change

const long delay_time = 50;

const int US_TIMEOUT = 30000;



// defines variables
//#define  RED    2;
//#define  GREEN  3;
//#define  BLUE   4;
#define  OFF   0;
#define  BLINK 1;
#define  ON    1;

// int I2Caddr = I2C_slave;  // the adress for each I2C slave
int duration,distance, prev_distance, speed;

int x = 0; // for the I2C function
int j=0; // temp
boolean int_flag;   // for I2C handling

void setup() {
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT); 
  pinMode(greenLed, OUTPUT); 
  pinMode(redLed, OUTPUT); 
  pinMode(blueLed, OUTPUT); 
  pinMode(buzzer, OUTPUT); 
  
  
  //check leds - showing signs of live
  digitalWrite(blueLed, HIGH);
  delay(500);
  digitalWrite(blueLed, LOW);
  digitalWrite(greenLed, HIGH);
  delay(500);
  digitalWrite(greenLed, LOW);
  digitalWrite(redLed, HIGH);
  delay(500);
  digitalWrite(redLed, LOW);
  
  Serial.begin(9600); // Starts the serial communication
  Serial.print("---> My I2C adress is: ");
  Serial.println(I2Caddr);
  delay (300);

  Wire.begin(I2Caddr); // join i2c bus as a slave with address #I2Caddr
  // Attach a function to trigger when something is received.
  Wire.onRequest(requestEventREAD); // register event - request for distance
  Wire.onReceive(receiveEventCMD);   // register event - Command from master
  
  Serial.begin(9600);

  distance  = readDistance();
  prev_distance = distance;
  
  Serial.print("finishing setup stage, entering loop ");
  //    Serial.println(sizeof(distance));
}

unsigned long previousMillis;
unsigned long currentMillis;
unsigned long temp_start_Millis;
unsigned long temp_end_Millis;
unsigned long temp_start;
unsigned long temp_end;


// ********* LOOP *************
void loop() {

//  int temp;
//  previousMillis  = millis();
//  currentMillis   = millis();
/*  
  digitalWrite(greenLed, LOW);
  digitalWrite(redLed, LOW);
  digitalWrite(blueLed, LOW);
  digitalWrite(buzzer, LOW);
  
  // to avoid reading of zero (no readings)
  temp_start_Millis = millis();
  temp_start = micros();
  
  // to ensure no interrrupt occured during sensor readings
  // re-read in case of intrerrupted read
  int_flag = false;
  do {
    if (int_flag == true)
      //Serial.println("ReRedaing due to I2C......");
      int_flag = false;
    
    } while (int_flag);
  
  temp_end_Millis = millis();
  temp_end = micros();
  
  if (temp>=2) {
    // Serial.println("* inside IF *");
    distance=temp; // minimal reading of sensor
    }
  else {
    //Serial.print("bad distance reading: ");
    //Serial.print(temp);
    //Serial.print("/");
    //Serial.println(distance);  
  
  }

//  while ( currentMillis - previousMillis < delay_time ) { // just wait
//    currentMillis  = millis();
//    }

  prev_distance = distance;
  
*/
} // of loop


// ************ functions ******************

// ********************************
void receiveEventCMD(int howmany) {
  byte cmd=7,led=7;

//  Serial.print("CMD event. howmany: ");
//  Serial.println(howmany);
  
  int byt_cnt=0;
  while (Wire.available()) { // loop through all but the last
    x = Wire.read();    // receive byte as an integer
    
//    Serial.print("byt_cnt: ");
//    Serial.print(byt_cnt);
//    Serial.print(" x: ");
//    Serial.print(x);
    
    //if (t>0) 
    if (0==byt_cnt ) 
      cmd = x; 
    else if (1==byt_cnt)
      led = x;
    else
      Serial.println("ERROR more than 2 bytes");
      
    if (1==byt_cnt)
      { // for debug: pring command after 2 bytes arrived
      Serial.print(" cmd: ");
      Serial.print(cmd);
      Serial.print(" led: ");
      Serial.println(led);
      }  
    

    if (1==byt_cnt && 2==cmd) { // execue on 2nd byte recieved
      switch (led) {
        case 1:
          digitalWrite(redLed, HIGH);
          break;
        case 2:
          digitalWrite(greenLed, HIGH);
          break;
        case 3:
          digitalWrite(blueLed, HIGH);
          break;
        case 4:
          digitalWrite(redLed, LOW);
          digitalWrite(greenLed, LOW);
          digitalWrite(blueLed, LOW); 
          break;
        default:
          Serial.print(" wrong CMD to led: ");
          Serial.print(led);
          break;
        } // of switch case
    } // of if CMD==2
    
    byt_cnt++;

  } // of while loop
} // of receiveEventCMD 

// *******************************************************
void requestEventREAD() {
  
//  Serial.print("I2C  read event. ");

//  temp_start_Millis = millis();
  
  int read_dist;
  
  // 10 March 18 - moving read to here
  read_dist = readDistance();
  
  // respond with message of 2 bytes
  //Wire.write(lowByte(distance));
  //Wire.write(highByte(distance));
  
  //10 March 18: stop using "distance" global variable, read here
  Wire.write(lowByte(read_dist));
  Wire.write(highByte(read_dist));
  // as expected by master
  //LightLeds(read_dist); // march 15
  Serial.print("in read distance event, dist is: ");
  Serial.println(read_dist);
  int_flag = true;  // to indicate an interrupt occured
                    // only after reading finished
  
}
// *******************************************************
int readDistance() {
  
  int dist;
  temp_start_Millis = millis();
  
  //Serial.println("start reading distance"); // 11 March 2020
  
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculating the distance
  dist = duration*0.034/2;
  Serial.print("Distance: "); // 11 March 2020
  Serial.println(dist); // 11 March 2020
  
  return(dist);
}

// *********** LightLeds *********************************
// *******************************************************
// not used

/*
void LightLeds(int _dist) {
  
  if (_dist>FAR)
    {
    }
    else if (_dist>CLOSE) {
      digitalWrite(greenLed, HIGH);
      //Serial.println("Green");
      } // of else-if
      else if (_dist>TIGHT) {
        digitalWrite(redLed, HIGH); 
        //Serial.println("Red");
        } // of else-if
        else if (_dist<=TIGHT) {
          digitalWrite(blueLed, HIGH); 
          //Serial.println("Blue");
          } // of else-if
        else {
          Serial.print("Shouldnt be here     ");
          } // of else
}
*/