

#include "host_packet.h"
#include "scheduler.h"

#ifndef HOST_INTERFACE_H_
#define HOST_INTERFACE_H_

#define Firmware_VersionA 0
#define Firmware_VersionB 0
#define Firmware_VersionC 0
#define Firmware_VersionD 7

#define HOST_CMD_READ_ALL_DATA      0x30
#define HOST_CMD_LDCLK_DIV          0x31
#define HOST_CMD_LED_STATES         0x32
#define HOST_CMD_CALIBRATE          0x33
#define HOST_CMD_DEFAULTS_A         0x34
#define HOST_CMD_DEFAULTS_B         0x35
#define HOST_CMD_SAVE_SETTINGS_A    0x3A
#define HOST_CMD_SAVE_SETTINGS_B    0x3B
#define HOST_CMD_CH_READ            0x36

//#define HOST_DEVICE_LDC1612 0x06
//#define HOST_DEVICE_LDC1614 0x07
//#define HOST_DEVICE_LDC1312 0x04
#define HOST_DEVICE_LDC1314 0x05

#ifdef HOST_DEVICE_LDC1612
#define HOST_DEVICE_CURRENT HOST_DEVICE_LDC1612
#define CH_2
#define SERIES_16
#endif
#ifdef HOST_DEVICE_LDC1614
#define HOST_DEVICE_CURRENT HOST_DEVICE_LDC1614
#define CH_2
#define SERIES_16
#endif
#ifdef HOST_DEVICE_LDC1312
#define HOST_DEVICE_CURRENT HOST_DEVICE_LDC1312
#define CH_2
#define SERIES_13
#endif
#ifdef HOST_DEVICE_LDC1314
#define HOST_DEVICE_CURRENT HOST_DEVICE_LDC1314
#define CH_2
#define SERIES_13
#endif

typedef struct
{
	host_cmd_packet_t packet;
	scheduled_task_t task;
} host_task_data_t;

// data lengths must not exceed HOST_MAX_CMD_DATA_LEN
#define HOST_FIRMWARE_VERSION_LENGTH     4
#define HOST_CONTROLLER_TYPE_LENGTH      10

#define HOST_PID_MENU_READ_ALL 0xFF

#define HOST_NUM_DEVICES 1

void initHostInterface();

void processCmdPacket ();

#endif
