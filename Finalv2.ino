 #include "Config.h"
#include "API.h"

/*********************** Define QR UART pins***************************************************/
SoftwareSerial qrSerial(32, 35); // RX, TX
/*********************** Define RFID object***************************************************/
MFRC522 rfid(SS_PIN, RST_PIN);
String RFIDCode="";
/**************************************Initiazlizingg LCD *********************************************/
EspSoftwareSerial::UART swSer1;
/**************************************Initiazlizingg Servo variables *********************************************/
// Servo myservo;  // create servo object to control a servo
int pos = 0;    // variable to store the servo position
// Recommended PWM GPIO pins on the ESP32 include 2,4,12-19,21-23,25-27,32-33 
int servoPin = 33;

String QR = "";
String CAM_ID;
HTTPClient http;
String parts[3]={"","",""};
int GetImageStatus=0;2
unsigned long previousmillies=0;
unsigned long seconds=0;
EN_SOLENOID en_state=ERROR;
int length=0;
int LastPostion=0;

volatile int switchState1 = HIGH; // Initial state assuming the pull-up resistor
volatile int switchState2 = HIGH; // Initial state assuming the pull-up resistor
int MyMode=0;

/************************  Limit Switch Variables *********************************************/
 bool fallingEdgeDetected1 = false;
 bool risingEdgeDetected1 = false;

 bool fallingEdgeDetected2 = false;
 bool risingEdgeDetected2 = false;

unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;
// const unsigned long debounceDelay = 150; // Adjust this value as needed

void setup()
{

  /********************* Initialize Serial port **************************************/
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);  // Start Serial2 with RX2 on GPIO16 and TX2 on GPIO17
	swSer1.begin(9600, EspSoftwareSerial::SWSERIAL_8N1, 21, 22, false, 256);
  swSer1.enableIntTx(false);
  /********************** Pin Configuration ******************************************/
  pinMode(Solenoid_1,OUTPUT);
  pinMode(Solenoid_2,OUTPUT);
  pinMode(Buzzer,OUTPUT);
  pinMode(LimitSwitch1,INPUT_PULLUP);
  pinMode(LimitSwitch2,INPUT_PULLUP);
  pinMode(CAM_SC1,INPUT_PULLUP);
  pinMode(CAM_SC2,INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(LimitSwitch1), LimitSwitch1Interrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(LimitSwitch2), LimitSwitch2Interrupt, CHANGE);
  /********************* Initialize QRSerial port ************************************/
  qrSerial.begin(9600);  
  
  /********************* Initialize RFID *********************************************/
  SPI.begin(); // init SPI bus
  rfid.PCD_Init(); // init MFRC522
  /********************* Wifi Init **************************************************/
  connectWiFi();

  CAM_ID=CamSelect();

  MyMode=SelectMode();
  if (MyMode==MODE_ERROR)
  {
    Serial.println("HTTP ERROR \n Please Check Serve \n      Then Reset ");
    MyMode=RFID_CAM_QR;
  }
  else if (MyMode>MODE_ERROR)
  {
    Serial.println("ERROR MODE \n Please Select Valid Mode Then Reset ");
    MyMode=RFID_CAM_QR;
  }
  else 
      Serial.println("Selected mode :"+String(MyMode) );
  /*********** GETTING MODE ********************************************************/
  // printLCD(String(MyMode));
    printLCD("Present Your Card Please");

}

