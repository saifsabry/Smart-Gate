#ifndef API_H
#define API_H
#include <WiFi.h>
#include <HTTPClient.h>

#include "SoftwareSerial.h"

/************************************** RFID Library *********************************************/
#include <SPI.h>
#include <MFRC522.h>
/************************************** LCD Library *********************************************/
// #include <Wire.h>
// #include <LiquidCrystal_I2C.h>
/************************************** Servo Library *********************************************/
// #include <ESP32Servo.h>
/********************** Functions Prototype ***************************************************/
void connectWiFi();
// void printat(String data ,int positoion,int line);

void LimitSwitch1Interrupt() ;
void LimitSwitch2Interrupt() ;
void BuzzerFail();
void BuzzerSuccess();

void splitString(String input, char delimiter, String parts[], int numParts) ;
String Post_Data(String camid,String camstatus,String userid,String userstatus,int postnumber);
void SolenoidState(EN_SOLENOID state);
void Do_Action();
int SelectMode(void);
String CamSelect();
void BuzzerError();
void Servo_Position(int position);

#endif
