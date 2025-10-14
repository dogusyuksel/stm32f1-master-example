#ifdef USE_LIBCANARD
#include "canard.h"
#include "canard_stm32.h"
#include "main.h"
#include "stm32f1xx_hal.h"

#define LOCAL_NODE_ID 10

extern UART_HandleTypeDef huart3;

void uavcanInit(void);
void sendCanard(void);
void receiveCanard(void);
void spinCanard(void);

#endif // USE_LIBCANARD
