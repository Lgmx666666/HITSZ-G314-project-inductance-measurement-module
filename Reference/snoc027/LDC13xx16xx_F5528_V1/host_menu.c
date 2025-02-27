/** @file
 * @brief Implementation of functions for host_menu
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "host_menu.h"
#include "host_interface.h"

#include "USB_API/USB_Common/device.h"
#include "USB_API/USB_Common/types.h"               //Basic Type declarations
#include "USB_API/USB_Common/usb.h"                 //USB-specific functions

#include "USB_API/USB_CDC_API/UsbCdc.h"
#include "usbConstructs.h"
#include "USB_config/descriptors.h"

static host_menu_item_t * item_head = NULL;
static host_menu_item_t * item_tail = NULL;

static host_menu_item_t item_home;

void sendString(char * string, uint8_t size) {
	static WORD i;
    if (cdcSendDataWaitTilDone((BYTE*)string,(WORD)size,CDC0_INTFNUM,0))
    {      //Send it
        USBCDC_abortSend((WORD*)&i,CDC0_INTFNUM);              //Operation probably still open; cancel it
    	// do nothing
    }
}

static void printMenu() {
	int8_t i;
	host_menu_item_t * item_ptr;
	char printString[32];
    i = sprintf ((char *)printString,"\r\n%s v%d.%d.%d.%d\r\n",
    		(char *)HOST_CONTROLLER_TYPE,
    		HOST_FIRMWARE_VERSION[0],HOST_FIRMWARE_VERSION[1],
    		HOST_FIRMWARE_VERSION[2],HOST_FIRMWARE_VERSION[3]);
    sendString((char *)printString,i);
    item_ptr = item_head;
    while (item_ptr != NULL) {
        i = sprintf ((char *)printString," %c : %s\r\n",(char) item_ptr->id, item_ptr->name);
        sendString((char *)printString,i);
        item_ptr = item_ptr->next;
    }
}
void initMenuItems() {
	item_home.id = '?';
	item_home.name = "menu";
	item_home.action = &printMenu;
    item_home.next = NULL;
    addMenuItem(&item_home);
}
uint8_t executeMenuItem(uint8_t id) {
	uint8_t executed = FALSE;
	host_menu_item_t * item_ptr;
	item_ptr = item_head;
	while (item_ptr != NULL) {
		if (item_ptr->id == id) {
			(item_ptr->action)();
			executed = TRUE;
		}
		item_ptr = item_ptr->next;
	}
	return executed;
}

void addMenuItem(host_menu_item_t * item) {
	if (item_head == NULL) {
		item_head = item;
		item_tail = item;
		item_tail->next = NULL;
	}
	else {
		item_tail->next = item;
		item_tail = item;
		item_tail->next = NULL;
	}
}

