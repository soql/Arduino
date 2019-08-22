#include "GPS.h"
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  GPS_CallBack();
}
void setup() {
   GPS_Init();

}

void loop() {
  GPS_Process();
}
