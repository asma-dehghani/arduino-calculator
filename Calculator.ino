#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <ctype.h>
#include <stdlib.h>

/* Display */
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Define rotary encoder pins
const int kDataPin = 2;
const int kClockPin = 3;
volatile int volatile_rotary_counter = 0;  // Stores encoder position change

int cursor_index = 0;

/* Keypad setup */
const byte kKeypadRows = 4;
const byte kKeypadColumns = 4;
byte row_pins[kKeypadRows] = {4, 5, 6, 7};
byte column_pins[kKeypadColumns] = {8, 9, 10, 11};
char keys[kKeypadRows][kKeypadColumns] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

Keypad keypad = Keypad(makeKeymap(keys), row_pins, column_pins, kKeypadRows, kKeypadColumns);

void UpdateCursor() {
  if (millis() / 250 % 2 == 0) {
    lcd.cursor();
  } else {
    lcd.noCursor();
  }
}

void setup() {
  Serial.begin(115200);
  lcd.begin(16, 2);

  // Set rotary encoder pins as inputs with internal pull-ups
  pinMode(kDataPin, INPUT_PULLUP);
  pinMode(kClockPin, INPUT_PULLUP);

  // Attach interrupt for rotary encoder clicks
  attachInterrupt(digitalPinToInterrupt(kDataPin), RotaryEncoderChange, CHANGE);
}

String memory = "";
String current = "";
char operation = 0;

double Calculate(char operation, uint32_t first_num, uint32_t second_num) {
  switch (operation) {
    case 'A':
      return first_num + second_num;
    case 'B':
      return first_num - second_num;
    case 'C':
      return first_num * second_num;
    case 'D':
      return first_num / second_num;
    default:
      return 0.0;
  }
}

void RotaryEncoderChange(String input_str) {
  int reading = digitalRead(kClockPin);

  if (reading != digitalRead(kDataPin)) {
    if (digitalRead(kDataPin) == LOW) {
      volatile_rotary_counter++;
    } else {
      volatile_rotary_counter--;
    }
  }

  // Update cursor position based on encoder movement and constrain within string bounds
  cursor_index = (cursor_index + volatile_rotary_counter + input_str.length() - 1) % input_str.length();
  volatile_rotary_counter = 0;  // Reset encoder count for next change
  int current_row = cursor_index < 16 ? 0 : 1;  // Determine row based on cursor_index
  lcd.setCursor(cursor_index, current_row);
}

void RecognizeOperation(char key) {
  switch (key) {
    case 'A':
      lcd.print("+");
      break;
    case 'B':
      lcd.print("-");
      break;
    case 'C':
      lcd.print("x");
      break;
    case 'D':
      lcd.print("/");
      break;
  }
}

void PrintResult(String first_num, String second_num) {
  char *endptr;
  uint32_t first_number = strtoul(first_num.c_str(), &endptr, 10);
  uint32_t second_number = strtoul(second_num.c_str(), &endptr, 10);
  memory = String(Calculate(operation, first_number, second_number));
  lcd.setCursor(0, 1);
  lcd.print("= " + memory);
}

void ProcessInput(char key) {
  if ('B' == key && current == "") {
    current = "-";
    lcd.print("-");
    return;
  }

  switch (key) {
    case 'A':
    case 'B':
    case 'C':
    case 'D':
      RecognizeOperation(key);
      if (!operation) {
        memory = current;
        current = "";
      }
      operation = key;
      return;
    case '#':
      PrintResult(memory, current);
      return;
  }

  if (current == "0") {
    current = String(key);
  } else if (key) {
    current += String(key);
  }

  lcd.print(key);
}

void loop() {
  UpdateCursor();

  char key = keypad.getKey();
  if (key) {
    ProcessInput(key);
  }
}
