
//#include <intrinsics.h>
#include <stdint.h>
#include <string.h>
//#include <cstring>
#include <stdio.h>

#include "spi_1p1.h"
#include "crc8.h"

#include "LDC13xx16xx_cmd.h"
#include "LDC13xx16xx_evm.h"
#include "host_interface.h"
#include "host_menu.h"
#include "scheduler.h"

#include "HAL_FLASH.h"

#include "USB_API/USB_Common/device.h"
#include "USB_API/USB_Common/types.h"               //Basic Type declarations
#include "USB_API/USB_Common/usb.h"                 //USB-specific functions

#include "USB_API/USB_CDC_API/UsbCdc.h"
#include "usbConstructs.h"
#include "USB_config/descriptors.h"

#define HOST_INTERFACE_MAX_STREAM_TASKS 5

#define HOST_STREAM_TICKS_DEF   100000UL // 1sec
#define HOST_STREAM_TICKS_MIN   1000 // 10ms
#define HOST_STREAM_TICKS_MAX   3000000UL // 30 sec
#define HOST_STREAM_TICKS_INC   1000 // 10ms

#define INFOC_START   (0x1880)
// flash memory
#pragma DATA_SECTION(FlashSerial, ".infoC")
uint8_t  FlashSerial[HOST_SERIAL_LENGTH];

static uint8_t serial[HOST_SERIAL_LENGTH];

//static uint8_t buffer[HOST_LEGACY_PACKET_LEN];

// INTERNAL
// this shared secret is randomly generated, do not change
// ^q,}pt#>(bv5#3#xs(r4yfo(a<s(cn
static const uint8_t sharedSecret[30] = {
		'^','q',',','}','p','t','#','>',
		'(','b','v','5','#','3','#','x',
		's','(','r','4','y','f','o','(',
		'a','<','s','(','c','n'
};

#ifdef HOST_DEVICE_LDC1612
const uint8_t HOST_CONTROLLER_TYPE[] = {'L','D','C','1','6','1','2','E','V','M','\0'};
#endif
#ifdef HOST_DEVICE_LDC1312
const uint8_t HOST_CONTROLLER_TYPE[] = {'L','D','C','1','3','1','2','E','V','M','\0'};
#endif
#ifdef HOST_DEVICE_LDC1614
const uint8_t HOST_CONTROLLER_TYPE[] = {'L','D','C','1','6','1','4','E','V','M','\0'};
#endif
#ifdef HOST_DEVICE_LDC1314
const uint8_t HOST_CONTROLLER_TYPE[] = {'L','D','C','1','3','1','4','E','V','M','\0'};
#endif

const uint8_t HOST_FIRMWARE_VERSION[] = {Firmware_VersionA,Firmware_VersionB,Firmware_VersionC,Firmware_VersionD};
const uint8_t HOST_CMD_STREAM_WHITELIST[] = {
		HOST_CMD_CONTROLLER_TYPE,
		HOST_CMD_FIRMWARE_VERSION,
		HOST_CMD_ENUMERATE_DEVICES,
		HOST_CMD_SPI_READ_BYTES,
		HOST_CMD_READ_ALL_DATA,
		HOST_CMD_LED_STATES
};

// for USB
extern uint8_t bCDCDataReceived_event;

// for timing (scheduler)
extern volatile uint16_t timer_ticks;

// host device variables
static uint8_t host_devices[HOST_NUM_DEVICES + 1] = { HOST_NUM_DEVICES, HOST_DEVICE_CURRENT };
static uint8_t default_addr;
//static uint8_t default_dev = 1;

// menu prototypes
static void menuAllData();
static void menuStartStream();
static void menuStopStream();
static void menuDecreasePeriod();
static void menuIncreasePeriod();
static void menuCycleLEDS();
static void menuCycleI2CAddr();

// string menu variables
static host_menu_item_t menu_items[] = {
		{'1',"all data",&menuAllData,NULL},
		{'2',"start stream",&menuStartStream,NULL},
		{'3',"stop stream",&menuStopStream,NULL},
		{'4',"stream period-",&menuDecreasePeriod,NULL},
		{'5',"stream period+",&menuIncreasePeriod,NULL},
		{'6',"cycle LEDs",&menuCycleLEDS,NULL},
		{'7',"cycle I2C Addr",&menuCycleI2CAddr,NULL}
};
#define HOST_INTERFACE_MENU_ITEMS (sizeof(menu_items)/sizeof(host_menu_item_t))

static enum {
		NORMAL = 0,
		ASCII
} packet_stream_type;

// streaming packet variables
//static uint8_t packet_stream_lock;
//static host_stream_start_data_t * packet_stream_data;

