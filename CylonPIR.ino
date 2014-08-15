/*
  Cylon eye scrolling across 32 LEDs
  Brian Gershon, 2013

  This program sets all LEDs to a random color, then moves a Cylon eye back and forth,
  then picks a new background color and starts again.

  Based initially on LED-driver code from:

  Nathan Seidle
  SparkFun Electronics 2011
  
  This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
  
  Controlling an LED strip with individually controllable RGB LEDs. This stuff is awesome.
  
  The SparkFun (individually controllable) RGB strip contains a bunch of WS2801 ICs. These
  are controlled over a simple data and clock setup. The WS2801 is really cool! Each IC has its
  own internal clock so that it can do all the PWM for that specific LED for you. Each IC
  requires 24 bits of 'greyscale' data. This means you can have 256 levels of red, 256 of blue,
  and 256 levels of green for each RGB LED. REALLY granular.
 
  To control the strip, you clock in data continually. Each IC automatically passes the data onto
  the next IC. Once you pause for more than 500us, each IC 'posts' or begins to output the color data
  you just clocked in. So, clock in (24bits * 32LEDs = ) 768 bits, then pause for 500us. Then
  repeat if you wish to display something new.
  
  This example code will display bright red, green, and blue, then 'trickle' random colors down 
  the LED strip.
  
  You will need to connect 5V/Gnd from the Arduino (USB power seems to be sufficient).
  
  For the data pins, please pay attention to the arrow printed on the strip. You will need to connect to
  the end that is the begining of the arrows (data connection)--->
  
  If you have a 4-pin connection:
  Blue = 5V
  Red = SDI
  Green = CKI
  Black = GND
  
  If you have a split 5-pin connection:
  2-pin Red+Black = 5V/GND
  Green = CKI
  Red = SDI
 */

int SDI = 2; //Red wire (not the red 5V wire!)
int CKI = 3; //Green wire

#define STRIP_LENGTH 32 //32 LEDs on this strip
long strip_colors[STRIP_LENGTH];

long CYLON_COLOR = 0x000055;  // medium red

/**
 * Motion Variables
 */
//motion
//the time we give the sensor to calibrate (10-60 secs according to the datasheet)
int calibrationTime = STRIP_LENGTH;  // let's use 32 seconds, the number of leds so we can show status
long unsigned int lowIn;  //the time when the sensor outputs a low impulse
//the amount of milliseconds the sensor has to be low 
//before we assume all motion has stopped
long unsigned int pause = 5000;  
boolean lockLow = true;
boolean takeLowTime;  
int pirPin = 6;    //the digital pin connected to the PIR sensor's output
int ledPin = 13;

boolean motion_detected = false;

/**
 * Main Program
 */
void fillWithColor(long color, long *strip_colors) {
  for(int x = 0 ; x < STRIP_LENGTH ; x++) {
    *(strip_colors + x) = color;
  }
}

void drawEye(int pos, long *colors) {
  int EYE_WIDTH[] = {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 5, 5, 5, 5, 4, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1};
  
  switch (EYE_WIDTH[pos]) {
  case 2:
    *(colors + pos) = CYLON_COLOR;
    *(colors + pos+1) = CYLON_COLOR;
    break;
  case 3:
    *(colors + pos-1) = CYLON_COLOR;
    *(colors + pos) = CYLON_COLOR;
    *(colors + pos+1) = CYLON_COLOR;
    break;
  case 4:
    *(colors + pos-2) = CYLON_COLOR;
    *(colors + pos-1) = CYLON_COLOR;
    *(colors + pos) = CYLON_COLOR;
    *(colors + pos+1) = CYLON_COLOR;
    break;
  case 5:
    *(colors + pos-2) = CYLON_COLOR;
    *(colors + pos-1) = CYLON_COLOR;
    *(colors + pos) = CYLON_COLOR;
    *(colors + pos+1) = CYLON_COLOR;
    *(colors + pos-2) = CYLON_COLOR;
  default:
    *(colors + pos) = CYLON_COLOR;
  }
}

