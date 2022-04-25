#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>
#include <Wire.h>
#include <Keypad.h>
#include <Password.h>
#include <ArduinoJson.h>

StaticJsonDocument<200> doc;
char msg[100];

uint8_t i = 0;    // Variable used for counter
int attempts = 0; // Attempts is used to store the number of wrong attempts
boolean unlocked = false;

const byte ROWS = 4; // Four rows
const byte COLS = 3; // Three columns

// Keypad pinmap
char keys[ROWS][COLS] = {
  { '1','2','3' },
  { '4','5','6' },
  { '7','8','9' },
  { '*','0','#' }
};
// Initializing pins for keypad
byte rowPins[ROWS] = {13, 12, 11, 10};  //R1,R2,R3,R4
byte colPins[COLS] = {9, 8, 7};         //C1,C2,C3

// Create instance for keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
SoftwareSerial unoSerial(2, 3); //Fingerprint pins (RX,TX)
SoftwareSerial espSerial(4, 5);

// Fingerprint
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&unoSerial);
Password password = Password("1234");

void setup(){
  // Add an event listener for this keypad
  keypad.addEventListener(keypadEvent);
  keypad.setDebounceTime(100);
  
  pinMode(6,OUTPUT);
  
  // Set the data rate
  Serial.begin(9600);
  espSerial.begin(9600);
  finger.begin(57600);
  
  if (finger.verifyPassword()) 
  {
    Serial.println("Found fingerprint sensor!");
  } 
  else 
  {
    Serial.println("Did not find fingerprint sensor!");
    while (1);
  }
  Serial.println("Waiting for valid finger...");
}

int getFingerprintIDez() 
{ 
  if((finger.getImage() | finger.image2Tz() | finger.fingerFastSearch() ) != FINGERPRINT_OK)
  {
    return -1;
  }
  else {
    // Found a match!
    unlocked = true;

    doc["STATUS"] = "Unlocked";
    doc["METHOD"] = "FingerPrint";
    doc["PASS"]   = "ID " + String(finger.fingerID);
    serializeJson(doc, msg);
    espSerial.println(msg);
    
    Serial.print("Found ID #"); 
    Serial.print(finger.fingerID);
    Serial.print(" with confidence of "); 
    Serial.println(finger.confidence);
   
    return finger.fingerID;
  }      
}

void keypadEvent(KeypadEvent KEYS) 
{
  switch (keypad.getState())
  {
  case PRESSED:
    Serial.print("Pressed: ");
    Serial.println(KEYS);
    i++;
    switch (KEYS)
    {
    case '#': 
      guessPassword(); 
      break;
    default:
      password.append(KEYS);
    }
  }
}

void guessPassword() 
{
  if (password.evaluate())
  {
    Serial.println("VALID PASSWORD ");
    unlocked = true;

    doc["STATUS"] = "Unlocked";
    doc["PASS"]   = "Valid";
    doc["METHOD"] = "Keypad";
    serializeJson(doc, msg);
    espSerial.println(msg);
    
    i=0;
    password.reset(); // Resets password after CORRECT entry
  }
  else
  {
    attempts = attempts + 1;
    if(attempts >= 3)
    { 
      doc["STATUS"] = "Lockout";
      doc["PASS"]   = "Invalid";
      serializeJson(doc, msg);
      espSerial.println(msg);
      Serial.println("LOCKOUT 10S");

      delay(30000); //Delay 10s and finally set attempts equal to zero
      attempts = 0;
    }
    
    Serial.println("INVALID PASSWORD ");
    doc["STATUS"] = "Locked";
    doc["PASS"] = "Invalid";
    serializeJson(doc, msg);
    espSerial.println(msg);
    
    i=0;
    password.reset(); // Resets password after INCORRECT entry
  }
}

void PasswordIsValid()
{
  digitalWrite(6, HIGH); 
  delay(5000);
  digitalWrite(6, LOW);
  
  unlocked = false;
  
  doc["STATUS"] = "Locked";
  doc["METHOD"] = "Auto";
  doc["PASS"]   = "Empty";
  serializeJson(doc, msg);
  espSerial.println(msg);
}

void loop() 
{
  // Storing keys
  char KEYS = keypad.getKey(); 
  getFingerprintIDez();
  
  // Check to see if either the finger or password is valid
  if(unlocked) PasswordIsValid();
}
