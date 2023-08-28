#ifndef _USB_USB_H_
#define _USB_USB_H_ 1
#include <hardware/structs/usb.h>
#include <stdint.h>
#include <usb/common.h>



#define USB_ENDPOINT_INDEX(index,is_h2d) (((index)<<1)|(!!(is_h2d)))

#define USB_DECLARE_DEVICE_DESCRIPTOR(vendor_id_,product_id_,revision_id_,manufacturer_string_id_,product_string_id_,serial_string_id_) \
	.vendor_id=(vendor_id_), \
	.product_id=(product_id_), \
	.revision_id=(revision_id_), \
	.manufacturer_string_id=(manufacturer_string_id_), \
	.product_string_id=(product_string_id_), \
	.serial_string_id=(serial_string_id_)
#define USB_DECLARE_CONFIGURATION_DESCRIPTOR(descriptor,length) \
	.configuration_descriptor=(descriptor), \
	.configuration_descriptor_length=(length)
#define USB_DECLARE_STRING_TABLE(table) .string_table=(table)
#define USB_DECLARE_ENDPOINT_HANDLER(index,is_h2d,handler,ctx) \
	.endpoints[USB_ENDPOINT_INDEX((index),(is_h2d))]={ \
		(handler), \
		(ctx) \
	}
#define USB_DECLARE_HID_REPORT_TABLE(table) .hid_report_table=(table)

#define USB_DEVICE_INTERFACE_DESCRIPTOR(interface_index,endpoint_count,class,subclass,protocol) \
	9, \
	USB_DESCRIPTOR_TYPE_INTERFACE, \
	(interface_index), \
	0, \
	(endpoint_count), \
	(class), \
	(subclass), \
	(protocol), \
	0
#define USB_DEVICE_ENDPOINT_DESCRIPTOR(index,is_h2d,transfer_type,max_length,poll_interval) \
	7, \
	USB_DESCRIPTOR_TYPE_ENDPOINT, \
	(index)|((is_h2d)?0:USB_ENDPOIND_DIRECTION_IN), \
	(transfer_type), \
	(max_length), \
	(max_length)>>8, \
	(poll_interval)
#define USB_DEVICE_HID_DESCRIPTOR(descriptor_count,report_length) \
	9, \
	USB_DESCRIPTOR_TYPE_HID, \
	0x11, \
	0x01, \
	0x00, \
	(descriptor_count), \
	0x22, \
	(report_length), \
	(report_length)>>8

#define USB_DEVICE_FLAG_CONFIGURED 1
#define USB_DEVICE_FLAG_SWITCH_ADDRESS 2

#define USB_TRANSFER_FLAG_STREAM 0x80



typedef struct _USB_STREAM{
	void (*callback)(void*,uint32_t,uint8_t,void*);
	void* ctx;
} usb_stream_t;



typedef struct _USB_ENDPOINT_HANDLER{
	void (*handler)(void*,const uint8_t*,uint16_t);
	void* ctx;
} usb_endpoint_handler_t;



typedef struct _USB_HID_REPORT{
	const uint8_t* report;
	uint16_t length;
} usb_hid_report_t;



typedef struct _USB_DEVICE_CONFIG{
	uint16_t vendor_id;
	uint16_t product_id;
	uint16_t revision_id;
	uint8_t manufacturer_string_id;
	uint8_t product_string_id;
	uint8_t serial_string_id;
	const uint8_t*const configuration_descriptor;
	uint16_t configuration_descriptor_length;
	const char*const* string_table;
	usb_endpoint_handler_t endpoints[USB_NUM_ENDPOINTS<<1];
	const usb_hid_report_t* hid_report_table;
} usb_device_config_t;



typedef struct _USB_ENDPOINT{
	uint8_t transfer_type;
	volatile _Bool is_stream;
	uint16_t max_packet_length;
	union{
		const void*volatile buffer;
		struct{
			void (*volatile stream_callback)(void*,uint32_t,uint8_t,void*);
			void*volatile stream_ctx;
		};
	};
	volatile uint32_t offset;
	volatile uint32_t length;
} usb_endpoint_t;



typedef struct _USB_DEVICE{
	const usb_device_config_t* config;
	uint16_t configuration_descriptor_interface_count;
	uint16_t string_table_length;
	volatile uint8_t flags;
	volatile uint8_t new_address;
	volatile uint32_t endpoint_pids;
	usb_endpoint_t endpoints[USB_NUM_ENDPOINTS<<1];
} usb_device_t;



void usb_init(const usb_device_config_t* config);



void usb_transfer(uint8_t index,const void* buffer,uint32_t length);



uint8_t usb_get_device_flags(void);



#endif