// tasks
static host_task_data_t packet_tasks[HOST_INTERFACE_MAX_STREAM_TASKS];

// prototypes
void taskStreamExecute(scheduled_task_t * self);

static void taskStopAll() {
	uint8_t i;
	for (i = 0; i < HOST_INTERFACE_MAX_STREAM_TASKS; i++) {
		if (packet_tasks[i].task.active == TRUE) {
			removeTask(&packet_tasks[i].task);
		}
	}
}
static uint8_t getTaskIndexFromPID(uint8_t pid) {
	uint8_t i;
	for (i = 0; i < HOST_INTERFACE_MAX_STREAM_TASKS; i++) {
		if (packet_tasks[i].task.pid == pid) {
			return i;
		}
	}
	return i;
}
static uint8_t getTaskIndexIdle() {
	uint8_t i;
	for (i = 0; i < HOST_INTERFACE_MAX_STREAM_TASKS; i++) {
		if (packet_tasks[i].task.active == FALSE) {
			return i;
		}
	}
	return i;
}

// TODO: error checking, also better way to cache and store data
static void menuAllData() {
	uint8_t i;
	uint8_t printString[48];
	uint16_t temp[2];
	uint32_t data[4];
#if defined(HOST_DEVICE_LDC1612) || defined (HOST_DEVICE_LDC1312)
	smbus_readWord(default_addr,LDC13xx16xx_CMD_DATA_MSB_CH0,&temp[0]);
	smbus_readWord(default_addr,LDC13xx16xx_CMD_DATA_LSB_CH0,&temp[1]);
	data[0] = ((uint32_t)temp[0] << 16) | temp[1];
	smbus_readWord(default_addr,LDC13xx16xx_CMD_DATA_MSB_CH1,&temp[0]);
	smbus_readWord(default_addr,LDC13xx16xx_CMD_DATA_LSB_CH1,&temp[1]);
	data[1] = ((uint32_t)temp[0] << 16) | temp[1];
	i = sprintf ((char *)printString,"%08x,%08x\r\n",data[0],data[1]);
#elif defined(HOST_DEVICE_LDC1614) || defined (HOST_DEVICE_LDC1314)
	smbus_readWord(default_addr,LDC13xx16xx_CMD_DATA_MSB_CH2,&temp[0]);
	smbus_readWord(default_addr,LDC13xx16xx_CMD_DATA_LSB_CH2,&temp[1]);
	data[2] = ((uint32_t)temp[0] << 16) | temp[1];
	smbus_readWord(default_addr,LDC13xx16xx_CMD_DATA_MSB_CH3,&temp[0]);
	smbus_readWord(default_addr,LDC13xx16xx_CMD_DATA_LSB_CH3,&temp[1]);
	data[3] = ((uint32_t)temp[0] << 16) | temp[1];
	i = sprintf ((char *)printString,"%08x,%08x,%08x,%08x\r\n",data[0],data[1],data[2],data[3]);
#endif
	sendString((char *)printString,i);
}

