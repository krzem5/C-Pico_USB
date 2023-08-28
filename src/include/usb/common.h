#ifndef _USB_COMMON_H_
#define _USB_COMMON_H_
#include <stdint.h>



#define USB_ENDPOIND_DIRECTION_IN 0x80

#define USB_DESCRIPTOR_TYPE_DEVICE 0x01
#define USB_DESCRIPTOR_TYPE_CONFIG 0x02
#define USB_DESCRIPTOR_TYPE_STRING 0x03
#define USB_DESCRIPTOR_TYPE_INTERFACE 0x04
#define USB_DESCRIPTOR_TYPE_ENDPOINT 0x05
#define USB_DESCRIPTOR_TYPE_HID 0x21
#define USB_DESCRIPTOR_TYPE_HID_REPORT 0x22

#define USB_REQUEST_GET_STATUS 0x00
#define USB_REQUEST_CLEAR_FEATURE 0x01
#define USB_REQUEST_SET_FEATURE 0x03
#define USB_REQUEST_SET_ADDRESS 0x05
#define USB_REQUEST_GET_DESCRIPTOR 0x06
#define USB_REQUEST_SET_DESCRIPTOR 0x07
#define USB_REQUEST_GET_CONFIGURATION 0x08
#define USB_REQUEST_SET_CONFIGURATION 0x09
#define USB_REQUEST_GET_INTERFACE 0x0a
#define USB_REQUEST_SET_INTERFACE 0x0b
#define USB_REQUEST_SYNC_FRAME 0x0c
#define USB_REQUEST_MSC_GET_MAX_LUN 0xfe
#define USB_REQUEST_MSC_RESET 0xff

#define USB_TRANSFER_TYPE_CONTROL 0
#define USB_TRANSFER_TYPE_ISOCHRONOUS 1
#define USB_TRANSFER_TYPE_BULK 2
#define USB_TRANSFER_TYPE_INTERRUPT 3



typedef struct __attribute__((packed)) _USB_SETUP_PACKET{
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
} usb_setup_packet_t;



typedef struct __attribute__((packed)) _USB_DEVICE_DESCRIPTOR{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdUSB;
	uint8_t bDeviceClass;
	uint8_t bDeviceSubClass;
	uint8_t bDeviceProtocol;
	uint8_t bMaxPacketSize0;
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t bcdDevice;
	uint8_t iManufacturer;
	uint8_t iProduct;
	uint8_t iSerialNumber;
	uint8_t bNumConfigurations;
} usb_device_descriptor_t;



typedef struct __attribute__((packed)) _USB_CONFIGURATION_DESCRIPTOR{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wTotalLength;
	uint8_t bNumInterfaces;
	uint8_t bConfigurationValue;
	uint8_t iConfiguration;
	uint8_t bmAttributes;
	uint8_t bMaxPower;
} usb_configuration_descriptor_t;



typedef struct __attribute__((packed)) _USB_ENDPOINT_DESCRIPTOR{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bEndpointAddress;
	uint8_t bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t bInterval;
} usb_endpoint_descriptor_t;



#endif
