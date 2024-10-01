// This is needed to enable necessary declarations in sys/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <canard.h>
#include <errno.h>
#include <getopt.h>
#include <socketcan.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static uint8_t LOCAL_NODE_ID = 5;
static char INTERFACE_NAME[128] = "can0";
static char BCASTDATA[5] = "";

/*
 * Application constants
 */
#define VERSION "00.00"

/*
 * Some useful constants defined by the UAVCAN specification.
 * Data type signature values can be easily obtained with the script show_data_type_info.py
 */
#define UAVCAN_NODE_ID_ALLOCATION_DATA_TYPE_ID 1
#define UAVCAN_NODE_ID_ALLOCATION_DATA_TYPE_SIGNATURE 0x0b2a812620a11d40
#define UAVCAN_NODE_STATUS_MESSAGE_SIZE 7

/*
 * Library instance.
 * In simple applications it makes sense to make it static, but it is not necessary.
 */
static CanardInstance canard;            ///< The library instance
static uint8_t canard_memory_pool[1024]; ///< Arena for memory allocation, used by the library

static uint64_t getMonotonicTimestampUSec(void) {
    struct timespec ts;
    memset(&ts, 0, sizeof(ts));
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        abort();
    }
    return (uint64_t)(ts.tv_sec * 1000000LL + ts.tv_nsec / 1000LL);
}

static void dump_buffer(const unsigned char *buffer, unsigned int len) {
    unsigned int i = 0, j = 0;
    unsigned int j_max = 0;

    if (!buffer) {
        return;
    }

    for (i = 0; i < len; i = i + 16) {
        j_max = 16;
        if (i + j_max > len) {
            j_max = len - i;
        }
        for (j = 0; j < j_max; j++) {
            fprintf(stderr, "0x%.02X ", buffer[i + j]);
        }
        for (j = 0; j < 16 - j_max; j++) {
            fprintf(stderr, "     ");
        }
        fprintf(stderr, "\t\t");
        for (j = 0; j < j_max; j++) {
            if (buffer[i + j] >= 0x20 && buffer[i + j] <= 0x7e)
                fprintf(stderr, "%c", buffer[i + j]);
        }
        fprintf(stderr, "\n");
    }
}

/**
 * This callback is invoked by the library when a new message or request or response is received.
 */
static void onTransferReceived(CanardInstance *ins, CanardRxTransfer *transfer) {
    (void)ins;

    if (!transfer || !transfer->payload_head) {
        return;
    }

    printf("payload_len: %d\n", transfer->payload_len);
    printf("payload_head:\n");
    dump_buffer(transfer->payload_head, CANARD_MULTIFRAME_RX_PAYLOAD_HEAD_SIZE);
    printf("-----------------------------------------------------------------------------------------------------------"
           "-------------------\n");
}

static char *transfer_type_to_str(CanardTransferType type) {
    switch (type) {
    case CanardTransferTypeResponse:
        return "Response";
    case CanardTransferTypeRequest:
        return "Request";
    case CanardTransferTypeBroadcast:
        return "Broadcast";
        ;
    default:
        fprintf(stderr, "unhandled type: %d\n", (int)type);
        break;
    }

    return "Unknown_Type";
}

/**
 * This callback is invoked by the library when it detects beginning of a new transfer on the bus that can be received
 * by the local node.
 * If the callback returns true, the library will receive the transfer.
 * If the callback returns false, the library will ignore the transfer.
 * All transfers that are addressed to other nodes are always ignored.
 */
static bool shouldAcceptTransfer(const CanardInstance *ins, uint64_t *out_data_type_signature, uint16_t data_type_id,
                                 CanardTransferType transfer_type, uint8_t source_node_id) {
    (void)out_data_type_signature;

    printf("-----------------------------------------------------------------------------------------------------------"
           "-------------------\n");
    printf("from(%d) -> to(%d)(%s_ %d)\n", source_node_id, ins->node_id, transfer_type_to_str(transfer_type),
           data_type_id);

    return true;
}

/**
 * This function is called at 1 Hz rate from the main loop.
 */
static void process1HzTasks(uint64_t timestamp_usec) {
    /*
     * Purging transfers that are no longer transmitted. This will occasionally free up some memory.
     */
    canardCleanupStaleTransfers(&canard, timestamp_usec);

    if (strlen(BCASTDATA)) {
        static uint8_t transfer_id; // Note that the transfer ID variable MUST BE STATIC (or heap-allocated)!
        uint8_t buffer[UAVCAN_NODE_STATUS_MESSAGE_SIZE];

        // fill status data here
        memset(buffer, 0, UAVCAN_NODE_STATUS_MESSAGE_SIZE);
        canardEncodeScalar(buffer, 0, 40, BCASTDATA);

        const int16_t bc_res = canardBroadcast(&canard, UAVCAN_NODE_ID_ALLOCATION_DATA_TYPE_SIGNATURE,
                                               UAVCAN_NODE_ID_ALLOCATION_DATA_TYPE_ID, &transfer_id,
                                               CANARD_TRANSFER_PRIORITY_LOW, buffer, UAVCAN_NODE_STATUS_MESSAGE_SIZE);
        if (bc_res <= 0) {
            fprintf(stderr, "Could not broadcast node status; error %d\n", bc_res);
        }
    }
}

