#include <LiquidCrystal.h>

const int rs = 2, en = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
int ledpin[] = {13,12,11};
int controlbtn[] = {8,9,10}; // power, pause, reset
int pressed = -1; // declaration of flag variable, used with button presses to change state of device board


char colors[] = {'Y','P','R','G','O'}; // shorthand for yellow, purple, red, green, orange
int colorsCount[] = {0,0,0,0,0}; // list for storing counts of colours, starts at 0 for all colours
bool sorting = false; // definition of boolean variable 'sorting' to be used for on/off status of device board

void setup() {
  for (int i = 0; i < 3; i++) {
    pinMode(ledpin[i], OUTPUT);
    pinMode(controlbtn[i], INPUT);
  }
  lcd.begin(16, 2); // definition of number of rows and columns in LCD panel to be used
  updateDisplay(); // calls function to refresh/update the display
  Serial.begin(9600);
  
}

void loop() {
  if (pressed == -1) {
    if (digitalRead(controlbtn[0]) == HIGH) { // power button when pressed
        pressed = 0; // condition met, flag changed to 0
        sorting = !sorting; // changes boolean variable 'sorting' to opposite value of value at the time
        updateSorter(); // calls serial communication for other board to update status as of status of boolean variable 'sorting'
        updateDisplay(); // calls function to refresh/update the display
    } else if (digitalRead(controlbtn[1]) == HIGH) { // pause button when pressed
        pressed = 1; // condition met, flag changed to 1
        sorting = !sorting; // changes boolean variable 'sorting' to opposite value of value at the time
        updateSorter(); // calls serial communication for other board to update status as of status of boolean variable 'sorting'
        updateDisplay(); // calls function to refresh/update the display
    } else if (digitalRead(controlbtn[2]) == HIGH) { // reset button when pressed
        pressed = 2;  // condition met, flag changed to 2
        for (int i = 0; i < 5; i++){
          colorsCount[i] = 0; // for loop changes each element in list 'colorsCount' to 0, resets list to 0 for each element in list
        }
      updateDisplay(); // display is updated to reflect new reset data values for 'colorsCount' list
    }
  } else {
    if (digitalRead(controlbtn[pressed]) == LOW){
      if (pressed == 1) { // if the button pressed was the temporary toggle, then the sorting bool is untoggled
        sorting = !sorting;
        updateSorter();
      }
      pressed = -1;
      updateDisplay(); // updates display with the new device state if there is one
    }
  }
  
  if (Serial.available() > 0) { // detects incoming data
    colorsCount[Serial.read()]++; // adds 1 to the count of whatever color was recieved through the serial port
    updateDisplay();
  }
}

void updateSorter() { // function tells other board a 1 to run the sorting process, else 0 for the process to be off
  if (sorting) {
    Serial.print(1);
  } else {
    Serial.print(0);
  }
  
}

void updateLED() { // function updates the colour of the LED bulb depending on status of 'sorting'
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledpin[i], LOW);
  }
  if (pressed == 1) {
    digitalWrite(ledpin[0], HIGH);  
    digitalWrite(ledpin[2], HIGH);  // temporary on state, showing purple via ledpin[0] and [2] both being on
  } else if (sorting) {
    digitalWrite(ledpin[1], HIGH);  // on state, turning on ledpin[1], green
  } else {
    digitalWrite(ledpin[0], HIGH); // off state, turning on ledpin[0], red
  }
}

void updateDisplay() { // function updates characters displayed on LCD display depending on colorCount list and the 'sorting' boolean value
  updateLED();
  lcd.clear();
  if (sorting) {
    lcd.print("ON"); // sorting = true, print ON
  } else {
    lcd.print("OFF"); // sorting = false, print OFF
  }
  int row = 1;
  for (int i = 0; i < 5; i++) {
    lcd.setCursor(2+ceil(i*2.5), row);
    lcd.print(colors[i]); // print shorthand colour names
    lcd.print(":"); 
    lcd.print(colorsCount[i]); // print the current values of each element in colorsCount for every i
    if (row == 1) { // toggles the row number because the values are printed in a zig zag fasion going up and down
      row = 0;
    } else {
      row = 1;
    }
  }
}
