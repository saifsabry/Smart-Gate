#ifndef CONFIG_H
#define CONFIG_H
  //#define CAM_ID          1
  /************************* Macros to define Sensor Pns ****************************************/
  #define Solenoid_1              26
  #define Solenoid_2              27
  #define SS_PIN                  4 // ESP32 pin GPIO5 
  #define RST_PIN                 5 // ESP32 pin GPIO27 
  #define Buzzer                  13
  #define LimitSwitch1            12
  
  #define LimitSwitch2            14
  #define CAM_SC1                 15
  #define CAM_SC2                 25 

  #define MAX_TIME_DELAY_SECONDS  8  
  #define MAX_WAIT_TIME           4000

  #define INSIDE    2
  #define OUTSIDE   1

  /*********************** Define wifi variables & server IP***********************************/
  const char* serverAddress = "http://192.168.217.190:5149";//todo 0.0.0.0
  const char* ReadDataScript = "/CheckRFID?rfid=";
  const char* ModeScript = "/GetModeNumber?mode=true";
  const char* UpdateCamera = "/updateCamera";
  const char* UpdateESPReply = "/updateESPReply";
  const char* UpdateStudent = "/updateStudent";
  const char* UpdateQRStatus = "/updateQRStatus";
  const char* CheckESPReply = "/CheckESPReply";
  const char* CheckQR = "/CheckQR";
  // const char* phppostScript = "/connect.php";
  const char* ssid = "El3ezaby"; 
  const char* password = "10000000@#"; 
  /************************* Enums****************************************************************/
  enum EN_SOLENOID
  {
    ERROR=0,
    ENTER=1,
    EXIT=2,
    TERMINATE=3
  };

  enum EN_MODE
  {
    HTTP_ERROR=0,
    RFID_CAM=1,
    RFID_CAM_QR=2,
    RFID_QR=3,
    RFID=4,
    MODE_ERROR=5
  };
  enum EN_ACCESS_MODE
  {
    ACCESS_ACCEPTED=0,
    ACCESS_DENIED=1,
    ACCESS_ACCEPTED_ENTER=2,
    ACCESS_ACCEPTED_EXIT=3
  };
#endif