void loop()
{
  if(WiFi.status() != WL_CONNECTED) 
  {
    connectWiFi();
  }
  /************************ Read data from RFID*************************************/

  if (rfid.PICC_IsNewCardPresent())
  { // new tag is available
    if (rfid.PICC_ReadCardSerial())
    { // NUID has been readed
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      Serial.print("RFID/NFC Tag Type: ");
      Serial.println(rfid.PICC_GetTypeName(piccType));

      // print UID in Serial Monitor in the hex format
      Serial.print("UID:");
      for (int i = 0; i < rfid.uid.size; i++) 
      {
        Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(rfid.uid.uidByte[i], HEX);
        RFIDCode+=rfid.uid.uidByte[i];
      }
      // printLCD((String)RFIDCode);
      Serial.println();
      Serial.println("RFIDCode="+RFIDCode);
      rfid.PICC_HaltA(); // halt PICC
      rfid.PCD_StopCrypto1(); // stop encryption on PCD
      Serial.println("Card Scanned Successfully");
    }
    /************************************** Creating HTTP GET Request from "read_data.php"*************************************************/
    String url =  String(serverAddress) +String( ReadDataScript)+String(RFIDCode);
    Serial.println(url);
    http.begin(url);
    int httpCode = http.GET();
    Serial.println(httpCode);
    if (httpCode > 0)
    {  
      String payload = http.getString().c_str();// payload carries either Data , NORECORDS"NO Records Found FOR RFID", "ERROR1" error excuting querry  or "ERROR2" Which means data didn't posted successfully
      Serial.println( payload.length());
      Serial.print("Received payload CheckRFID: " );
      Serial.println(payload);
     // Serial.println( payload.equals("NORECORDS"));

      if (!payload.equals("NORECORDS") && !payload.equals("ERROR1")&& !payload.equals("ERROR2"))
      {
        splitString(payload, ',', parts, 3);
        // Print the result
        for (int i = 0; i < 3; i++)
        {
          Serial.print("Part ");
          Serial.print(i + 1);
          Serial.print(": ");
          Serial.println(parts[i]);
        }  
        /********************************** Post Data to make python code use it ********************************************/
        if(MyMode==RFID_CAM||MyMode==RFID_CAM_QR)
        {  // Servo_Position(parts[2].toInt());
            if (parts[2].toInt()==2) // User is inside 
            {
              GetImageStatus=0;
              Do_Action();
            } 
            else 
            {
              Serial.println("Waiting For Camera ");
              Post_Data(String(CAM_ID),"1",(String)parts[0],"NULL",0);  //ID,Status,userID,Status,post0
              GetImageStatus=1;
            }
        }
        else if (MyMode==RFID ||MyMode==RFID_QR)
        {
          GetImageStatus=0;
          Do_Action();
        }
        else
        {
          Serial.println("Mode Error: "+(String)payload);
          printLCD("Mode Error");
        }
      }
      else
      {
        SolenoidState(TERMINATE);
        BuzzerFail();
        printLCD("Present Your Card Please");

      }
    }
    else
    {
      Serial.println("Error on HTTP request");
      printLCD("Network Error");
      SolenoidState(TERMINATE);
      BuzzerError();
      printLCD("Try Present Your Card again Please");
    }
    http.end();
    /******************************* wait for GET From camera table *********************************/
    //Serial.println("Getting Image Status");
    if (MyMode==RFID_CAM||MyMode==RFID_CAM_QR)
    {
      while (GetImageStatus)
      {
        // Timing condition for the system to aboart if it exceeded 4 seconds
        if(millis()-previousmillies>1000)
        {
          seconds++;
          //Serial.println("seconds="+(String)seconds);
          previousmillies=millis();
          if (seconds==MAX_TIME_DELAY_SECONDS)
          {
            Serial.println("Didn't Acquire You, Rescan Please");
            printLCD("Didn't Acquire You, Rescan Please");
            Post_Data(String(CAM_ID),"0","0","NULL",0);          //ID,Status,userID,Status,post0
            delay(1000);
            Post_Data(String(CAM_ID),"0","0","NULL",1);         //ID,Status,userID,Status,post1
            break;
          }
        }
        String url =  String(serverAddress) + String(CheckESPReply) + "?id="+(String)parts[0];  //Reply.php
        Serial.println(url);
        http.begin(url);
        int httpCode = http.GET();
        Serial.println(httpCode);
        if (httpCode > 0)
        {
          String payload = http.getString();
          Serial.print("Received payload CheckESPReply: " );
          Serial.println(payload);
          if (payload=="TRUE"||payload=="FALSE")
          {
            String Reply=Post_Data(String(CAM_ID),"0","0","NULL",1);  //ID,Status,userID,status,post1
            if (Reply=="OK" && payload=="TRUE")
              Do_Action();
            else if (Reply=="ERROR " || payload=="FALSE")
            {
              BuzzerFail();
              Serial.println(" Error Occured Please Rescan Your Card");
            } 
            Reply="";
            break;
          }
        }
        else
        {
          Serial.println("Error on HTTP request");
        }
        http.end();
      }
    }
    GetImageStatus=0;
    RFIDCode="";
  }
  /************************ Read data from Software Serial and print to Serial Monitor***************************/
  if (MyMode==RFID_QR||MyMode==RFID_CAM_QR)             // Deals With QR Only
  {
    while (qrSerial.available() > 0)
    {
      char receivedChar = qrSerial.read();
      Serial.print(receivedChar);
      QR+=receivedChar;
      length=QR.length();
      Serial.print("Length is : ");
      Serial.println(length);
      
    }
    // Print the received data to Serial Monitor
    if (length>0  &&  length<15)
    {
      Serial.println();
      String x=QR.c_str();
      // printLCD((String)x);
      x.remove(x.length()-1);
      Serial.println(x.length());
      Serial.println("Received Data:" + QR);
      /************************************** Creating HTTP GET Request*************************************************/
      String url =  String(serverAddress) + String(CheckQR) + "?qr="+String(x);
      //+(String)QR;
      Serial.println(url);
      http.begin(url);
      int httpCode = http.GET();
      Serial.println(httpCode);
      if (httpCode > 0)
      {
        String payload = http.getString();
        Serial.print("Received payload CheckQR: " );
        Serial.println(payload);
        if (!payload.equals("NORECORDS") && !payload.equals("ERROR1")&& !payload.equals("ERROR2"))
        {
          splitString(payload, ',', parts, 3);
        
          // Print the result
          for (int i = 0; i < 3; i++)
          {
            Serial.print("Part ");
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.println(parts[i]);
          }
        }
        else
        {
          Serial.println("ERROR on Payload CheckQR Request: "+(String)payload);
          BuzzerFail();
        }
        Do_Action_QR();
      }
      else
      {
        Serial.println("Error on HTTP request");
        BuzzerError();
      }
      http.end();

    }  
    QR = "";
    parts[0]=parts[1]=parts[2]="";
    delay(1000); // Delay for 1 second before making the next request
  }
  length=0;
  seconds=0;
}

