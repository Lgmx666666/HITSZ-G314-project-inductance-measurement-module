/** @file
 * @brief Host String Menu
 * @defgroup usb usb
 * @{
 * @ingroup usb
 *
 * @brief <b>Host String Menu</b> @n@n
 * This file contains functions that allow string menus to be printed to USB CDC.
 *
 * @author
 * Charles Cheung (chuck@ti.com)
 */

#ifndef HOST_MENU_H_
#define HOST_MENU_H_

#define HOST_MENU_MS 24000	   // ticks for 1ms

typedef struct host_menu_item {
	uint8_t id;
	char * name;
	void (* action)();
	struct host_menu_item * next;
} host_menu_item_t;

extern const uint8_t HOST_CONTROLLER_TYPE[];
extern const uint8_t HOST_FIRMWARE_VERSION[];
extern const uint8_t HOST_CMD_STREAM_WHITELIST[];

/** Send String
Send the string through USBCDC
@param string string
@param size string size
@remarks
CDC wait till done method called (blocking)
*/
void sendString(char * string, uint8_t size);
/** Send String in Background
Send the string through USBCDC
@param string string
@param size string size
@remarks
non-blocking
*/
void sendStringBackground(char * string, uint8_t size);
/** Initialize Menu Items
Initialize the unpopulated menu
@remarks
adds the show menu '?' menu item
*/
void initMenuItems();
/** Execute menu item
Calls the function the specified menu item points to
@param id id of the menu item
@return TRUE if menu item found and function called, FALSE otherwise
*/
uint8_t executeMenuItem(uint8_t id);
/** Add menu item
Adds the specified item to the menu
@param item pointer to the menu item to add
*/
void addMenuItem(host_menu_item_t * item);

#endif /* HOST_MENU_H_ */

/** @} */
