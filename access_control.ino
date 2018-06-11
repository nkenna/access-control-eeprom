#include <EEPROM.h>
#include <Key.h>
#include <Keypad.h>
#include <LiquidCrystal.h>

const int rs = 43, en = 41, d4 = 39, d5 = 37, d6 = 35, d7 = 33;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int buzzerPin = 12; //buzzer pin
int statusLed = 13; //led pin
int relayPin = 30; //relay pin

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keys[ROWS][COLS] = {
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};

byte rowPins[ROWS] = {22, 24, 26, 28}; //connect to the row pinouts of the keypad
byte colPins[COLS] =  {23, 25, 27, 29};//connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

char new_password[8]; //new password during password updates
char verify_new_password[8]; //verify new password during password updates
char original_password[8]; //password stored in eeprom
char entered_password[8]; //password enterd through keypad
int key = 0;
int attempts = 0;
boolean freezed = false;


void setup() {
  // put your setup code here, to run once:
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.print("Lock System");
  
  Serial.begin(9600);

  //set all useful pins to output
  pinMode(buzzerPin, OUTPUT);
  pinMode(statusLed, OUTPUT);
  pinMode(relayPin, OUTPUT);

  //turn relay low, make sure our lock is locked
  digitalWrite(relayPin, LOW);

  //retrieved default passkey from eeprom
   for (int i = 0; i < 8; i++){
      original_password[i] =  EEPROM.read(i);
       
   }
   
  
   delay(200);

   //notify everbody that the lock is locked
   lcd.setCursor(0, 1);
   lcd.print("System Locked");

   Serial.println("Enter Password:");
   delay(1000);

}

void loop() {
  // put your main code here, to run repeatedly:
  char entered_key = keypad.getKey();

  

  if(entered_key == 'A' &&  key == 0){//if A is pressed and no key have been pressed, enter update mode
    Serial.println("A pressed");
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Update Mode");
    
    //function containing update codes
    updatePassword();
    
  }else if(entered_key == '#'){//if # key is pressed, clear screen, reset entered keys to zero
    lcd.clear();
    lcd.setCursor(0, 0);
    key = 0;
    memset(entered_password, 0, sizeof entered_password);
    Serial.println("");
    
  }else if(entered_key == '*' &&  key == 0){//if * key is pressed, just lock everything back 
    digitalWrite(buzzerPin, LOW);
    digitalWrite(relayPin, LOW);
    digitalWrite(statusLed, HIGH);
    Serial.println("System locked");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Lock System");
    lcd.setCursor(0, 1);
    lcd.print("System locked.....");
    
    }else{
      lcd.setCursor(0, 1);

      if(entered_key){//make sure something was actually pressed
        entered_password[key++] = entered_key; //fill up enter_password char arry with pressed keys till is up to 8
        Serial.print(entered_key);
        }

      if (key == 8){//if it is up to 8, start comparing for correct passkey
        delay(200);

        for (int i = 0; i < 8; i++){
          original_password[i] =  EEPROM.read(i);       
   }

    if(strncmp(original_password, entered_password, 8) == 0){
      
      lcd.setCursor(0, 1);
      lcd.print("Lock Opened......");
      Serial.println("Success");
      
      Serial.print(original_password);
      //digitalWrite(relayPin, HIGH);
      statusSuccess();
   }else{ //ouchs, wrong passkey
    attempts++;//increment attempts

    if (attempts == 3){//if attempts is up to 3, freeze lock
      lcd.setCursor(0, 1);
      lcd.print("Lock Freezed");
      statusFreeze();
      Serial.println("system locked");
      digitalWrite(relayPin, LOW);
      freezed = true;
      attempts = 0;
    }
    Serial.println("no luck, try again");
    statusError(); //function to run if passkey is not coorect
    
  }

  key = 0;
  }

        
       }
 
}

//update passkey function
void updatePassword(){
  
  if (freezed){//if freezed, please don't bother
    lcd.setCursor(0, 1);
    lcd.print("Lock freezed");
    Serial.println("System freezed, enter passkey to continue");
  }else{
    int ukey = 0;
    lcd.setCursor(0, 0);
    lcd.print("Enter cur. Passkey:");
    Serial.println("Enter current passkey:");

    while(ukey < 8){
      char entered_ukey = keypad.getKey();
      
      if(entered_ukey){
        entered_password[ukey++] = entered_ukey;
      
      }
  }
  ukey = 0;
  
Serial.println(entered_password);


//read from eeprom and compare
if(strncmp(original_password, entered_password, 8) == 0){
  Serial.println("Enter new passkey:");
  lcd.setCursor(0, 0);
  lcd.print("Enter new passkey:");
  
  
   while(ukey < 8){
    char entered_ukey = keypad.getKey();

    if(entered_ukey){
      new_password[ukey++] = entered_ukey;
      }
  }
  ukey = 0;
  Serial.println(new_password);

//verify new passkey
  Serial.println("Verify new passkey:");
  lcd.setCursor(0, 0);
  lcd.print("Verify new passkey:");

      
 while(ukey < 8){
    char entered_ukey = keypad.getKey();

    if(entered_ukey){
      verify_new_password[ukey++] = entered_ukey;
      }
  }
  ukey = 0;
  Serial.println(verify_new_password); 

//compare new and verify password
if(strncmp(new_password, verify_new_password, 8) == 0){
  Serial.println("passkey verified");
  Serial.println(verify_new_password);
  lcd.setCursor(0, 0);
  lcd.print("Passkey Verified");
  
  
  //write to eeprom after verification
   for (int i = 0; i < 8; i++){
    EEPROM.write(i, verify_new_password[i]);
    
   }
   delay(500);

   lcd.setCursor(0, 0);
   lcd.print("Passkey Updated");

   Serial.println("passkey update successful");
}else{
  Serial.println("passkey does not match");
}
  
}else{
  Serial.println("passkey not found");
  lcd.setCursor(0, 0);
  lcd.print("passkey not found");
}
  
  }
  
  
}

void statusError(){
  for(int i = 0; i == 2; i++){
    digitalWrite(buzzerPin, HIGH);
    delay(500);
    digitalWrite(buzzerPin, LOW);
    delay(500);
  }
  
}

void statusSuccess(){
  digitalWrite(buzzerPin, HIGH);
  digitalWrite(statusLed, HIGH);
  digitalWrite(relayPin, HIGH);
    delay(500);
   digitalWrite(buzzerPin, LOW);
    digitalWrite(statusLed, LOW);
}

void statusFreeze(){
  digitalWrite(buzzerPin, LOW);
  digitalWrite(statusLed, HIGH);
  delay(5000);
}

