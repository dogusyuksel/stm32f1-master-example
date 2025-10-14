// file: csp_client.c
#include "can.h"
#include "csp/csp.h"
//#include "csp/interfaces/can.h"
#include "csp/drivers/can_socketcan.h"
//#include "cmsis_os.h"
#include "main.h"
#include "stm32f1xx_hal.h"
#include <string.h>


csp_iface_t csp_can_iface;
csp_conf_t csp_conf;

// Basit HAL transmit fonksiyonu
static int halcan_tx(void *driver_data, uint32_t id, const uint8_t *data, uint8_t dlc) {
    CAN_TxHeaderTypeDef txHeader = {0};
    uint32_t txMailbox;
    txHeader.StdId = id;
    txHeader.IDE = CAN_ID_STD;
    txHeader.RTR = CAN_RTR_DATA;
    txHeader.DLC = dlc;
    if (HAL_CAN_AddTxMessage(&hcan, &txHeader, (uint8_t *)data, &txMailbox) != HAL_OK) {
        return CSP_ERR_TX;
    }
    return CSP_ERR_NONE;
}

// Basit HAL receive fonksiyonu (örnek)
static int halcan_rx(void *driver_data, uint32_t *id, uint8_t *data, uint8_t *dlc, uint32_t timeout) {
    CAN_RxHeaderTypeDef rxHeader;
    if (HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &rxHeader, data) != HAL_OK) {
        return CSP_ERR_RX;
    }
    *id = rxHeader.StdId;
    *dlc = rxHeader.DLC;
    return CSP_ERR_NONE;
}

void csp2_can_init(void) {
    // CAN driver
    static csp_can_driver_t can_driver = {
        .tx = halcan_tx,
        .rx = halcan_rx,
    };

    // CSP konfigurasyonu
    csp_conf_get_defaults(&csp_conf);
    csp_conf.address = 1;
    csp_init(&csp_conf);

    // Interface tanımla
    csp_can_init(&csp_can_iface, &can_driver, 1000000, true);
    csp_iflist_add(&csp_can_iface);

    // Default route
    csp_route_set(CSP_DEFAULT_ROUTE, &csp_can_iface, CSP_NODE_MAC);
}

void vCspTask(void *argument) {
    csp_socket_t *sock = csp_socket(CSP_SO_NONE);
    csp_conn_t *conn;

    while (1) {
        conn = csp_connect(CSP_PRIO_NORM, 2, 10, 1000, CSP_O_NONE);
        if (conn) {
            const char *msg = "Hello from STM32!";
            csp_send(conn, msg, strlen(msg), 0);
            csp_close(conn);
        }
        vTaskDelay(1000);
    }
}

// ----------- Task Create Function ----------- //
void csp_start_tasks(void) {
    xTaskCreate(vCspTask, "CSP Task", 256, NULL, tskIDLE_PRIORITY + 1, NULL);
}
