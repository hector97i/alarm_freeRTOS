#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <ArduinoJson.h>
#include <Keypad.h>
#include "queue.h"

#define S1 3
#define S2 4
#define S3 5
#define S4 A5
#define BUZZ 6

const byte rowsCount = 4;
const byte columsCount = 4;

char keys[rowsCount][columsCount] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

const byte rowPins[rowsCount] = {13, 12, 11, 10};
const byte columnPins[columsCount] = {9, 8, 7, A0};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, columnPins, rowsCount, columsCount);
char password[5] = "0000";

DynamicJsonDocument data(64);

QueueHandle_t xPrintQueue, xBuzzerQueue;
TaskHandle_t vSetPwdHandle, vSetSensorsHandle;

typedef bool TaskProfiler;
TaskProfiler Sensor1Profiler, Sensor2Profiler, Sensor3Profiler, Sensor4Profiler, ArmedProfiler, DelayFlagProfiler;

char *readKeypad()
{
  char password[5];
  int i = 0;
  while (i < 4)
  {
    char key = keypad.getKey();
    if (key)
    {
      password[i] = key;
      i++;
    }
  }

  return password;
}

void setup()
{
  // put your setup code here, to run once:

  data["sensor"] = "";
  data["state"] = false;

  ArmedProfiler = false;
  Serial.begin(9600);
  pinMode(S1, INPUT);
  pinMode(S2, INPUT);
  pinMode(S3, INPUT);
  pinMode(S4, INPUT);
  pinMode(BUZZ, OUTPUT);
  xPrintQueue = xQueueCreate(10, sizeof(char *));
  xBuzzerQueue = xQueueCreate(5, sizeof(int));

  xTaskCreate(vHandlerTaskSetPwd, "KEYPAD HANDLER TASK", 100, NULL, 3, &vSetPwdHandle);

  xTaskCreate(vTaskReadSensors, "SENSORs READING TASK", 300, NULL, 4, &vSetSensorsHandle);

  xTaskCreate(vBuzzerGatekeeper, "Buzzer GateKeeper", 100, NULL, 1, NULL);

}




void vBuzzerGatekeeper(void *pvParameters)
{
  int f;
  const TickType_t xTicksToWait = pdMS_TO_TICKS(50);

  while(1)
  {

    xQueueReceive(xBuzzerQueue, &f, xTicksToWait);
    analogWrite(BUZZ, f);
  }
}

void vTaskReadSensors(void *pvParameters)
{

  String s = "";
  char state[10];
  const TickType_t xTicksToWait = pdMS_TO_TICKS(10);
  
  while(1)
  {
    Sensor1Profiler = !digitalRead(S1);
    Sensor2Profiler = !digitalRead(S2);
    Sensor3Profiler = !digitalRead(S3);
    Sensor4Profiler = !digitalRead(S4);

    if(ArmedProfiler){
        data["sensor"] = "1";
        data["state"] = !Sensor1Profiler;
        serializeJson(data, Serial);
        Serial.println();
    
        data["sensor"] = "2";
        data["state"] = !Sensor2Profiler;
        serializeJson(data, Serial);
        Serial.println();
    
        data["sensor"] = "3";
        data["state"] = !Sensor3Profiler;
        serializeJson(data, Serial);
        Serial.println();
    
        data["sensor"] = "4";
        data["state"] = !Sensor4Profiler;
        serializeJson(data, Serial);
        Serial.println();
      if( !(Sensor1Profiler && Sensor2Profiler && Sensor3Profiler && Sensor4Profiler) ){
        if(DelayFlagProfiler){
          
          // Serial.println("BUZZ ON");
          int vol = 127;
          // xQueueSend(xBuzzerQueue, &vol, xTicksToWait);
          analogWrite(BUZZ, vol);
        }
      }
    }

    //taskENTER_CRITICAL();
    /*
    data["sensor"] = "1";
    data["state"] = !Sensor1Profiler;
    serializeJson(data, Serial);
    Serial.println();

    data["sensor"] = "2";
    data["state"] = !Sensor2Profiler;
    serializeJson(data, Serial);
    Serial.println();

    data["sensor"] = "3";
    data["state"] = !Sensor3Profiler;
    serializeJson(data, Serial);
    Serial.println();

    data["sensor"] = "4";
    data["state"] = !Sensor4Profiler;
    serializeJson(data, Serial);
    Serial.println();
    */
    //taskEXIT_CRITICAL();

    // Serial.println("S1, " + String(Sensor1Profiler));
    // Serial.println("S2, " + String(Sensor2Profiler));
    // Serial.println("S3, " + String(Sensor3Profiler));
    // Serial.println("S4, " + String(Sensor4Profiler));

    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}


void vHandlerTaskSetPwd(void *pvParameters)
{

  while (1)
  {
    const TickType_t xTicksToWait = pdMS_TO_TICKS(10);
    if (ArmedProfiler)
    {
      if (!strncmp(password,readKeypad() , 4)){
        int vol = 0;
        // Serial.println("BUZZ OFF");
        // xQueueSend(xBuzzerQueue, &vol, xTicksToWait);
        analogWrite(BUZZ, vol);
        ArmedProfiler = false;
      }
    }
    else
    {
      memcpy(password, readKeypad(), sizeof(password) - sizeof(char));
      // Serial.println("ARMED");
      ArmedProfiler = true;
      DelayFlagProfiler = false;
      vTaskSuspend(vSetSensorsHandle);
      vTaskDelay(5000 / portTICK_PERIOD_MS);
      vTaskResume(vSetSensorsHandle);
      DelayFlagProfiler = true;
    }
    // }

    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void loop() {}
