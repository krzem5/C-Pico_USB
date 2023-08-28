#ifndef _USB_MSC_H_
#define _USB_MSC_H_ 1
#include <stdint.h>



#define USB_SCSI_CMD_TEST_UNIT_READY 0x00
#define USB_SCSI_CMD_INQUIRY 0x12
#define USB_SCSI_CMD_MODE_SELECT_6 0x15
#define USB_SCSI_CMD_MODE_SENSE_6 0x1a
#define USB_SCSI_CMD_START_STOP_UNIT 0x1b
#define USB_SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL 0x1e
#define USB_SCSI_CMD_READ_CAPACITY_10 0x25
#define USB_SCSI_CMD_REQUEST_SENSE 0x03
#define USB_SCSI_CMD_READ_FORMAT_CAPACITY 0x23
#define USB_SCSI_CMD_READ_10 0x28
#define USB_SCSI_CMD_WRITE_10 0x2a

#define USB_MSC_STATUS_PASSED 0
#define USB_MSC_STATUS_FAILED 1

#define USB_MSC_SECTOR_SIZE 512



typedef void (*usb_msc_sector_callback_t)(void*,uint32_t,void*);



typedef struct __attribute__((packed)) _USB_SCSI_COMMAND{
	uint32_t signature;
	uint32_t tag;
	uint32_t size;
	uint8_t direction;
	uint8_t lun;
	uint8_t length;
	uint8_t command[16];
} usb_scsi_command_t;



typedef struct __attribute__((packed)) _USB_SCSI_STATUS{
	uint32_t signature;
	uint32_t tag;
	uint32_t residue;
	uint8_t status;
} usb_scsi_status_t;



typedef struct _USB_MSC_HANDLER_CONTEXG{
	uint8_t current_sector[USB_MSC_SECTOR_SIZE];
	uint8_t h2d_endpoint;
	uint8_t d2h_endpoint;
	uint32_t size;
	const char* vendor;
	const char* product;
	const char* revision;
	usb_scsi_status_t status;
	_Bool send_status;
	_Bool waiting_for_write;
	volatile _Bool drive_acknowledged;
	uint32_t current_sector_lba;
	uint32_t current_lba_offset;
	usb_msc_sector_callback_t sector_callback;
	void* sector_callback_ctx;
} usb_msc_handler_context_t;



void usb_msc_init(uint8_t h2d_endpoint,uint8_t d2h_endpoint,uint32_t size,const char* vendor,const char* product,const char* revision,usb_msc_sector_callback_t sector_callback,void* sector_callback_ctx,usb_msc_handler_context_t* ctx);



void usb_msc_h2d_handler(void* ctx,const uint8_t* buffer,uint16_t length);



void usb_msc_d2h_handler(void* ctx,const uint8_t* buffer,uint16_t length);



#endif
