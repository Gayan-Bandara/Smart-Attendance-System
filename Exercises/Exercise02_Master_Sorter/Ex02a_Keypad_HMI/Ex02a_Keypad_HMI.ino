/*
  Exercise 02a — Human-Machine Interface (Keypad + LCD)
  IIoT-Batch02 | Week 02

  Components:
  - 4x4 Matrix Keypad
  - LCD I2C 16x2

  Behavior:
  - Type digits to enter target weight
  - Press * to clear input
  - Press # to save and display the target weight
*/

#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

// ─── LCD (I2C address 0x27, 16 cols, 2 rows) ─────────────────────────────────
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ─── Keypad Layout ────────────────────────────────────────────────────────────
const byte ROWS = 4, COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[4] = {32, 33, 25, 26};
byte colPins[4] = {27, 14, 12, 13};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String inputStr  = "";
int targetWeight = 0;

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Enter weight(g):");
}

void loop() {
  char key = keypad.getKey();
  if (!key) return;

  if (key == '*') {
    // Clear input
    inputStr = "";
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Cleared!");
    delay(600);
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Enter weight(g):");

  } else if (key == '#') {
    // Save target weight
    targetWeight = inputStr.toInt();
    inputStr = "";
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Target saved:");
    lcd.setCursor(0, 1); lcd.print(targetWeight); lcd.print(" g");
    Serial.print("Target weight set to: ");
    Serial.print(targetWeight); Serial.println(" g");

  } else {
    // Append digit
    inputStr += key;
    lcd.setCursor(0, 1);
    lcd.print("                "); // clear row
    lcd.setCursor(0, 1);
    lcd.print(inputStr + " g");
  }
}
