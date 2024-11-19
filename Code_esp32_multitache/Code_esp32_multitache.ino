#include <max6675.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>


#define GPIO19 19
#define GPIO18 18

static QueueHandle_t xQueue;
SemaphoreHandle_t mutex;

TaskHandle_t taskBlinkHandle;
TaskHandle_t taskBlinkHandle1;
TaskHandle_t taskTEMPHandle;
TaskHandle_t taskLCDHandle;


int thermoDO = 12;
int thermoCS = 13;
int thermoCLK = 14;

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup() {
  Serial.begin(115200);
  lcd.init();                      // initialize the lcd 
  lcd.init();
  lcd.backlight();
  lcd.setCursor(3,0);
  lcd.print("Hello, world!");

  xQueue = xQueueCreate(1, sizeof(float));
  mutex = xSemaphoreCreateMutex();
  /* Task creation*/
  xTaskCreatePinnedToCore(TaskBlink,         // Task function
                          "Blink",           // Task name
                          2048,              // Stack size
                          NULL,              //no parameter
                          1,                 // Priority
                          &taskBlinkHandle,  // Task handler
                          1);                //CPU affinity

  xTaskCreatePinnedToCore(TaskBlink1,         // Task function
                          "Blink1",           // Task name
                          2048,              // Stack size
                          NULL,              //no parameter
                          1,                 // Priority
                          &taskBlinkHandle1,  // Task handler
                          0);                //CPU affinity

  xTaskCreatePinnedToCore(TaskTEMP,         // Task function
                          "TEMP",           // Task name
                          2048,              // Stack size
                          NULL,              //no parameter
                          3,                 // Priority
                          &taskTEMPHandle,  // Task handler
                          1);                //CPU affinity

  xTaskCreatePinnedToCore(TaskLCD,         // Task function
                          "LCD",           // Task name
                          2048,              // Stack size
                          NULL,              //no parameter
                          1,                 // Priority
                          &taskLCDHandle,  // Task handler
                          1);                //CPU affinity
}

void TaskLCD(void *pvParameters) {
  for (;;) {
    float temper;
    xQueueReceive(xQueue, &temper, portMAX_DELAY);
    Serial.print("C = "); 
    Serial.println(temper);
    lcd.setCursor(3,1); lcd.print("     ");
    lcd.setCursor(3,1); lcd.print(temper);
    vTaskDelay(10 / portTICK_PERIOD_MS); 
  }
}

void TaskBlink(void *pvParameters) {
  pinMode(GPIO19, OUTPUT);  //led to pin 19
  for (;;) {
    digitalWrite(GPIO19, HIGH);            //set pin19
    vTaskDelay(25 / portTICK_PERIOD_MS);  //wait for 250 ms
    digitalWrite(GPIO19, LOW);             // reset pin19
    vTaskDelay(25 / portTICK_PERIOD_MS);  //wait for 250 ms
  }
}

void TaskBlink1(void *pvParameters) {
  pinMode(GPIO18, OUTPUT);  //led to pin 19
  for (;;) {
    digitalWrite(GPIO18, HIGH);            //set pin19
    vTaskDelay(500 / portTICK_PERIOD_MS);  //wait for 250 ms
    digitalWrite(GPIO18, LOW);             // reset pin19
    vTaskDelay(500 / portTICK_PERIOD_MS);  //wait for 250 ms
  }
}


void TaskTEMP(void *pvParameters) {
  for (;;) {

   xSemaphoreTake(mutex, portMAX_DELAY);
   float temp = thermocouple.readCelsius();
   xQueueSendToBack(xQueue, &temp, portMAX_DELAY);
   xSemaphoreGive(mutex);
   vTaskDelay(1000 / portTICK_PERIOD_MS); 
   //Serial.print("octets : ");
   //Serial.println(uxTaskGetStackHighWaterMark2(NULL));
   
  }
}

void loop() {}

