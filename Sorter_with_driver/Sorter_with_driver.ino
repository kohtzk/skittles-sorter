#include <Servo.h>

// defining the color sensor pins
#define S0 2
#define S1 3
#define S2 4
#define S3 5
#define sensorOut 6

// setting the stepper pin and the pin the brushes are wired to
const int stopPin = 8;
const int stepperPin = 7;

bool sorting = false;
bool onStrip = false;

Servo switcher;
int oldcol = 2;
int oldercol = 2;

void setup() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);
  pinMode(stepperPin, OUTPUT);
  
  switcher.attach(9);
  setServo(2);
  
  // Setting frequency-scaling to 20%
  digitalWrite(S0,HIGH);
  digitalWrite(S1,LOW);

  // start serial communication
  Serial.begin(9600);
}
void loop() {
  int rgbw[4]; // initialises the red, green, blue, white array for storing color values
  if (Serial.available() > 0) { // if recieving information from the controller arduino
    int intin = Serial.parseInt(); // parse as int
    if (intin == 0) { // if 0, then turn off the sorter / stop sorting
      sorting = false;
    } else if (intin == 1){ // if 1 then turn on the sorter / start sorting
      sorting = true;
    } // not just an if else to try and reduce false signals
  }
  // if the sorter is on/sorting, then continue sorting
  if (sorting) {
    // detects if the brushes are touching a metal strip
    int buttonState = digitalRead(stopPin);
    if (buttonState == HIGH && !onStrip) { // if the brushes are on the strip but weren't already on the strip
        onStrip = true;
        getavgrgb(rgbw, 20);
        oldercol = oldcol;
        oldcol = getcolor(rgbw);
        // sends the color of the current skittle to the controller so it can keep count
        Serial.write(oldcol); 
        delay(100); // waits to give the skittle time to go down into its appropriate bin
        step(50); // moves the stepper by a bit to give the skittle more time to go down the slope before changing where it points
        setServo(oldercol); // starts moving the servo so its in the right position in time
        step(100); // moves the stepper on a bit to try and clear it from the metal strip - this is to stop it accidentally counting the same strip twice
    } else if (buttonState == LOW && onStrip) {
        onStrip = false; // if no longer contacting one of the metal strips, then change onStrip to false
    }
    step(2); // advance the stepper motor
  }
}

void step() {
  // sends a pulse on the stepper pin which tells the driver to advance the stepper motor one step in the chosen direction
  digitalWrite(stepperPin, HIGH);
  delayMicroseconds(1500);
  digitalWrite(stepperPin, LOW);
  delayMicroseconds(1500);
}

void step(int steps) {
  // executes the step function to advance the stepper motor 'steps' times
  for (int i = 0; i < steps; i++) {
    step();
  }
}

void setServo(int bin) {
  // this sets the servo to the correct position depending on what color the skittle is
  // a switch statement was used instead of  35+bin*36 because the servo was slightly off for some of them, and it wasnt a consistent difference
  switch (bin) {
    case 0:
      switcher.write(35);
      break;
    case 1:
      switcher.write(66);
      break;
    case 2:
      switcher.write(102);
      break;
    case 3:
      switcher.write(138);
      break;
    case 4:
      switcher.write(177);
      break;
  }
}

void getavgrgb(int *rgbw, int rounds) {
  // resets the rgbw array to all zeros
  for (int i = 0; i < 4; i++) {
    rgbw[i] = 0;
  }
  // takes 'rounds' sets of readings and puts the total for each color in the array
  for (int i = 0; i < rounds; i++) {
    // sets the color sensor to 'red' mode
    digitalWrite(S2,LOW);
    digitalWrite(S3,LOW);
    // scales the reading to be between 0 and 255
    rgbw[0] += map(pulseIn(sensorOut, LOW), 2800, 940, 0, 255);
    
    // sets the color sensor to 'green' mode
    digitalWrite(S2,HIGH);
    digitalWrite(S3,HIGH);
    // scales the reading to be between 0 and 255
    rgbw[1] += map(pulseIn(sensorOut, LOW), 3500, 940, 0, 255);
    
    // sets the color sensor to 'blue' mode
    digitalWrite(S2,LOW);
    digitalWrite(S3,HIGH);
    // scales the reading to be between 0 and 255
    rgbw[2] += map(pulseIn(sensorOut, LOW), 2500, 690, 0, 255);
    
    // sets the color sensor to 'white' mode
    digitalWrite(S2,HIGH);
    digitalWrite(S3,LOW);
    // scales the reading to be between 0 and 255
    rgbw[3] += map(pulseIn(sensorOut, LOW), 950, 285, 0, 255);

    // the values are different for the scaling for each color because there are different intensities of ambient light that are detected
    // the high and low values for each color was found by measuring the values when there was a white or black card in front of the sensor
  }
  // finds the average from all the rounds of readings by dividing by the amount of rounds
  for (int i = 0; i < 4; i++) {
    rgbw[i] /= rounds;
  }
}



// returns 0-4 for yellow, purple, red, green, orange
int getcolor(int *rgbw) {
  int total = 0;
  int maxlight = -1000;
  int brightest = -1;
  int nextbrightest = -1;

  // adjustments
  rgbw[0] -= 15; // lowers red value

  // finds the brightest color sensed by the color sensor excluding white
  // also finds the total brightness of all colors combined
  for (int i = 0; i < 4; i ++) {
    total += rgbw[i];
    // if the current color is brighter than the current brightest color then it is now the current brightest
    if (rgbw[i] > maxlight && i < 3) {
      maxlight = rgbw[i];
      brightest = i;
    }
  }
  maxlight = -1000;
  //finds the brightest color in the array excluding the brightest color, i.e. the second brightest color, excluding white
  for (int i = 0; i < 3; i ++) {
    if (rgbw[i] > maxlight &&  i != brightest) {
      maxlight = rgbw[i];
      nextbrightest = i;
    }
  }
  if (brightest == 0) { //brightest = red
    if (nextbrightest == 2 || rgbw[1] == rgbw[2]) { // nextbrightest = blue
      if (total < 1100) {
        if (rgbw[0] - rgbw[2] < rgbw[2] - rgbw[1]) { // if red-blue < blue-green
          return 1; // purple
        } else {
          return 2; // red
        }
      } else {
        return 4; // orange
      }
      
    } else { // nextbrightest = green
        if (rgbw[1] > (rgbw[0] + rgbw[2])/2) {
          return 0; //yellow
        } else if (rgbw[1] > (rgbw[0] + rgbw[2]*9)/10) {
          return 4; // orange
        } else {
          return 2; // red
        }
    }
  } else if (brightest == 1) { //brightest = green
    if (nextbrightest == 0) { // nextbrightest = red
      if (rgbw[1] - rgbw[0] < rgbw[0] - rgbw[2]) { // if green-red < red-blue
        return 0; // yellow
      } else {
        return 3; // green
      }
    } else {
      return 3; // green
    }
  } else { //brightest = blue
    return 1; // purple
  }
}










  
