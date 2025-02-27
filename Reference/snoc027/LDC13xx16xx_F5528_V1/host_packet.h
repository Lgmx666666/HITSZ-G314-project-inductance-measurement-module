/** @file
 * @brief Interface definitions and structures for host protocol.
 * @defgroup usb usb
 * @{
 * @ingroup usb
 *
 * @brief <b>Host Protocol Definitions</b> @n@n
 * This file contains host protocol definitions
 * Based on code by Chris Tracy (chris.tracy@ti.com)
 *
 * @author
 * Charles Cheung (chuck@ti.com)
 */

#ifndef HOST_PACKET_H_
#define HOST_PACKET_H_

#define HOST_ID                     'L'
#define HOST_CRC_SIZE                1
#define HOST_IN_HDR_SIZE             5
#define HOST_OUT_HDR_SIZE            5

#define HOST_MAX_CMD_DATA_LEN        26  // maximum data length for all commands
#define HOST_MAX_CMD_PACKET_LENGTH       (HOST_IN_HDR_SIZE + HOST_MAX_CMD_DATA_LEN + HOST_CRC_SIZE)
#define HOST_MAX_REPLY_PACKET_LENGTH     (HOST_OUT_HDR_SIZE + HOST_MAX_CMD_DATA_LEN + HOST_CRC_SIZE)
#define HOST_ERR_PACKET_LENGTH       (HOST_OUT_HDR_SIZE + HOST_CRC_SIZE)

#define HOST_STREAM_IN_DATA_SIZE     HOST_MAX_CMD_DATA_LEN
#define HOST_STREAM_OUT_DATA_SIZE    1

#define HOST_STREAM_IN_PACKET_LENGTH    (HOST_OUT_HDR_SIZE + HOST_STREAM_IN_DATA_SIZE + HOST_CRC_SIZE)
#define HOST_STREAM_OUT_PACKET_LENGTH   (HOST_OUT_HDR_SIZE + HOST_STREAM_OUT_DATA_SIZE + HOST_CRC_SIZE)

#define HOST_STREAM_IN_EHDR_LEN		 5   // enhanced header length

#define HOST_SERIAL_LENGTH 16

// Standard Commands
#define HOST_CMD_CLEAR_FAULTS          0x00
#define HOST_CMD_CONTROLLER_TYPE       0x01
#define HOST_CMD_FIRMWARE_VERSION      0x02
#define HOST_CMD_ENUMERATE_DEVICES     0x03
#define HOST_CMD_SET_DEFAULT_DEVICE    0x04
#define HOST_CMD_START_STREAM          0x05
#define HOST_CMD_STOP_STREAM           0x06
#define HOST_CMD_STOP_ALL_STREAMS      0x07
#define HOST_CMD_READ_SERIAL           0x08
#define HOST_CMD_WRITE_SERIAL          0x09
#define HOST_CMD_ENTER_BSL             0x0A
#define HOST_CMD_SPI_READ_BYTES        0x10
#define HOST_CMD_SPI_WRITE_BYTES       0x11
#define HOST_CMD_SMBUS_READ_BYTES      0x12
#define HOST_CMD_SMBUS_WRITE_BYTES     0x13
#define HOST_CMD_I2C_READ_BYTES        0x14
#define HOST_CMD_I2C_WRITE_BYTES       0x15
#define HOST_CMD_SPI_SETUP             0x1A
#define HOST_CMD_I2C_SETUP             0x1B