void menuReadAll(scheduled_task_t * self) {
	menuAllData();
}
static void menuStartStream() {
	uint8_t x;
	char printString[16];
	// grab data from datastream & output; start process
	packet_stream_type = ASCII;
	x = getTaskIndexFromPID(HOST_PID_MENU_READ_ALL);
	if (x >= HOST_INTERFACE_MAX_STREAM_TASKS) {
		// check to see if any stream is idle
		x = getTaskIndexIdle();
		if (x >= HOST_INTERFACE_MAX_STREAM_TASKS) {
			// stop all streams
			taskStopAll();
			x = 0;
		}
	}
	else {
		// remove existing task
		removeTask(&packet_tasks[x].task);
	}
	packet_tasks[x].task.ticks = HOST_STREAM_TICKS_DEF;
	packet_tasks[x].task.pid = HOST_PID_MENU_READ_ALL;
	packet_tasks[x].task.function = &menuReadAll;
	addTask(&packet_tasks[x].task);
	x = sprintf ((char *)printString,"stream start\r\n");
	sendString((char *)printString,x);
}
static void menuStopStream() {
	uint8_t x;
	char printString[16];
	x = getTaskIndexFromPID(HOST_PID_MENU_READ_ALL);
	if (x < HOST_INTERFACE_MAX_STREAM_TASKS) {
		removeTask(&packet_tasks[x].task);
	}
	x = sprintf ((char *)printString,"stream stop\r\n");
	sendString((char *)printString,x);
}
static void menuDecreasePeriod() {
	uint8_t x;
	char printString[16];
	x = getTaskIndexFromPID(HOST_PID_MENU_READ_ALL);
	// task does not exist
	if (x >= HOST_INTERFACE_MAX_STREAM_TASKS) {
		x = sprintf ((char *)printString,"no stream\r\n");
	}
	else {
		if (packet_tasks[x].task.ticks >= (HOST_STREAM_TICKS_MIN + HOST_STREAM_TICKS_INC)) {
			packet_tasks[x].task.ticks -= HOST_STREAM_TICKS_INC;
		}
		x = sprintf ((char *)printString,"%d ms\r\n",(packet_tasks[x].task.ticks)/100);
	}
	sendString((char *)printString,x);
}
static void menuIncreasePeriod() {
	uint8_t x;
	char printString[16];
	x = getTaskIndexFromPID(HOST_PID_MENU_READ_ALL);
	// task does not exist
	if (x >= HOST_INTERFACE_MAX_STREAM_TASKS) {
		x = sprintf ((char *)printString,"no stream\r\n");
	}
	else {
		if (packet_tasks[x].task.ticks <= (HOST_STREAM_TICKS_MAX - HOST_STREAM_TICKS_INC)) {
			packet_tasks[x].task.ticks += HOST_STREAM_TICKS_INC;
		}
		x = sprintf ((char *)printString,"%d ms\r\n",(packet_tasks[x].task.ticks)/100);
	}
	sendString((char *)printString,x);
}
static void menuCycleLEDS() {
	uint8_t x;
	char printString[8];
	x = evm_LED_Set(CYCLE);
	x = sprintf ((char *)printString,"%x\r\n",x);
	sendString((char *)printString,x);
}
static void menuCycleI2CAddr() {
	uint8_t x;
	char printString[8];
	if (++default_addr > EVM_MAX_I2CADDR) {
		default_addr = EVM_MIN_I2CADDR;
	}
	default_addr = evm_changeAddr(default_addr);
	x = sprintf ((char *)printString,"%02x\r\n",default_addr);
	sendString((char *)printString,x);
}

static void readSerialFromFlash() {
	uint8_t i;
	// import serial
	for (i = 0; i < HOST_SERIAL_LENGTH; i++) {
		serial[i] = FlashSerial[i];
	}
	// validate crc
	if (calcCRC8((uint8_t *) &serial[0], HOST_SERIAL_LENGTH)) {
		memset(&serial[0],0xFF,HOST_SERIAL_LENGTH);
	}
}

void initHostInterface() {
	uint8_t i;
//	default_dev = 1;
	default_addr = EVM_DEFAULT_I2CADDR;
	// read serial data
	readSerialFromFlash();
	// TODO: scan for attached devices, add commands based on devices found
	packet_stream_type = NORMAL;
	initMenuItems();
	for (i = 0; i < HOST_INTERFACE_MENU_ITEMS; i++) {
		addMenuItem(&menu_items[i]);
	}
	// reset packet_tasks
	for (i = 0; i < HOST_INTERFACE_MAX_STREAM_TASKS; i++) {
		packet_tasks[i].task.timedout = FALSE;
		packet_tasks[i].task.active = FALSE;
	}
	initScheduler();
}

static void sendPacket (host_cmd_packet_t *packet, uint8_t size) {
//	static WORD i;
	packet->fields.crc = calcCRC8((uint8_t *)packet, size-HOST_CRC_SIZE);
	packet->bytes[size-HOST_CRC_SIZE] = packet->fields.crc;
//    if (cdcSendDataInBackground((BYTE*)packet,(WORD)size,CDC0_INTFNUM,0))
    if (cdcSendDataInBackground((BYTE*)packet,(WORD)HOST_MAX_REPLY_PACKET_LENGTH,CDC0_INTFNUM,0))
    {      //Send it
//        USBCDC_abortSend((WORD*)&i,CDC0_INTFNUM);              //Operation probably still open; cancel it
    	_nop();
    }
}
static void sendErrorPacket (host_cmd_packet_t *packet, ERROR_CODE error_code) {
	packet->fields.hdr.length = 0;
	packet->fields.hdr.err_code = (int8_t) error_code;
	sendPacket(packet,HOST_ERR_PACKET_LENGTH);
}