void LimitSwitch1Interrupt() 
{
    static int lastState1 = HIGH; // Keep track of the last state of the pin
    int currentState1 = digitalRead(LimitSwitch1); // Read the current state of the pin

    if (lastState1 == HIGH && currentState1 == LOW) {
        fallingEdgeDetected1 = true; // Falling edge detected
      //  Serial.println("Falling edge detected");
    } else if (lastState1 == LOW && currentState1 == HIGH) {
        risingEdgeDetected1 = true; // Rising edge detected
      //  Serial.println("Rising edge detected");
    }
    lastState1 = currentState1; // Update last state

}
void LimitSwitch2Interrupt() 
{
     static int lastState2 = HIGH; // Keep track of the last state of the pin
    int currentState2 = digitalRead(LimitSwitch2); // Read the current state of the pin

    if (lastState2 == HIGH && currentState2 == LOW) {
        fallingEdgeDetected2 = true; // Falling edge detected
      //  Serial.println("Falling edge detected");
    } else if (lastState2 == LOW && currentState2 == HIGH)
     {
        risingEdgeDetected2 = true; // Rising edge detected
    }
    lastState2 = currentState2; // Update last state
}
void Do_Action()
{
    if (parts[2].toInt()==1)   // I want to Enter
    {
      lastDebounceTime1=millis();
      BuzzerSuccess();
      SolenoidState(ENTER);
      Serial.println("Enter");
      while(millis() - lastDebounceTime1 < MAX_WAIT_TIME)
      {
        if (risingEdgeDetected1==1 && fallingEdgeDetected1==1)
        { 
          break;
        }
      }
      Serial.println("After while");
      digitalWrite(Solenoid_1, LOW);
      digitalWrite(Solenoid_2, LOW);
      if (risingEdgeDetected1 &&fallingEdgeDetected1)
      {

          Serial.println("Limit Switch 1 Activated!");
          String Reply=Post_Data("NULL","NULL",parts[0],"2",2);  //ID,Status,userID,status,post2
          Serial.println("GoT Reply : "+ Reply); 
          lastDebounceTime1 = millis(); // Update the debounce time
        risingEdgeDetected1 =fallingEdgeDetected1 = 0; // Reset the flag
      }
      printLCD("Present Your Card Please");
    }
    else if(parts[2].toInt()==2)  //Exit
    {
      // printLCD("Exit");
      lastDebounceTime1=millis();
      BuzzerSuccess2();
      SolenoidState(EXIT);
      Serial.println("Exit");
      while(millis() - lastDebounceTime1 <MAX_WAIT_TIME)
      {
        if (risingEdgeDetected1==1 && fallingEdgeDetected1==1)
        { 
          break;
        }
      }
      digitalWrite(Solenoid_1, LOW);
      digitalWrite(Solenoid_2, LOW);
      if (risingEdgeDetected1 &&fallingEdgeDetected1) 
      {

          Serial.println("Limit Switch 2 Activated!");
          String Reply=Post_Data("NULL","NULL",parts[0],"1",2);  //ID,Status,userID,status,post2
          Serial.println("GoT Reply : "+ Reply); 
          lastDebounceTime1 = millis(); // Update the debounce time
          risingEdgeDetected1=fallingEdgeDetected1=0; // Reset the flag
      }
      printLCD("Present Your Card Please");

    }
    else  //Error
    {
      SolenoidState(TERMINATE);
      BuzzerFail();
    }
    parts[0]=parts[1]=parts[2]=""; 
}



