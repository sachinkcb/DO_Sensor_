#include <Arduino.h>
#include "CApplication.h"

// #define SERIAL_DEBUG
#ifdef SERIAL_DEBUG
#define debugPrint(...) Serial.print(__VA_ARGS__)
#define debugPrintln(...) Serial.println(__VA_ARGS__)
#define debugPrintf(...) Serial.printf(__VA_ARGS__)
#define debugPrintlnf(...) Serial.println(F(__VA_ARGS__))
#else
#define debugPrint(...)    //blank line
#define debugPrintln(...)  //blank line
#define debugPrintf(...)   //blank line
#define debugPrintlnf(...) //blank line
#endif

TaskHandle_t Task1;
TaskHandle_t Task2;
TaskHandle_t Task3;
TaskHandle_t Task4;

cApplication App;
char taskStatucBuff[50];
Ticker FeedWatchDog;

/*Tickervtask to feed watchdog in safemode*/
void feedWatchDogTask(void)
{  
  App.feedWdtInSafeMode();
}

/************************
 * Task 1
*************************/
void Task1code(void *pvParameters)
{
  for (;;)
  {
    App.frameHandlingTask();
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}

/************************
 * Task2 
*************************/
void Task2code(void *pvParameters)
{
  for (;;)
  {
    App.applicationTask();
    vTaskDelay(100/portTICK_PERIOD_MS);
  }
}

/************************
 * Task3 
*************************/
void Task3code(void *pvParameters)
{
  for (;;)
  {
    App.commandParseTask();
    vTaskDelay(10/portTICK_PERIOD_MS);    
  }
}

/************************
 * Task4 
*************************/
void Task4code(void *pvParameters)
{
  for (;;)
  {
    App.RfidTask();
    vTaskDelay(100/portTICK_PERIOD_MS);    
  }
}


/*******************************
*  Function that create tasks 
********************************/
void CreateTasks(int Value)
{  
  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                  Task1code, /* Task function. */
                  "Frame",   /* name of task. */
                  15000,     /* Stack size of task */
                  NULL,      /* parameter of the task */
                  3,         /* priority of the task */
                  &Task1,    /* Task handle to keep track of created task */
                  CONFIG_ARDUINO_RUNNING_CORE);        /* pin task to core 1 */
  delay(500);

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                Task2code, /* Task function. */
                "App",   /* name of task. */
                10000,     /* Stack size of task */
                NULL,      /* parameter of the task */
                2,         /* priority of the task */
                &Task2,    /* Task handle to keep track of created task */
                0);        /* pin task to core 1 */
  delay(500);

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                Task3code, /* Task function. */
                "command",   /* name of task. */
                10000,     /* Stack size of task */
                NULL,      /* parameter of the task */
                1,         /* priority of the task */
                &Task3,    /* Task handle to keep track of created task */
                0);        /* pin task to core 1 */
  delay(500);
  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                Task4code, /* Task function. */
                "RFID",   /* name of task. */
                10000,     /* Stack size of task */
                NULL,      /* parameter of the task */
                1,         /* priority of the task */
                &Task4,    /* Task handle to keep track of created task */
                0);        /* pin task to core 1 */
  delay(500);
}


/************************
 * Setup initialization
*************************/
void setup()
{
  // put your setup code here, to run once:
  debugPrintln("=================================Code begin=================================");  
  int retValue = App.appInit();
  if(retValue)
  {
    CreateTasks(retValue);
    App.AppWatchdogInit(&Task1,&Task2,&Task3,&Task4);
    debugPrintln("Total 4 Watchdog Init.....");
  }
  else
  {
    /*Ticker task to feed watchdog in safemode*/
    FeedWatchDog.attach(1, feedWatchDogTask); 
  }
}

/************************
 * loop
*************************/
void loop()
{}