static void executeCmdPacket (host_cmd_packet_t * responsePkt) {
	static uint16_t x;
	static uint8_t defaults[49];
	// Load the Reply Packet Header
//    	responsePkt->fields.hdr.id = HOST_ID;
//    	responsePkt->fields.hdr.command_code = commandPkt->fields.hdr.command_code;
//    	responsePkt->fields.hdr.device_num = commandPkt->fields.hdr.device_num;
//    	responsePkt->fields.hdr.err_code = (uint8_t)ERR_OK;

	switch (responsePkt->fields.hdr.command_code)
	{
		case HOST_CMD_CONTROLLER_TYPE:
			// copy from const data
			memcpy(&(responsePkt->fields.data[0]),&HOST_CONTROLLER_TYPE[0],HOST_CONTROLLER_TYPE_LENGTH);
			responsePkt->fields.hdr.length = HOST_CONTROLLER_TYPE_LENGTH;
			sendPacket(responsePkt,HOST_OUT_HDR_SIZE + HOST_CONTROLLER_TYPE_LENGTH + HOST_CRC_SIZE);
			break;
		case HOST_CMD_FIRMWARE_VERSION:
			// hack here to reset device
			removeAllTasks();
			// copy from const data
			memcpy(&(responsePkt->fields.data[0]),&HOST_FIRMWARE_VERSION[0],HOST_FIRMWARE_VERSION_LENGTH);
			responsePkt->fields.hdr.length = HOST_FIRMWARE_VERSION_LENGTH;
			sendPacket(responsePkt,HOST_OUT_HDR_SIZE + HOST_FIRMWARE_VERSION_LENGTH + HOST_CRC_SIZE);
			break;
		case HOST_CMD_ENUMERATE_DEVICES:
			// copy from const data
			memcpy(&(responsePkt->fields.data[0]),&host_devices[0],host_devices[0] + 1);
			responsePkt->fields.hdr.length = host_devices[0] + 1;
			sendPacket(responsePkt,HOST_OUT_HDR_SIZE + host_devices[0] + 1 + HOST_CRC_SIZE);
			break;
		/*case HOST_CMD_SET_DEFAULT_DEVICE:*/
			// Set the default device
			// only one device so command not available
			/*break;*/
		case HOST_CMD_I2C_SETUP:
			// validate length and pec
			if (responsePkt->fields.hdr.length != 5) {
				sendErrorPacket(responsePkt,ERR_BAD_PACKET_SIZE);
				break;
			}
			else if (responsePkt->fields.data[3] != 0) {
				sendErrorPacket(responsePkt,ERR_PARAM_OUT_OF_RANGE);
				break;
			}
			x = (responsePkt->fields.data[2] << 8) | responsePkt->fields.data[1];
			// set the clock (kills any previous read/write)
			i2c_setClock((i2c_clock_t) (responsePkt->fields.data[0] & I2C_CLK_MASK), x);
			// set default address
			default_addr = evm_changeAddr(responsePkt->fields.data[4]);
			responsePkt->fields.data[4] = default_addr;
			sendPacket(responsePkt,HOST_OUT_HDR_SIZE + responsePkt->fields.hdr.length + HOST_CRC_SIZE);
			break;
		case HOST_CMD_SMBUS_READ_BYTES:
			// validate length
			if (responsePkt->fields.hdr.length != 3) {
				sendErrorPacket(responsePkt,ERR_BAD_PACKET_SIZE);
				break;
			}
			x = responsePkt->fields.data[2];
			// read bytes
			if (!smbus_readBytes(responsePkt->fields.data[0], &(responsePkt->fields.data[1]), x)) {
				sendErrorPacket(responsePkt,ERR_I2C_READ_TIMEOUT);
				break;
			}
			else {
				responsePkt->fields.hdr.length = x + 2;
				sendPacket(responsePkt,HOST_OUT_HDR_SIZE + responsePkt->fields.hdr.length + HOST_CRC_SIZE);
			}
			break;
		case HOST_CMD_SMBUS_WRITE_BYTES:
			// validate length
			if (responsePkt->fields.hdr.length < 2) {
				sendErrorPacket(responsePkt,ERR_BAD_PACKET_SIZE);
				break;
			}
			x = responsePkt->fields.hdr.length - 1;
			// write bytes
			if (!i2c_writeBytes(responsePkt->fields.data[0], &(responsePkt->fields.data[1]), x)) {
				sendErrorPacket(responsePkt,ERR_I2C_WRITE_TIMEOUT);
				break;
			}
			else {
				sendPacket(responsePkt,HOST_OUT_HDR_SIZE + responsePkt->fields.hdr.length + HOST_CRC_SIZE);
			}
			break;
		case HOST_CMD_I2C_READ_BYTES:
			// validate length
			if (responsePkt->fields.hdr.length != 2) {
				sendErrorPacket(responsePkt,ERR_BAD_PACKET_SIZE);
				break;
			}
			x = responsePkt->fields.data[1];
			// read bytes
			if (!i2c_readBytes(responsePkt->fields.data[0], &(responsePkt->fields.data[1]), x)) {
				sendErrorPacket(responsePkt,ERR_I2C_READ_TIMEOUT);
				break;
			}
			else {
				responsePkt->fields.hdr.length = x + 1;
				sendPacket(responsePkt,HOST_OUT_HDR_SIZE + responsePkt->fields.hdr.length + HOST_CRC_SIZE);
			}
			break;
		case HOST_CMD_I2C_WRITE_BYTES:
			// validate length
			if (responsePkt->fields.hdr.length == 0) {
				sendErrorPacket(responsePkt,ERR_BAD_PACKET_SIZE);
				break;
			}
			x = responsePkt->fields.hdr.length - 1;
			// write bytes
			if (!i2c_writeBytes(responsePkt->fields.data[0], &(responsePkt->fields.data[1]), x)) {
				sendErrorPacket(responsePkt,ERR_I2C_WRITE_TIMEOUT);
				break;
			}
			else {
				sendPacket(responsePkt,HOST_OUT_HDR_SIZE + responsePkt->fields.hdr.length + HOST_CRC_SIZE);
			}
			break;
		case HOST_CMD_START_STREAM: {
			host_stream_start_packet_t * startPkt = (host_stream_start_packet_t *) responsePkt;
			// validate if command code is supported
			for (x = 0; x < sizeof(HOST_CMD_STREAM_WHITELIST); x++) {
				if (startPkt->fields.data.command_code == HOST_CMD_STREAM_WHITELIST[x]) {
					break;
				}
			}
			if (x >= sizeof(HOST_CMD_STREAM_WHITELIST)) {
				// command code not supported for streaming
				sendErrorPacket(responsePkt,ERR_PARAM_OUT_OF_RANGE);
				break;
			}
			x = getTaskIndexFromPID(startPkt->fields.data.pid);
			if (x >= HOST_INTERFACE_MAX_STREAM_TASKS) {
				// task not found; get idle index
				x = getTaskIndexIdle();
				// no tasks are idle; maximum tasks alloted
				if (x >= HOST_INTERFACE_MAX_STREAM_TASKS) {
					sendErrorPacket(responsePkt,ERR_PARAM_OUT_OF_RANGE);
					break;
				}
			}
			else {
				// remove existing task
				removeTask(&packet_tasks[x].task);
			}
			packet_stream_type = NORMAL;
			// setup packet
			memcpy(&packet_tasks[x].packet,startPkt,HOST_STREAM_IN_PACKET_LENGTH);
			packet_tasks[x].packet.fields.hdr.command_code = startPkt->fields.data.command_code;
			packet_tasks[x].packet.fields.hdr.length -= HOST_STREAM_IN_EHDR_LEN;
			// setup data
			if (packet_tasks[x].packet.fields.hdr.length) {
				memcpy(&packet_tasks[x].packet.fields.data,&startPkt->fields.data.bytes[0],packet_tasks[x].packet.fields.hdr.length);
			}
			// setup task
			// set PID
			packet_tasks[x].task.pid = startPkt->fields.data.pid;
			// set ticks
			packet_tasks[x].task.ticks = (startPkt->fields.data.interval_divider_hi << 8) |
					startPkt->fields.data.interval_divider_lo;
			// set task function
			packet_tasks[x].task.function = &taskStreamExecute;
			// start task
			addTask(&packet_tasks[x].task);
			}
		    sendPacket(responsePkt,HOST_OUT_HDR_SIZE + responsePkt->fields.hdr.length + HOST_CRC_SIZE);
			break;
		case HOST_CMD_STOP_STREAM:
			// validate length
			if (responsePkt->fields.hdr.length != 1) {
				sendErrorPacket(responsePkt,ERR_BAD_PACKET_SIZE);
				break;
			}
			x = getTaskIndexFromPID(responsePkt->fields.data[0]);
			if (x >= HOST_INTERFACE_MAX_STREAM_TASKS) {
				sendErrorPacket(responsePkt,ERR_PARAM_OUT_OF_RANGE);
				break;
			}
			else {
				removeTask(&packet_tasks[x].task);
				sendPacket(responsePkt,HOST_OUT_HDR_SIZE + responsePkt->fields.hdr.length + HOST_CRC_SIZE);
			}
			break;
		case HOST_CMD_STOP_ALL_STREAMS:
			taskStopAll();
			sendPacket(responsePkt,HOST_OUT_HDR_SIZE + responsePkt->fields.hdr.length + HOST_CRC_SIZE);
			break;
		case HOST_CMD_READ_SERIAL:
			memcpy(&(responsePkt->fields.data[0]),&serial[0],HOST_SERIAL_LENGTH);
			responsePkt->fields.hdr.length = HOST_SERIAL_LENGTH;
			sendPacket(responsePkt,HOST_OUT_HDR_SIZE + HOST_SERIAL_LENGTH + HOST_CRC_SIZE);
			break;
		case HOST_CMD_WRITE_SERIAL:
			if (responsePkt->fields.hdr.length != HOST_SERIAL_LENGTH) {
				sendErrorPacket(responsePkt,ERR_BAD_PACKET_SIZE);
				break;
			}
			// validate crc
			for (x = 0; x < HOST_SERIAL_LENGTH; x++) {
				serial[x] = responsePkt->fields.data[x] ^ sharedSecret[x];
			}
			if (calcCRC8((uint8_t *) &serial[0], HOST_SERIAL_LENGTH)) {
				// restore serial
				readSerialFromFlash();
				sendErrorPacket(responsePkt,ERR_PARAM_OUT_OF_RANGE);
				break;
			}
			sendPacket(responsePkt,HOST_OUT_HDR_SIZE + HOST_SERIAL_LENGTH + HOST_CRC_SIZE);
			break;
		case HOST_CMD_ENTER_BSL:
			taskStopAll();
			USB_reset ();
			USB_disable();
			_disable_interrupts();
			((void (*)())0x1000)();
			break;
		// LDC1000-specific cases
		case HOST_CMD_READ_ALL_DATA: {

			uint16_t data = 1;
	//		uint16_t mask = FDC1004_BIT_DONE_1 | FDC1004_BIT_DONE_2 | FDC1004_BIT_DONE_3 | FDC1004_BIT_DONE_4;
			uint8_t addr = responsePkt->fields.data[0];
			// validate length
			if (responsePkt->fields.hdr.length != 1) {
				sendErrorPacket(responsePkt,ERR_BAD_PACKET_SIZE);
				break;
			}
			if (!smbus_readWord(addr, LDC13xx16xx_CMD_MUX_CONFIG, &data)) {
				sendErrorPacket(responsePkt,ERR_I2C_READ_TIMEOUT);
				break;
			}
		//	if ((data & mask) != mask) {
		//		sendErrorPacket(responsePkt,ERR_COMMAND_BUSY);
		//		break;
		//	}
			smbus_readWord(addr, LDC13xx16xx_CMD_DATA_MSB_CH0, &data);
			responsePkt->fields.data[1] = data >> 8;
			responsePkt->fields.data[2] = data & 0xFF;
			smbus_readWord(addr, LDC13xx16xx_CMD_DATA_LSB_CH0, &data);
			responsePkt->fields.data[3] = data >> 8;
			responsePkt->fields.data[4] = data & 0xFF;

			smbus_readWord(addr, LDC13xx16xx_CMD_DATA_MSB_CH1, &data);
			responsePkt->fields.data[5] = data >> 8;
			responsePkt->fields.data[6] = data & 0xFF;
			smbus_readWord(addr, LDC13xx16xx_CMD_DATA_LSB_CH1, &data);
			responsePkt->fields.data[7] = data >> 8;
			responsePkt->fields.data[8] = data & 0xFF;

			//Edited by Wipro

			
			#if defined(HOST_DEVICE_LDC1612) || defined (HOST_DEVICE_LDC1312)

			responsePkt->fields.hdr.length = 9;

			
			#elif defined(HOST_DEVICE_LDC1614) || defined (HOST_DEVICE_LDC1314)

			
				
				smbus_readWord(addr, LDC13xx16xx_CMD_DATA_MSB_CH2, &data);
				responsePkt->fields.data[9] = data >> 8;
				responsePkt->fields.data[10] = data & 0xFF;
				smbus_readWord(addr, LDC13xx16xx_CMD_DATA_LSB_CH2, &data);
				responsePkt->fields.data[11] = data >> 8;
				responsePkt->fields.data[12] = data & 0xFF;
			
				smbus_readWord(addr, LDC13xx16xx_CMD_DATA_MSB_CH3, &data);
				responsePkt->fields.data[13] = data >> 8;
				responsePkt->fields.data[14] = data & 0xFF;
				smbus_readWord(addr, LDC13xx16xx_CMD_DATA_LSB_CH3, &data);
				responsePkt->fields.data[15] = data >> 8;
				responsePkt->fields.data[16] = data & 0xFF;
				
				/* Stub code for testing only */
				/*
				responsePkt->fields.data[9] = responsePkt->fields.data[1];
				responsePkt->fields.data[10] = responsePkt->fields.data[2];
				responsePkt->fields.data[11] = responsePkt->fields.data[3];
				responsePkt->fields.data[12] = responsePkt->fields.data[4];
				responsePkt->fields.data[13] = responsePkt->fields.data[5];
				responsePkt->fields.data[14] = responsePkt->fields.data[6];
				responsePkt->fields.data[15] = responsePkt->fields.data[7];
				responsePkt->fields.data[16] = responsePkt->fields.data[8];
				*/
				/***** Upto this .. Stub Code */

				responsePkt->fields.hdr.length = 17;
			// Edit ends.
				
			#endif	
			sendPacket(responsePkt,HOST_OUT_HDR_SIZE + responsePkt->fields.hdr.length + HOST_CRC_SIZE);
			break;
		}
		case HOST_CMD_CH_READ: {
			// validate length
			if (responsePkt->fields.hdr.length != 1) {
				sendErrorPacket(responsePkt,ERR_BAD_PACKET_SIZE);
				break;
			}
			x = 1;
			x += evm_readFreq(responsePkt->fields.data[0],&responsePkt->fields.data[1]);
			responsePkt->fields.hdr.length = x;
			sendPacket(responsePkt,HOST_OUT_HDR_SIZE + responsePkt->fields.hdr.length + HOST_CRC_SIZE);
			break;
		}
		case HOST_CMD_LED_STATES:
			// read
			if (responsePkt->fields.hdr.length == 0) {
				responsePkt->fields.data[0] = evm_LED_State();
				responsePkt->fields.hdr.length = 1;
				sendPacket(responsePkt,HOST_OUT_HDR_SIZE + responsePkt->fields.hdr.length + HOST_CRC_SIZE);
				break;
			}
			// validate length
			else if (responsePkt->fields.hdr.length != 1) {
				sendErrorPacket(responsePkt,ERR_BAD_PACKET_SIZE);
				break;
			}
			evm_LED_Set ((evm_led_state_t) (responsePkt->fields.data[0]));
			sendPacket(responsePkt,HOST_OUT_HDR_SIZE + responsePkt->fields.hdr.length + HOST_CRC_SIZE);
			break;
//		case HOST_CMD_CALIBRATE:
//			break;
		case HOST_CMD_DEFAULTS_A:
			// validate length
			if (responsePkt->fields.hdr.length != 0) {
				sendErrorPacket(responsePkt,ERR_BAD_PACKET_SIZE);
				break;
			}
			// read
			responsePkt->fields.hdr.length = evm_readDefaults(0,&(responsePkt->fields.data[0]),24);
			sendPacket(responsePkt,HOST_OUT_HDR_SIZE + responsePkt->fields.hdr.length + HOST_CRC_SIZE);
			break;
		case HOST_CMD_DEFAULTS_B:
			// validate length
			if (responsePkt->fields.hdr.length != 0) {
				sendErrorPacket(responsePkt,ERR_BAD_PACKET_SIZE);
				break;
			}
			// read
			responsePkt->fields.hdr.length = evm_readDefaults(24,&(responsePkt->fields.data[0]),24);
			sendPacket(responsePkt,HOST_OUT_HDR_SIZE + responsePkt->fields.hdr.length + HOST_CRC_SIZE);
			break;
		case HOST_CMD_SAVE_SETTINGS_A:
			// validate length
			if (responsePkt->fields.hdr.length != 24) {
				sendErrorPacket(responsePkt,ERR_BAD_PACKET_SIZE);
				break;
			}
			memcpy(&defaults[0],&(responsePkt->fields.data[0]),responsePkt->fields.hdr.length);
			sendPacket(responsePkt,HOST_OUT_HDR_SIZE + responsePkt->fields.hdr.length + HOST_CRC_SIZE);
			break;
			// TODO: separate function for serial writing
		case HOST_CMD_SAVE_SETTINGS_B:
			// validate length
			if (responsePkt->fields.hdr.length != 24) {
				sendErrorPacket(responsePkt,ERR_BAD_PACKET_SIZE);
				break;
			}
			memcpy(&defaults[24],&(responsePkt->fields.data[0]),responsePkt->fields.hdr.length);
			defaults[48] = calcCRC8(&defaults[0], 48);
			// save serial
			//Erase INFOC
//			do {
					//Erase INFOC
					Flash_SegmentErase((uint16_t*)INFOC_START);
//					x = Flash_EraseCheck((uint16_t*)INFOC_START,128/2);
//			} while (x == FLASH_STATUS_ERROR);
			if (!(calcCRC8((uint8_t *) &serial[0], HOST_SERIAL_LENGTH))) {
				FlashWrite_8(
							 (uint8_t*)serial,
							 (uint8_t*)FlashSerial,
							 HOST_SERIAL_LENGTH
							 );
			}
			// save defaults
			evm_saveDefaults(&defaults[0]);
			sendPacket(responsePkt,HOST_OUT_HDR_SIZE + responsePkt->fields.hdr.length + HOST_CRC_SIZE);
			break;
		default:
			sendErrorPacket(responsePkt,ERR_INVALID_FUNCTION_CODE);
			break;
	}   // end switch
}

