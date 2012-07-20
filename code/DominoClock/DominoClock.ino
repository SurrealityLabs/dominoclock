#include <Wire.h>
#include "RTC.h"
#include "DateTime.h"
#include "wiring_private.h"

#include "CommandShell.h"
CommandShell CommandLine;

RTC_DS1307 RTC;

#define DataPinPort PORTD
#define DataPinTris DDRD
#define DataPinBit 4

#define LoadPinPort PORTD
#define LoadPinTris DDRD
#define LoadPinBit 3

#define ClockPinPort PORTD
#define ClockPinTris DDRD
#define ClockPinBit 2

uint8_t TopLookup[]    = {0b00000000, 0b00001000, 0b00010001, 0b01001100, 0b01010101, 0b01011101, 0b01110111};
uint8_t BottomLookup[] = {0b00000000, 0b00010000, 0b00100010, 0b10011000, 0b10101010, 0b10111010, 0b11101110};

/* UART command set array, customized commands may add here */
commandshell_cmd_struct_t uart_cmd_set[] =
{
  {
    "setDate", "\tsetDate [day] [month] [year]", setDateFunc      }
  ,
  {
    "setTime", "\tsetTime [hours] [minutes] [seconds]", setTimeFunc      }
  ,
  {
    "printTime", "\tprintTime", printTimeFunc      }
  ,
  {
    0,0,0      }
};

unsigned char daysInMonth[] = {
  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

int setDateFunc(char * args[], char num_args) {
  if(3 != num_args) {
    Serial.println(F("Insufficient arguments!"));
    return 1;
  }

  int dayNum = atoi(args[0]);
  int monthNum = atoi(args[1]);
  int yearNum = atoi(args[2]);

  if((yearNum < 2000) || (yearNum > 2100)) {
    Serial.println(F("Invalid year!"));
    return 2;
  }

  if((monthNum < 1) || (monthNum > 12)) {
    Serial.println(F("Invalid month!"));
    return 3;
  }

  // Leap year handling
  if(monthNum == 2) {
    if(yearNum % 4 == 0) {
      if(yearNum % 100 == 0) {
        if(yearNum % 400 == 0) {
          daysInMonth[1] = 29;//is a leap year
        } 
        else {
          daysInMonth[1] = 28;//not a leap year
        } 
      } 
      else {
        daysInMonth[1] = 29;//is a leap year
      }
    } 
    else {
      daysInMonth[1] = 28;// not a leap year
    } 
  }

  if((dayNum < 1) || (dayNum > (daysInMonth[monthNum - 1]))) {

    Serial.println(F("Invalid day!"));
    return 4;
  }
  DateTime setTime = RTC.now();
  setTime.setYear(yearNum);
  setTime.setMonth(monthNum);
  setTime.setDay(dayNum);
  RTC.adjust(setTime);

  Serial.print(F("Setting date to "));
  Serial.print(dayNum);
  Serial.print('/');
  Serial.print(monthNum);
  Serial.print('/');
  Serial.println(yearNum);
  return 0;  
}

int setTimeFunc(char * args[], char num_args) {
  if(3 != num_args) {
    Serial.println(F("Insufficient arguments!"));
    return 1;
  }

  int hourNum = atoi(args[0]);
  int minNum = atoi(args[1]);
  int secNum = atoi(args[2]);

  if((hourNum < 0) || (hourNum > 23)) {
    Serial.println(F("Invalid hours!"));
    return 2;
  }

  if((minNum < 0) || (minNum > 59)) {
    Serial.println(F("Invalid minutes!"));
    return 3;
  }

  if((secNum < 0) || (secNum > 59)) {

    Serial.println(F("Invalid seconds!"));
    return 4;
  }
  DateTime setTime = RTC.now();
  setTime.setHour(hourNum);
  setTime.setMinute(minNum);
  setTime.setSecond(secNum);
  RTC.adjust(setTime);

  Serial.print(F("Setting time to "));
  Serial.print(hourNum);
  Serial.print(F(":"));
  Serial.print(minNum);
  Serial.print(F(":"));
  Serial.println(secNum);
  return 0;  
}

int printTimeFunc(char * args[], char num_args) {
  Serial.print(F("The current time is:"));
  DateTime now = RTC.now();
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
  return 0;
}

void setup() {
    Serial.begin(19200);
    Serial.println("Starting");

    Wire.begin();
    RTC.begin();
    if (! RTC.isrunning()) {
        Serial.println("RTC is NOT running!");
        // following line sets the RTC to the date & time this sketch was compiled
        RTC.adjust(DateTime(__DATE__, __TIME__));
    }
    Serial.println("RTC setup");
    
    CommandLine.commandTable = uart_cmd_set;
    CommandLine.init(&Serial);
    
    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
    
    // Clear the display
    FastShiftOut(0);
    FastShiftOut(0);
    FastShiftOut(0);
    FastShiftOut(0);
    FastShiftOut(0);
    FastShiftOut(0);
    analogWrite(5, 0);
}

void loop() {
    static uint8_t runFlag = 0;
    static uint8_t hours1 = 0, hours2 = 0, tenmin1 = 0, tenmin2 = 0, min1 = 0, min2 = 0;
    uint8_t i, j, k;
    DateTime now = RTC.now();
    CommandLine.runService();
    
    if((runFlag == 0) || (now.second() == 0)) {
        runFlag = 1;
        // figure out display
        
        // start with 12 hours
        i = now.hour() % 12;
        if(i == 0) i = 12;
        
        if(i <= 6) {
            j = random(0, i);
            k = i - j;
        } else {
            j = random(6 - i, 6);
            k = i - j; 
        }
        
        hours1 = TopLookup[j];
        hours2 = BottomLookup[k];
        
        // now tens of minutes
        i = now.minute() / 10;
        
        if(i <= 6) {
            j = random(0, i);
            k = i - j;
        } else {
            j = random(6 - i, 6);
            k = i - j; 
        }
        
        tenmin1 = TopLookup[j];
        tenmin2 = BottomLookup[k];

        // now ones of minutes
        i = now.minute() % 10;
        
        if(i <= 6) {
            j = random(0, i);
            k = i - j;
        } else {
            j = random(6 - i, 6);
            k = i - j; 
        }
        
        min1 = TopLookup[j];
        min2 = BottomLookup[k];
        
        // output display
        FastShiftOut(min2);
        FastShiftOut(min1);
        FastShiftOut(tenmin2);
        FastShiftOut(tenmin1);
        FastShiftOut(hours2);
        FastShiftOut(hours1);
    }
}

void FastShiftOut(uint8_t shiftVal) {
    uint8_t i;
    
    cbi(ClockPinPort, ClockPinBit);
    
    for(i=0; i<8; i++) {
        if(shiftVal & (1 << i)) {
            sbi(DataPinPort, DataPinBit);
        } else {
            cbi(DataPinPort, DataPinBit);
        }
        sbi(ClockPinPort, ClockPinBit);
        asm("nop\n nop\n nop\n");
        cbi(ClockPinPort, ClockPinBit);
        asm("nop\n nop\n nop\n");
    }
}

