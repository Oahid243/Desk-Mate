//Desk Mate
//Version 1.0 21/7/18
//Designed and Developed by Md. Ferdouse Oahid (Tanin), All Rights Reserved!

#include <LiquidCrystal.h>
#include<SoftwareSerial.h>
#include <Wire.h>
#include "RTClib.h"

//Outputs
const int flushLamp=8, buzzer = 12, ACPlug = 13, Lm35 = 0, alarmCtrl = 1;
const int rs = 2, en = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7; //Display

//Objects
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
SoftwareSerial BT(10,11);
RTC_DS1307 rtc;


//Veriables
char c;
String temp, temperature;
int tempReading, tempHour;
int menuDuration=200000;
char daysOfTheWeek[7][3] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};

//Data
String BTReadData, data;

//Helpers
int btSts = 0, alarmSts = 0, gagtSts = 0, menuSts = 0, timerRunning = 0;
int timerHr=0, timerMin=0, hrMinSelector=0;
int btn1, btn2;
int secPast = 0, minPast = 0;
int btLoadingTime = 0;

//Special Characters
byte degCel[8] = {
  0b11100,
  0b10100,
  0b11100,
  0b00111,
  0b01000,
  0b01000,
  0b01000,
  0b00111
};

//Setup
void setup () {

    lcd.begin(20, 4);
    lcd.createChar(0, degCel);
    Serial.begin(9600);
    BT.begin(9600);
    
    if (! rtc.begin()) {
      lcd.print("Couldn't find RTC");
      while (1);
    }

    if (! rtc.isrunning()) {
      lcd.print("RTC is NOT running!");
      while (1);
      //rrtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    else{
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      //set custom time
      //rtc.adjust(DateTime(2018, 7, 27, 22, 57, 0));
    }
}

//Loop
void loop () {

    DateTime now = rtc.now();
    //Reading Data from Bluetooth
    if(BT.available()){
      btSts=1;
      btLoadingTime=now.second();
      //BTReceiver();
    }

    //On board buttons
    btn1 = analogRead(1);
    btn2 = analogRead(3);
    if(btn1>1000){
      if(alarmSts==1){
        alarmSts=0;
      }
      else if(gagtSts==1){
        gagtSts=0;
      }
    }

    //Formate received data
    if(btSts==1 && ((btLoadingTime+3)==now.second() || (btLoadingTime-57)==now.second())){
      dataFormater();
    }
    if(btn2>609&&btn2<620){
      delay(500);
      if(menuSts==0){
        menuSts=1;
        menuMaker();
      }
      else if(menuSts==1){
        menuSts=2;
        menuMaker();
      }
      else{
        menuSts=0;
        menuMaker();
      }
    }
    timeRow();
}

//Functions
void menuMaker(){
    lcd.setCursor(0,1);
    if(menuSts==1){
      lcd.print("Timer:");
    }
    else if(menuSts==2){
      lcd.print("AC Gadget Timer:");
    }
    else if(menuSts==0){
      lcd.clear();
      return;
    }
    while(menuDuration>0){
      timeRow();
      lcd.setCursor(7,2);
      lcd.print(strFormater(timerHr)+":"+strFormater(timerMin));
      btn2=analogRead(3);
      
      //Menu button pressed
      if(btn2>609&&btn2<620){
        delay(250);
        menuDuration=200000;
        if(menuSts==1){
          menuSts=2;
          return menuMaker();
        }
        else if(menuSts==2){
          menuSts=0;
          return menuMaker();
        }
      }
      
      //Up button pressed
      else if(btn2>704&&btn2<715){
        delay(250);
        menuDuration=200000;
        if(hrMinSelector==0&&timerRunning!=1){
          timerHr++;
        }
        else if(hrMinSelector==1&&timerRunning!=1){
          timerMin++;
        }
      }
      
      //Down button pressed
      else if(btn2>833&&btn2<844){
        delay(250);
        menuDuration=200000;
        if(hrMinSelector==0&&timerRunning!=1){
          timerHr--;
        }
        else if(hrMinSelector==1&&timerRunning!=1){
          timerMin--;
        }
      }
      
      //Go button pressed
      else if(btn2>1000){
          delay(250);
          if(hrMinSelector==0){
            hrMinSelector=1;
          }
          else if(hrMinSelector==1){
            menuSts=0;
            return menuMaker();
          }
          menuDuration=200000;
      }
      menuDuration--;
    }
    menuSts=0;
    hrMinSelector=0;
    lcd.clear();
}

void BTReceiver(){
  //Reading Data from Bluetooth
    while(BT.available()){
      delay(5);
      c = BT.read();
      BTReadData += c;
    }
    //Synchronizing Data
    if(BTReadData.length()>0){
      data += BTReadData;
      Serial.println(data);
      BTReadData = "";
    }
}

int repTaskIndex, indvTaskIndex, gagtTaskIndex;
String curntRoutineTime, curntRoutine, curntTasktime, curntTask, nextTask, upcomming;
int nextRoutineTime, nextTaskTime;
int starCount=0, routineCount=0, repTaskCount=0, indvTaskCount=0, gagtTaskCount=0;
int routineNow=1, repTaskNow=1, indvTaskNow=1, gagtTaskNow=1;

void dataFormater(){
    char c;
    int strLen = data.length();
    int i=0;
    while(i<strLen){
      if(data[i]=='*'){
        starCount++;
        if(starCount==1){
          repTaskIndex=i+1;
        }
        else if(starCount==2){
          indvTaskIndex=i+1;
        }
        else if(starCount==3){
          gagtTaskIndex=i+1;
        }
      }
      if(data[i]=='#'){
        if(starCount==0){
          routineCount++;
        }
        else if(starCount==1){
          repTaskCount++;
        }
        else if(starCount==2){
          indvTaskCount++;
        }
        else if(starCount==3){
          gagtTaskCount++;
        }
      }
      i++;
    }
}

void routineSetter(){
    int i=0;
    int itemCount=0;
    while(data[i]!='*'){
      if(data[i]=='#'){
        itemCount++;
        if(itemCount==routineNow){
          i++;
          while(data[i]!=':'){
            curntRoutineTime+=data[i];
            i++;
          }
          while(data[i]!='\n'){
            curntRoutine+=data[i];
          }
          
        }
      }
      i++;
    }
    
}

void repTaskSetter(){
    
}

void indvTaskSetter(){
    
}

void gagtTaskSetter(){
    
}

void timeRow(){

    DateTime now = rtc.now();
    //Date
    if(now.second()!=secPast){
      secPast=now.second();
      lcd.setCursor(0,0);
      lcd.print(strFormater(now.day()));
      lcd.print('/');
      lcd.print(strFormater(now.month()));
      //Day
      lcd.setCursor(5,0);
      lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
      //Time
      lcd.setCursor(8,0);
      tempHour = timeFormater(now.hour());
      lcd.print(strFormater(tempHour));
      lcd.print(':');
      lcd.print(strFormater(now.minute()));
      lcd.print(':');
      lcd.print(strFormater(now.second()));
    }
    if(now.minute()!=minPast){
      minPast=now.minute();
      //Reading Temperature
      tempReading = analogRead(Lm35)* 0.48828125;
      lcd.setCursor(17,0);
      lcd.print(tempReading);
      lcd.setCursor(19,0);
      lcd.write(byte(0));
    }
}

//To cancel single character digits
String strFormater(int i){
    if(i<0){
      return "00";
    }
    else if(i<10){
      temp = (String) i;
      return "0"+temp;
    }
    return (String) i;
}

//Roman time conversion
int timeFormater(int i){
    if(i>12){
      return i-12;
    }
    return i;
}