void processCmdPacket ()
{
static host_cmd_packet_t       commandPkt;
static host_cmd_packet_t       responsePkt;
static uint16_t x;

	// Set up rcv operation for 1 byte of data to make sure it is a header
	// If USBCDC_receiveData fails because of surprise removal
	// or suspended by host abort and leave main loop
	if (USBCDC_receiveData((BYTE*)&commandPkt.fields.hdr.id, 1, CDC0_INTFNUM) ==
		kUSBCDC_busNotAvailable)
	{
		x = 1;
		USBCDC_abortReceive(&x,CDC0_INTFNUM);
		return;
	}

	// check namespace, if not the same, trigger ASCII menu
	if (commandPkt.fields.hdr.id != HOST_ID) {
		if (!executeMenuItem(commandPkt.fields.hdr.id)) {
			x = USBCDC_rejectData(CDC0_INTFNUM);
			// consume excess bytes in buffer
		}
		return;
	}

    // Set up rcv operation for rest of header data
    // If USBCDC_receiveData fails because of surprise removal
    // or suspended by host abort and leave main loop
    if (USBCDC_receiveData((BYTE*)&commandPkt.fields.hdr.command_code ,HOST_IN_HDR_SIZE-1, CDC0_INTFNUM) ==
        kUSBCDC_busNotAvailable)
    {
        x = HOST_IN_HDR_SIZE - 1;
        USBCDC_abortReceive(&x,CDC0_INTFNUM);
        return;
    }

    // validate header
    // Validate packet length
     if (commandPkt.fields.hdr.length > HOST_MAX_CMD_DATA_LEN) {
         memcpy((uint8_t *)&responsePkt,(uint8_t *)&commandPkt, HOST_IN_HDR_SIZE);
         sendErrorPacket(&responsePkt,ERR_BAD_PACKET_SIZE);
         return;
     }
     // check device number out of range
     else if (commandPkt.fields.hdr.device_num > 1) {
         memcpy((uint8_t *)&responsePkt,(uint8_t *)&commandPkt, HOST_IN_HDR_SIZE);
         sendErrorPacket(&responsePkt,ERR_INVALID_DEVICE);
         return;
     }
     // check for host-side crc errors
     else if (commandPkt.fields.hdr.err_code != (int8_t)ERR_OK) {
         memcpy((uint8_t *)&responsePkt,(uint8_t *)&commandPkt, HOST_IN_HDR_SIZE);
         sendErrorPacket(&responsePkt,ERR_PARAM_OUT_OF_RANGE);
         return;
     }

    // Receive the rest of the data
    if (USBCDC_receiveData((BYTE*)&commandPkt.fields.data[0],commandPkt.fields.hdr.length + HOST_CRC_SIZE, CDC0_INTFNUM) ==
        kUSBCDC_busNotAvailable)
    {
        x = commandPkt.fields.hdr.length+HOST_CRC_SIZE;
        USBCDC_abortReceive(&x,CDC0_INTFNUM);
        return;
    }

    x = HOST_IN_HDR_SIZE + commandPkt.fields.hdr.length + HOST_CRC_SIZE;

	// Valid packet will compute to a CRC of 0, when the CRC of the entire packet, including the CRC Byte is computed
	if (calcCRC8((uint8_t *) &commandPkt, (uint8_t) x))
	{   // CRC failed
		memcpy((uint8_t *)&responsePkt,(uint8_t *)&commandPkt, HOST_IN_HDR_SIZE); // echo header
		sendErrorPacket(&responsePkt,ERR_COM_CRC_FAILED);
        return;
	}
	// execute command
	else
	{   // CRC passed
		memcpy((uint8_t *)&responsePkt,(uint8_t *)&commandPkt, x);
		executeCmdPacket(&responsePkt);
		return;
	}
}

void taskStreamExecute(scheduled_task_t * self) {
	static uint8_t x, y;
	static host_cmd_packet_t responsePkt;
	if (packet_stream_type == NORMAL) {
		x = getTaskIndexFromPID(self->pid);
		if (x < HOST_INTERFACE_MAX_STREAM_TASKS) {
			y = HOST_IN_HDR_SIZE + packet_tasks[x].packet.fields.hdr.length + HOST_CRC_SIZE;
			memcpy((uint8_t *)&responsePkt,(uint8_t *)&packet_tasks[x].packet, y);
			executeCmdPacket(&responsePkt);
		}
	}
}

