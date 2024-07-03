#include <LiquidCrystal_I2C.h>
#include <RotaryEncoder.h>
#include <Keypad.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include "StackArray.h"
#include "tokenize.h"

// Define rotary encoder pins
const int  PIN_IN1 = 2;
const int PIN_IN2 = 3;

/* Display */
LiquidCrystal_I2C lcd(0x27, 16, 2);
RotaryEncoder encoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::TWO03);

int lastClkState;
int cursorPos;
int row;
int col;


bool shift_key = false;
char last_key;

// Variables for calculating final result
String memory = "";

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


long lastDebounceTime = 0;
const long debounceDelay = 40; // Debounce delay in milliseconds

void PlaceCursor()
{
  row = cursorPos / 16;
  col = cursorPos % 16;
  lcd.setCursor(col, row);
  lcd.cursor();
  Serial.println(col);
}

void UpdateRotary()
{
  static int pos = 0;
  encoder.tick();
  int newPos = encoder.getPosition();
  if (pos != newPos)
  {
    long currentTime = millis();
    if ((currentTime - lastDebounceTime) > debounceDelay)
    {
      int temp = newPos - pos;

      cursorPos -= temp;

      pos = newPos;
      PlaceCursor();
      lastDebounceTime = currentTime;
    }
    else
    {
      encoder.setPosition(pos);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lastClkState = digitalRead(PIN_IN1);
  lcd.setCursor(0, 0);
  lcd.cursor();
}

/**
   Check if a char is digit.

   @param chr Input char
   @return True if the char is digit
*/
bool IsDigit(char chr)
{
  return (chr >= '0' && chr <= '9');
}

/**
   Check if a char is Operator.

   @param chr Input char
   @return True if the char is operator
*/
bool IsOperator(char chr)
{
  return chr == '+' || chr == '-' || chr == '*' || chr == '/' || chr == '^';
}

/**
   Check if a string is number.

   @param input_number Input char
   @return True if the string is a number
*/
bool IsNumber(const String &input_nubmer)
{
  int size_of_input_number = strlen(input_nubmer.c_str());
  for (int i = 0; i < size_of_input_number; i++)
  {
    if (!IsDigit(input_nubmer[i]))
    {
      return false;
    }
  }
  return true;
}

/**
   Determine priority of an operator.

   @param op Input char
   @return its priority as a number
*/
int Precedence(char op)
{
  if (op == '+' || op == '-')
    return 1;
  if (op == '*' || op == '/')
    return 2;
  if (op == '^')
    return 3;
  return 0;
}

/**
   Apply operator on the operands.

   @param first_number Input char
   @param second_number Input char
   @return the result of operation
*/
double ApplyOperator(uint32_t first_number, uint32_t second_number, char op)
{
  switch (op)
  {
    case '+':
      return first_number + second_number;
    case '-':
      return second_number - first_number;
    case '*':
      return first_number * second_number;
    case '/':
      return second_number / static_cast<double>(first_number);
    case '^':
      return pow(second_number, first_number);
    default:
      return 0;
  }
}

/**
   Parse and evaluate a mathematical expression

   @param expression Input char
   @return the result of a methematical expression
*/
double EvaluateExpression(const String &expression)
{
  StackArray<char> operators;
  StackArray<double> operands;

  Tokenize tokenize(expression);

  // Parse the expression token by token
  while (true)
  {
    String token = tokenize.GetNext();

    // break when token is empty
    if (strlen(token.c_str()) == 0)
    {
      break;
    }
    // If the token is a number
    if (IsNumber(token))
    {
      char *endptr;
      uint32_t first_number = strtoul(token.c_str(), &endptr, 10);
      operands.push(first_number);
    }
    // If the token is an operator
    else if (IsOperator(token[0]))
    {
      char current_operator = token[0];
      // While the operator stack is not empty and the top operator has equal or higher precedence
      while (!operators.isEmpty() && Precedence(operators.peek()) >= Precedence(current_operator))
      {
        // Pop two operands and one operator
        uint32_t first_previous_operand = operands.peek();
        operands.pop();
        uint32_t second_previous_operand = operands.peek();
        operands.pop();
        char previous_operator = operators.peek();
        operators.pop();
        // Apply the operator to the operands and push the result onto the operand stack
        operands.push(ApplyOperator(first_previous_operand, second_previous_operand, previous_operator));
      }
      // Push the current operator onto the operator
      operators.push(current_operator);
    }
    else if (token[0] == '(')
    {
      // If the token is an opening parenthesis Push it onto the operator stack.
      operators.push('(');
    }
    else if (token[0] == ')')
    { // If the token is a closing parenthesis While the operator stack is not empty and the top operator is not an opening parenthesis
      while (!operators.isEmpty() && operators.peek() != '(')
      {
        // Pop two operands and one operator
        uint32_t first_previous_operand = operands.peek();
        operands.pop();
        uint32_t second_previous_operand = operands.peek();
        operands.pop();
        char previous_operator = operators.peek();
        operators.pop();
        // Apply the operator to the operands and push the result onto the operand stack
        operands.push(ApplyOperator(first_previous_operand, second_previous_operand, previous_operator));
      }
      // Pop the opening parenthesis
      operators.pop();
    }
  }

  // While the operator stack is not empty
  while (!operators.isEmpty())
  {
    // Pop two operands and one operator
    uint32_t first_previous_opernad = operands.peek();
    operands.pop();
    uint32_t second_previous_operand = operands.peek();
    operands.pop();
    char op = operators.peek();
    operators.pop();
    // Apply the operator to the operands and push the result onto the operand stack
    operands.push(ApplyOperator(first_previous_opernad, second_previous_operand, op));
  }

  return operands.peek();
}

/**
  Recognize operation for LCD to print.

  @param key Input char
*/
void RecognizeOperationforLCD(char key)
{
  switch (key)
  {
    case 'A':
      if (shift_key == true)
      {
        lcd.print("(");
        memory += "(";
      }
      else
      {
        lcd.print("+");
        memory += "+";
      }
      break;
    case 'B':
      if (shift_key == true)
      {
        lcd.print(")");
        memory += ")";
      }
      else
      {
        lcd.print("-");
        memory += "-";
      }
      break;
    case 'C':
      if (shift_key == true)
      {
        lcd.print("^");
        memory += "^";
      }
      else
      {
        lcd.print("*");
        memory += "*";
      }
      break;
    case 'D':
      if (shift_key == true)
      {

        String bufinput = memory;
        memory = "";
        for (int i = 0 ; i < bufinput.length() ; i++)
        {
          if (i != cursorPos)
          {
            memory += bufinput[i];
          }
        }
        lcd.clear();
        lcd.print(memory);

      }
      else {
        lcd.print("/");
        memory += "/";
      }
      break;
  }
  last_key = key;
}

/**
   Print final result on LCD.
*/
void PrintResult()
{
  char buffer[10];
  double result = EvaluateExpression(memory);
  dtostrf(result, 6, 4, buffer);
  lcd.setCursor(0, 1);
  lcd.print(buffer);
}

/**
   Process keyboard input.

   @param key Input char
*/
void ProcessInput(char key)
{
  if ('B' == key && memory == "" && !shift_key)
  {
    memory += "-";
    lcd.print("-");
    return;
  }

  switch (key)
  {
    case 'A':
    case 'B':
    case 'C':
    case 'D':
      RecognizeOperationforLCD(key);
      return;
    case '*':
      if (shift_key)
      {
        if (last_key == '*')
        {
          shift_key = false;
          lcd.clear();
          lcd.setCursor(0, 0);
          memory = "";
          return;
        }
      }
      shift_key = !shift_key;
      RecognizeOperationforLCD(key);
      return;
    case '#':
      PrintResult();
      shift_key = false;
      return;
  }

  if (key)
  {
    if (col == -1) {
      String temp = memory;
      memory = String(key) + temp;
    }
    else if (col < memory.length() && col != 0 && col != -1) {
      memory = memory.substring(0, col) + String(key) + memory.substring(col);
    } else {
      memory += String(key);
    }

  }
  lcd.clear();
  lcd.print(memory);
}

void loop()
{
  delay(2);
  UpdateRotary();
  char key = keypad.getKey();
  if (key)
  {
    ProcessInput(key);
  }
}