void setupMotion() {
  pinMode(pirPin, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(pirPin, LOW);

  //give the sensor some time to calibrate
  Serial.print("calibrating sensor ");
  
  fillWithColor(0x000055, strip_colors);
  for(int i = 0; i < calibrationTime; i++){
    strip_colors[STRIP_LENGTH - i] = 0x0;
    post_frame();

    Serial.print(".");
    delay(1000);
    }
  Serial.println(" done");
  Serial.println("SENSOR ACTIVE");
  delay(50);
}

void setup() {
  pinMode(SDI, OUTPUT);
  pinMode(CKI, OUTPUT);
  
  fillWithColor(0x0, strip_colors);
    
  randomSeed(analogRead(0));

  Serial.begin(9600);
  setupMotion();
}

long getRandomColor(long intensity) {
  long new_color = 0;
  for(int c = 0 ; c < 3 ; c++){
    new_color <<= 8;
      new_color |= random(intensity); //Give me a number from 0 to intensity (0x11 is min intensity, 0xFF is max intensity)
  }
  return new_color;
}

void checkMotion() {
 if(digitalRead(pirPin) == HIGH){
   digitalWrite(ledPin, HIGH);   //the led visualizes the sensors output pin state
   if(lockLow){  
     //makes sure we wait for a transition to LOW before any further output is made:
     lockLow = false;
     motion_detected = true;
     Serial.println("---");
     Serial.print("motion detected at ");
     Serial.print(millis()/1000);
     Serial.println(" sec"); 
     delay(50);
     }         
     takeLowTime = true;
   }

 if(digitalRead(pirPin) == LOW){       
   digitalWrite(ledPin, LOW);  //the led visualizes the sensors output pin state

   if(takeLowTime){
    lowIn = millis();          //save the time of the transition from high to LOW
    takeLowTime = false;       //make sure this is only done at the start of a LOW phase
    }
   //if the sensor is low for more than the given pause, 
   //we assume that no more motion is going to happen
   if(!lockLow && millis() - lowIn > pause){  
       //makes sure this block of code is only executed again after 
       //a new motion sequence has been detected
       lockLow = true;
       motion_detected = false;       
       Serial.print("motion ended at ");      //output
       Serial.print((millis() - pause)/1000);
       Serial.println(" sec");
       delay(50);
       }
   }
   
   if (motion_detected) {
    // crazy mode
    //    long new_color = getRandomColor(0x11);
    CYLON_COLOR = 0x000055;
    fillWithColor(0x080800, strip_colors);
  } else {
    CYLON_COLOR = 0x000005;  // dim red
    fillWithColor(0x0, strip_colors);
    delay(150);
  }

}

void loop() {
  //Pre-fill the color array with known values
//  strip_colors[0] = 0xFF0000; //Bright Red
//  strip_colors[1] = 0x00FF00; //Bright Green
//  strip_colors[2] = 0x0000FF; //Bright Blue
//  strip_colors[3] = 0x000001; //Faint red
//  strip_colors[4] = 0x800000; //1/2 red (0x80 = 128 out of 256)
//  post_frame(); //Push the current color frame to the strip

  int DELAY[STRIP_LENGTH];
  int speed = 30;
  
  DELAY[0] = 5127/speed;
  DELAY[1] = 2460/speed;
  DELAY[2] = 1912/speed;
  DELAY[3] = 1583/speed;
  DELAY[4] = 1343/speed;
  DELAY[5] = 1150/speed;
  DELAY[6] = 986/speed;
  DELAY[7] = 842/speed;
  DELAY[8] = 710/speed;
  DELAY[9] = 588/speed;
  DELAY[10] = 473/speed;
  DELAY[11] = 363/speed;
  DELAY[12] = 257/speed;
  DELAY[13] = 153/speed;

  DELAY[14] = 0;
  DELAY[15] = 0;
  DELAY[16] = 0;
  DELAY[17] = 0;

  DELAY[18] = 153/speed;
  DELAY[19] = 257/speed;
  DELAY[20] = 363/speed;
  DELAY[21] = 473/speed;
  DELAY[22] = 588/speed;
  DELAY[23] = 710/speed;
  DELAY[24] = 842/speed;
  DELAY[25] = 986/speed;
  DELAY[26] = 1150/speed;
  DELAY[27] = 1343/speed;
  DELAY[28] = 1583/speed;
  DELAY[29] = 1912/speed;
  DELAY[30] = 2460/speed;
  DELAY[31] = 5127/speed / 2;

  while(1) {
//    long new_color = getRandomColor(0x11);

    for(int y = 0 ; y < STRIP_LENGTH ; y++) {
//      fillWithColor(new_color, strip_colors);
      checkMotion();
      if (motion_detected) {
      } else {
      }
      drawEye(y, strip_colors);
      post_frame();
      delay(DELAY[y]);
    }

    for(int y = STRIP_LENGTH - 1; y > 0 ; y--) {
//      fillWithColor(new_color, strip_colors);
      checkMotion();
      drawEye(y, strip_colors);
      post_frame();
      delay(DELAY[y]);
    }

  }
}

//Takes the current strip color array and pushes it out
void post_frame (void) {
  //Each LED requires 24 bits of data
  //MSB: R7, R6, R5..., G7, G6..., B7, B6... B0 
  //Once the 24 bits have been delivered, the IC immediately relays these bits to its neighbor
  //Pulling the clock low for 500us or more causes the IC to post the data.

  for(int LED_number = 0 ; LED_number < STRIP_LENGTH ; LED_number++) {
    long this_led_color = strip_colors[LED_number]; //24 bits of color data

    for(byte color_bit = 23 ; color_bit != 255 ; color_bit--) {
      //Feed color bit 23 first (red data MSB)
      
      digitalWrite(CKI, LOW); //Only change data when clock is low
      
      long mask = 1L << color_bit;
      //The 1'L' forces the 1 to start as a 32 bit number, otherwise it defaults to 16-bit.
      
      if(this_led_color & mask) 
        digitalWrite(SDI, HIGH);
      else
        digitalWrite(SDI, LOW);
  
      digitalWrite(CKI, HIGH); //Data is latched when clock goes high
    }
  }

  //Pull clock low to put strip into reset/post mode
  digitalWrite(CKI, LOW);
  delayMicroseconds(500); //Wait for 500us to go into reset
}