// Error Codes
//
// Note: These codes MUST match the values of their counterparts defined
//       in the USB2ANY SDK (see USB2ANY_SDK.h).
//
typedef enum
{
    ERR_OK                      =  0,
    ERR_COM_RX_OVERFLOW         = -1,
    ERR_COM_RX_BUF_EMPTY        = -2,
    ERR_COM_TX_BUF_FULL         = -3,
    ERR_COM_TX_STALLED          = -4,
    ERR_COM_TX_FAILED           = -5,
    ERR_COM_OPEN_FAILED         = -6,
    ERR_COM_PORT_NOT_OPEN       = -7,
    ERR_COM_PORT_IS_OPEN        = -8,
    ERR_COM_READ_TIMEOUT        = -9,
    ERR_COM_READ_ERROR          = -10,
    ERR_COM_WRITE_ERROR         = -11,
    ERR_DEVICE_NOT_FOUND        = -12,
    ERR_COM_CRC_FAILED          = -13,

    ERR_INVALID_PORT            = -20,
    ERR_ADDRESS_OUT_OF_RANGE    = -21,
    ERR_INVALID_FUNCTION_CODE   = -22, // invalid command code
    ERR_BAD_PACKET_SIZE         = -23, // invalid length
    ERR_INVALID_HANDLE          = -24, // invalid device
    ERR_OPERATION_FAILED        = -25,
    ERR_PARAM_OUT_OF_RANGE      = -26, // invalid parameter
    ERR_PACKET_OUT_OF_SEQUENCE  = -27,
    ERR_INVALID_PACKET_HEADER   = -28, // invalid header
    ERR_UNIMPLEMENTED_FUNCTION  = -29,
    ERR_TOO_MUCH_DATA           = -30,
    ERR_INVALID_DEVICE          = -31,
    ERR_UNSUPPORTED_FIRMWARE    = -32,
    ERR_BUFFER_TOO_SMALL        = -33,
    ERR_NO_DATA                 = -34,
    ERR_RESOURCE_CONFLICT       = -35,
    ERR_NO_EVM                  = -36,
    ERR_COMMAND_BUSY            = -37,
    ERR_ADJ_POWER_FAIL          = -38,

    ERR_I2C_INIT_ERROR          = -40,
    ERR_I2C_READ_ERROR          = -41,
    ERR_I2C_WRITE_ERROR         = -42,
    ERR_I2C_BUSY                = -43,
    ERR_I2C_ADDR_NAK            = -44,
    ERR_I2C_DATA_NAK            = -45,
    ERR_I2C_READ_TIMEOUT        = -46,
    ERR_I2C_READ_DATA_TIMEOUT   = -47,
    ERR_I2C_READ_COMP_TIMEOUT   = -48,
    ERR_I2C_WRITE_TIMEOUT       = -49,
    ERR_I2C_WRITE_DATA_TIMEOUT  = -50,
    ERR_I2C_WRITE_COMP_TIMEOUT  = -51,
    ERR_I2C_NOT_MASTER          = -52,
    ERR_I2C_ARBITRATION_LOST    = -53,
    ERR_I2C_NO_PULLUP_POWER     = -54,

    ERR_SPI_INIT_ERROR          = -60,
    ERR_SPI_WRITE_READ_ERROR    = -61,

    ERR_DATA_WRITE_ERROR        = -70,
    ERR_DATA_READ_ERROR         = -71,
    ERR_TIMEOUT                 = -72

} ERROR_CODE;


// header data structures
typedef struct
{
    uint8_t  id;             // 'L'
    uint8_t  command_code;
    uint8_t  device_num;
    int8_t   err_code;
    uint8_t  length;
} host_hdr_t;

// standard packet structures
typedef union
{
    uint8_t    bytes[HOST_MAX_CMD_PACKET_LENGTH];
    struct
    {
    	host_hdr_t        hdr;
        uint8_t           data[HOST_MAX_CMD_DATA_LEN];    // Variable length depending on specific command
        uint8_t           crc;                           // Note CRC will immediately follow last byte of data. This reserves room for CRC but not actually hold CRC
    } fields;
} host_cmd_packet_t;

typedef union
{
    uint8_t    bytes[HOST_ERR_PACKET_LENGTH];
    struct
    {
    	host_hdr_t       hdr;
        uint8_t          crc;                           // Note CRC will immediately follow last byte of data. This reserves room for CRC but not actually hold CRC
    } fields;
} host_err_packet_t;

// stream data structures
//typedef struct
//{
//	uint8_t command_code;
//	uint8_t bytes [HOST_MAX_CMD_DATA_LEN];
//} host_stream_data_t;

typedef struct
{
	uint8_t				 pid;
    uint8_t              interval_divider_lo;
    uint8_t              interval_divider_hi;
    uint8_t              format;				// only header+data+crc (0) supported at the moment
	uint8_t              command_code;
	uint8_t              bytes [HOST_MAX_CMD_DATA_LEN-HOST_STREAM_IN_EHDR_LEN];
} host_stream_start_data_t;

// input packet
typedef union
{
    uint8_t    bytes[HOST_STREAM_IN_PACKET_LENGTH];
    struct
    {
    	host_hdr_t                  hdr;
        host_stream_start_data_t    data;
        uint8_t                     crc;
    } fields;
} host_stream_start_packet_t;

// output packet
typedef union
{
    uint8_t    bytes[HOST_STREAM_OUT_PACKET_LENGTH];
    struct
    {
    	host_hdr_t        hdr;
    	uint8_t           pid;
        uint8_t           crc;
    } fields;
} host_stream_stop_packet_t;

// i2c packet structures
typedef union
{
    uint8_t    bytes[HOST_MAX_CMD_PACKET_LENGTH];
    struct
    {
    	host_hdr_t        hdr;
        uint8_t           data[HOST_MAX_CMD_DATA_LEN];    // Variable length depending on specific command
        uint8_t           crc;                           // Note CRC will immediately follow last byte of data. This reserves room for CRC but not actually hold CRC
    } fields;
} host_i2c_packet_t;

// supported device command structures
// TODO: add standard device index packet

#endif /* HOST_PACKET_H_ */