/**
 * Transmits all frames from the TX queue, receives up to one frame.
 */
static void processTxRxOnce(SocketCANInstance *socketcan, int32_t timeout_msec) {
    // Transmitting
    for (const CanardCANFrame *txf = NULL; (txf = canardPeekTxQueue(&canard)) != NULL;) {
        const int16_t tx_res = socketcanTransmit(socketcan, txf, 0);
        if (tx_res < 0) // Failure - drop the frame and report
        {
            canardPopTxQueue(&canard);
            (void)fprintf(stderr, "Transmit error %d, frame dropped, errno '%s'\n", tx_res, strerror(errno));
        } else if (tx_res > 0) // Success - just drop the frame
        {
            canardPopTxQueue(&canard);
        } else // Timeout - just exit and try again later
        {
            break;
        }
    }

    // Receiving
    CanardCANFrame rx_frame;
    const uint64_t timestamp = getMonotonicTimestampUSec();
    const int16_t rx_res = socketcanReceive(socketcan, &rx_frame, timeout_msec);
    if (rx_res < 0) // Failure - report
    {
        (void)fprintf(stderr, "Receive error %d, errno '%s'\n", rx_res, strerror(errno));
    } else if (rx_res > 0) // Success - process the frame
    {
        canardHandleRxFrame(&canard, &rx_frame, timestamp);
    } else {
        ; // Timeout - nothing to do
    }
}

static struct option parameters[] = {
    {"iface", required_argument, 0, 'i'},
    {"nodeid", required_argument, 0, 'n'},
    {"bcast", required_argument, 0, 't'},
    {"help", no_argument, 0, 'h'},
    {NULL, 0, 0, 0},
};

static void show_usage_and_exit(char *appname) {
    if (appname) {
        printf("usage of %s ", appname);
    }
    printf("(version: %s)\n", VERSION);
    printf("\t\"--iface\"/\"-i\"\t:\tused for specifying can interface (eg: can0 - which is default)\n");
    printf("\t\"--nodeid\"/\"-n\"\t:\tused for specifying can id (eg: 5 - which is default)\n");
    printf("\t\"--bcast\"/\"-t\"\t:\tused for specifying broadcasted data (eg: test - no bcast if not specified)\n");
    printf("\t\tmax bcast data len is: %d\n", UAVCAN_NODE_STATUS_MESSAGE_SIZE);

    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
    int c, o;
    char *endptr = NULL;
    uint64_t next_1hz_service_at = 0;
    int16_t res = 0;
    SocketCANInstance socketcan;

    while ((c = getopt_long(argc, argv, "i:n:t:h:", parameters, &o)) != -1) {
        switch (c) {
        case 'i':
            strncpy(INTERFACE_NAME, optarg, sizeof(INTERFACE_NAME));
            break;
        case 'n':
            LOCAL_NODE_ID = (uint8_t)strtoul(optarg, &endptr, 10);
            break;
        case 't':
            strncpy(BCASTDATA, optarg, sizeof(BCASTDATA));
            break;
        case 'h':
        default:
            show_usage_and_exit(argv[0]);
        }
    }

    printf("Configuration: \n");
    printf("\tinterface  : %s\n", INTERFACE_NAME);
    printf("\tnode id    : %d\n", LOCAL_NODE_ID);
    printf("\tbcast data : %s\n\n", BCASTDATA);

    /*
     * Initializing the CAN backend driver; in this example we're using SocketCAN
     */
    res = socketcanInit(&socketcan, INTERFACE_NAME);
    if (res < 0) {
        (void)fprintf(stderr, "Failed to open CAN iface '%s'\n", INTERFACE_NAME);
        return 1;
    }

    /*
     * Initializing the Libcanard instance.
     */
    canardInit(&canard, canard_memory_pool, sizeof(canard_memory_pool), onTransferReceived, shouldAcceptTransfer, NULL);

    if (canardGetLocalNodeID(&canard) == 0) {
        canardSetLocalNodeID(&canard, LOCAL_NODE_ID);
    }

    /*
     * Running the main loop.
     */
    next_1hz_service_at = getMonotonicTimestampUSec();

    for (;;) {
        processTxRxOnce(&socketcan, 10);

        const uint64_t ts = getMonotonicTimestampUSec();

        if (ts >= next_1hz_service_at) {
            next_1hz_service_at += 1000000;
            process1HzTasks(ts);
        }
    }

    return 0;
}
