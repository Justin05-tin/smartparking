#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#define SS_PIN 10
#define RST_PIN 9
#define SERVO_PIN 2
#define BUZZER_PIN A0
#define LED_PIN 7

#define IR_SENSOR_1 4
#define IR_SENSOR_2 5
#define IR_SENSOR_3 6
#define BUTTON_PIN A1

MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo servo;
LiquidCrystal_I2C lcd(0x27, 16, 2);

int availableSlots = 3;  
String parkedRFIDs[3] = {"", "", ""};  

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);  

    Serial.begin(9600);
    SPI.begin();
    mfrc522.PCD_Init();
    servo.attach(SERVO_PIN);
    servo.write(0);  
    
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(IR_SENSOR_1, INPUT);
    pinMode(IR_SENSOR_2, INPUT);
    pinMode(IR_SENSOR_3, INPUT);
    pinMode(LED_PIN, OUTPUT);
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("VKU SmartParking");

    delay(2000);
    displayParkingStatus();
    
}

void loop() {
   if (digitalRead(BUTTON_PIN) == LOW) {
        servo.write(90); 
    } else {
        servo.write(0);  
    }
    
 

    displayParkingStatus();  

    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
        String rfid = getRFID();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("RFID: " + rfid);

        int index = findRFID(rfid);

        if (index != -1) {
             parkedRFIDs[index] = "";  
            availableSlots = min(availableSlots + 1, 3);  
            servo.write(90);  

            lcd.setCursor(2, 1);
            lcd.print("See you again!");
            buzzBuzzerTwice();
            delay(2000);
            
            displayAvailableSlots();
            servo.write(0);  
        } 
        else if (availableSlots > 0) {
            // 🚘 Xe vào → availableSlots -1
            int emptyIndex = findEmptySlot();
            if (emptyIndex != -1) {
                parkedRFIDs[emptyIndex] = rfid;  
                availableSlots = max(availableSlots - 1, 0);  
                servo.write(90);  
                
                lcd.setCursor(4, 1);
                lcd.print("Welcome!");
                buzzBuzzerTwice();
                delay(2000);
                
                displayAvailableSlots();
                servo.write(0);  
            }
        }
        else {
         
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Full slot!");
            
            buzzBuzzerLong();  
            delay(2000);
        }
    }

    delay(500);  
    digitalWrite(LED_PIN,HIGH); 
    delay(500);
    digitalWrite(LED_PIN,LOW);
    delay(500);
}

void displayParkingStatus() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("VKU SmartParking");

    lcd.setCursor(0, 1);
    lcd.print("S1:" + String(digitalRead(IR_SENSOR_1) == LOW ? "X " : "V "));
    lcd.print("S2:" + String(digitalRead(IR_SENSOR_2) == LOW ? "X " : "V "));
    lcd.print("S3:" + String(digitalRead(IR_SENSOR_3) == LOW ? "X " : "V "));
    
    delay(2000);
}

// 📊 Hiển thị số chỗ trống sau khi quẹt thẻ
void displayAvailableSlots() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Available: " + String(availableSlots) );

    delay(2000);
}


int findRFID(String rfid) {
    for (int i = 0; i < 3; i++) {
        if (parkedRFIDs[i] == rfid) {
            return i;
        }
    }
    return -1;
}

int findEmptySlot() {
    for (int i = 0; i < 3; i++) {
        if (parkedRFIDs[i] == "") {
            return i;
        }
    }
    return -1;
}


String getRFID() {
    String rfid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        rfid.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
        rfid.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    mfrc522.PICC_HaltA();
    return rfid;
}

void buzzBuzzer(unsigned int duration) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(duration);
    digitalWrite(BUZZER_PIN, LOW);
}


void buzzBuzzerTwice() {
    for (int i = 0; i < 2; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(100);
        digitalWrite(BUZZER_PIN, LOW);
        delay(300);
    }
}

void buzzBuzzerLong() {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1500);  // Kêu 1.5 giây
    digitalWrite(BUZZER_PIN, LOW);
}