void Do_Action_QR()
{
    if (parts[2].toInt()==1)   // Enter
    {
      lastDebounceTime1=millis();
      BuzzerSuccess();
      SolenoidState(ENTER);
      Serial.println("Enter");
      while(millis() - lastDebounceTime1 < MAX_WAIT_TIME)
      {
        if (risingEdgeDetected1==1 && fallingEdgeDetected1==1)
        { 
          break;
        }
      }
      Serial.println("After while");
      digitalWrite(Solenoid_1, LOW);
      digitalWrite(Solenoid_2, LOW);
      if (risingEdgeDetected1 &&fallingEdgeDetected1)
      {

          Serial.println("Limit Switch 1 Activated!");
          String Reply=Post_Data("NULL","NULL",parts[0],"2",4);  //ID,Status,userID,status,post2
          Serial.println("GoT Reply : "+ Reply); 
          lastDebounceTime1 = millis(); // Update the debounce time
        risingEdgeDetected1 =fallingEdgeDetected1 = 0; // Reset the flag
        // risingEdgeDetected2 =fallingEdgeDetected2 = 0;
      }
      printLCD("Present Your Card Please");
    }
    else if(parts[2].toInt()==2)  //Exit
    {
      // printLCD("Exit");
      lastDebounceTime1=millis();
      BuzzerSuccess2();
      SolenoidState(EXIT);
      Serial.println("Exit");
      while(millis() - lastDebounceTime1 <MAX_WAIT_TIME)
      {
        if (risingEdgeDetected1==1 && fallingEdgeDetected1==1)
        { 
          break;
        }
      }
      digitalWrite(Solenoid_1, LOW);
      digitalWrite(Solenoid_2, LOW);
      if (risingEdgeDetected1 &&fallingEdgeDetected1) 
      {

          Serial.println("Limit Switch 2 Activated!");
          String Reply=Post_Data("NULL","NULL",parts[0],"1",4);  //ID,Status,userID,status,post2
          Serial.println("GoT Reply : "+ Reply); 
          lastDebounceTime1 = millis(); // Update the debounce time
        // risingEdgeDetected2=fallingEdgeDetected2=0; // Reset the flag
        risingEdgeDetected1 =fallingEdgeDetected1 = 0;
      }
      printLCD("Present Your Card Please");

    }
    else  //Error
    {
      SolenoidState(TERMINATE);
      BuzzerFail();
    }
    parts[0]=parts[1]=parts[2]=""; 
}
void SolenoidState(EN_SOLENOID state)
{
    switch (state) 
    {
    case (ERROR):
      printLCD("Status :ERROR");
      digitalWrite(Solenoid_1, LOW);
      digitalWrite(Solenoid_2, LOW);
      Serial.println("Status :error");
      delay(1000);
      break;
    case( ENTER):
      //printLCD("Status :ENTER");
      digitalWrite(Solenoid_1, HIGH);
      digitalWrite(Solenoid_2, LOW);
      Serial.println("Status :opening");
      //delay(MAX_WAIT_TIME);
      break;
    case (EXIT):
      //printLCD("Status :EXIT");
      digitalWrite(Solenoid_1, LOW);
      digitalWrite(Solenoid_2, HIGH);
      Serial.println("closing");
      Serial.println("Status :EXIT");
      //delay(MAX_WAIT_TIME);
      break;
    case (TERMINATE):
    //  printLCD("Terminate");
      digitalWrite(Solenoid_1, LOW);
      digitalWrite(Solenoid_2, LOW);
      break;
    default:
      Serial.println("Unknown state");
      break;
  }
}
String Post_Data(String camid,String camstatus,String userid,String userstatus,int postnumber)
{
  /****************************** HTTP Post**************************************************/
  String postData="";
  String URL="";
  if(postnumber==0)
  {
    postData = "camid=" +String(camid)+"&camstatus="+String(camstatus)+"&userid="+String(userid); 
    URL =  String(serverAddress) + String(UpdateCamera) ;

  }
  else if (postnumber==1)
  {
    postData = "camid=" +String(camid)+"&ESPReply="+String(userstatus)+"&userid="+String(userid);
    URL =  String(serverAddress) + String(UpdateESPReply) ;
  }
  else if (postnumber==2)
  {
    postData ="userstatus="+String(userstatus)+"&userid="+String(userid);
    URL =  String(serverAddress) + String(UpdateStudent) ;

  }
  else if (postnumber==3)
  {
    postData ="status="+String(userstatus)+"&id="+String(userid);
    URL =  String(serverAddress) + String(UpdateQRStatus) ;
  }
    else if (postnumber==4)
  {
    postData ="status="+String(userstatus)+"&id="+String(userid);
    URL =  String(serverAddress) + String(UpdateQRStatus) ;
  }
 // String URL =  String(serverAddress) + String(ImageScript) ;
  http.begin(URL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");  
  int httpCode = http.POST(postData);
  String Response = http.getString(); 
  Serial.print("URL : "); Serial.println(URL); 
  Serial.print("Data: "); Serial.println(postData);
  Serial.print("httpCode: "); Serial.println(httpCode);
  Serial.print("Response for postnumber "+(String )postnumber)+":"; Serial.println(Response);
  Serial.println("--------------------------------------------------");
  delay(2000);//5000
  return Response;
}

void splitString(String input, char delimiter, String parts[], int numParts) 
{
  int partIndex = 0;
  int startIndex = 0;

  for (int i = 0; i < input.length(); i++) {
    if (input.charAt(i) == delimiter) {
      parts[partIndex++] = input.substring(startIndex, i);
      startIndex = i + 1;

      if (partIndex >= numParts) {
        break; // Break if we have enough parts
      }
    }
  }
  // Add the last part if there's any remaining
  if (partIndex < numParts) {
    parts[partIndex] = input.substring(startIndex);
  }
}
void connectWiFi()
{
  WiFi.mode(WIFI_OFF);
  delay(1000);
  //This line hides the viewing of ESP as wifi hotspot
  WiFi.mode(WIFI_STA);
  
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  printLCD("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
    
  Serial.print("connected to : "); Serial.println(ssid);
  printLCD("connected to :"+String(ssid));
  delay(2000);

  Serial.print("IP address: "); Serial.println(WiFi.localIP());
}
void printLCD(const String& str)
{
  char charArray[str.length() + 1];  // Create a char array with size of the string + 1 for the null terminator
  str.toCharArray(charArray, str.length() + 1);  // Convert the string to char array
  Serial2.write(charArray);  // Send the char array over Serial2
  swSer1.write(charArray);  

}
void BuzzerSuccess()
{
  Serial.println("Buzzer HIGH............."+String(ACCESS_ACCEPTED));
  printLCD("0"); 
  for (int i=0 ; i <3; i++)
  {
    digitalWrite(Buzzer,HIGH);
    delay(200);
    digitalWrite(Buzzer,LOW);
    delay(200);
  }
}
void BuzzerSuccess2()
{
  Serial.println("Buzzer HIGH............."+String(ACCESS_ACCEPTED));
  printLCD("0"); 
  for (int i=0 ; i <6; i++)
  {
    digitalWrite(Buzzer,HIGH);
    delay(100);
    digitalWrite(Buzzer,LOW);
    delay(100);
  }
}
void BuzzerFail()
{
    Serial.println("Buzzer LOW................."+String(ACCESS_DENIED));
    printLCD(String(ACCESS_DENIED));
    digitalWrite(Buzzer,HIGH);
    delay(1000);
    digitalWrite(Buzzer,LOW);
}

void BuzzerError()
{
    Serial.println("Buzzer ERROR");
    printLCD("System Error");
    digitalWrite(Buzzer,HIGH);
    delay(1000);
    digitalWrite(Buzzer,LOW);
    delay(1000);
    digitalWrite(Buzzer,HIGH);
    delay(1000);
    digitalWrite(Buzzer,LOW);
    delay(1000);
}
String CamSelect()
{
  int c1=digitalRead(CAM_SC1);
  delay(200);
  int c2=digitalRead(CAM_SC2);
  delay(200);
  if (c1==1 && c2==1)
    return "1";
  else if (c1==0 && c2==1)
    return "2";
  else if (c1==1 && c2==0)
    return "3";
  else if (c1==0 && c2==0)
    return "4";
}
int SelectMode(void)
{
  String url =  String(serverAddress) + String(ModeScript);
  Serial.println(url);
  http.begin(url);
  int httpCode = http.GET();
  Serial.println(httpCode);
  if (httpCode > 0)
  {  
    String payload = http.getString().c_str();// payload carries either Data , NORECORDS"NO Records Found FOR RFID", "ERROR1" error excuting querry  or "ERROR2" Which means data didn't posted successfully
    Serial.println( payload.length());
    Serial.print("Received payload Mode: " );
    Serial.println(payload);
    // Serial.println( payload.equals("NORECORDS"));

    if (payload.equals("1")  )
    {
      return RFID_CAM ;
    }
    else if (payload.equals("2")  )
    {
      return RFID_CAM_QR ;
    }
    else if (payload.equals("3")  )
    {
      return RFID_QR ;
    }
    else if (payload.equals("4")  )
    {
      return RFID ;
    }
    else 
      return MODE_ERROR;
  }
  return HTTP_ERROR;
}
