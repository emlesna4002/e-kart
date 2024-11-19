#include <Wire.h> 
#include <LiquidCrystal_I2C.h>


static QueueHandle_t SpeedQueue;
static QueueHandle_t BrakeQueue;

SemaphoreHandle_t mutex1;
TaskHandle_t task1Handle;
TaskHandle_t task2Handle;

LiquidCrystal_I2C lcd(0x27,16,2);

void setup() {
  Serial.begin(115200);
  lcd.init(); lcd.init();
  lcd.backlight(); lcd.setCursor(3,0);
  lcd.print("Hello, world!");

  /*------------creation process-----------*/
  /*notes priorité 0 = plus important //  */
  xTaskCreatePinnedToCore(TaskSpeed,         // Task function
                          "vitesse",           // Task name
                          2048,              // Stack size /// 2048 = 2Ko
                          NULL,              //no parameter
                          1,                 // Priority
                          &task1Handle,  // Task handler
                          1);                //CPU affinity

  xTaskCreatePinnedToCore(TaskBrake,         // Task function
                          "freinage",           // Task name
                          2048,              // Stack size
                          NULL,              //no parameter
                          0,                 // Priorité haute
                          &task2Handle,  // Task handler
                          1);                //CPU affinity
  /*------------creation process end-----------*/
  /*------------creation Queue / mux-----------*/

  SpeedQueue = xQueueCreate(1, sizeof(uint8_t));// queue partage valeur d'accéleration max 254
  BrakeQueue = xQueueCreate(1, sizeof(bool));// queue partage valeur de freinage (flag)
  mutex1 = xSemaphoreCreateMutex();

  /*------------creation Queue / mutex end-----------*/
}

/*aquisition de la valeur vitesse et écriture*/
void TaskSpeed(void *pvParameters) {
  for (;;) {
   //code lecture pédale
   bool brake;
   xQueueReceive(BrakeQueue, &brake, portMAX_DELAY);
   if(brake == 1){// Freinage
    //on desactive l'acceleration
   }
   /////////////////////////////////
   Serial.print("Fonction Speed --- Nbr octets : ");
   Serial.println(uxTaskGetStackHighWaterMark2(NULL));
   vTaskDelay(1000 / portTICK_PERIOD_MS); 
  }
}

/*aquisition du capteur de frein et controle du drapeau active/desactive acceleration*/
void TaskBrake(void *pvParameters) {
  for (;;) {
  ////Code lecture capteur

   xSemaphoreTake(mutex1, portMAX_DELAY);
   bool brake = 0;//////
   xQueueSendToBack(BrakeQueue, &brake, portMAX_DELAY);
   xSemaphoreGive(mutex1);
   
   ///////////
   Serial.print("Fonction Brake --- Nbr octets : ");
   Serial.println(uxTaskGetStackHighWaterMark2(NULL));
   vTaskDelay(1000 / portTICK_PERIOD_MS); 
  }
}

void loop() {

}
