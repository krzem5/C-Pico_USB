#include <hardware/gpio.h>
#include <pico/bootrom.h>
#include <pico/time.h>
#include <usb/common.h>
#include <usb/fat16.h>
#include <usb/hid.h>
#include <usb/hid_keyboard.h>
#include <usb/msc.h>
#include <usb/usb.h>



static fat16_context_t _fat16_ctx;
static usb_msc_handler_context_t _msc_ctx;



static void ep1_h2d_handler(void* ctx,const uint8_t* buffer,uint16_t length){
	if (length==1&&buffer[0]=='q'){
		reset_usb_boot(0,0);
	}
	usb_transfer(USB_ENDPOINT_INDEX(1,0),buffer,length);
}



static void ep1_d2h_handler(void* ctx,const uint8_t* buffer,uint16_t length){
	usb_transfer(USB_ENDPOINT_INDEX(1,1),NULL,64);
}



static const char text[]="Hello!\n";



static const fat16_file_descriptor_t _fat16_file_descriptors[]={
	{
		"GREETING",
		FAT16_ATTRIBUTE_READONLY|FAT16_ATTRIBUTE_ARCHIVE,
		FAT16_DATE(2023,8,28),
		FAT16_TIME(14,27,6),
		text,
		sizeof(text)-1
	},
	{{0}}
};



static const uint8_t _usb_keyboard_hid_report[]={
	USB_HID_KEYBOARD_REPORT
};



static const uint8_t _usb_device_configuration[]={
	USB_DEVICE_INTERFACE_DESCRIPTOR(0,2,0xff,0,0),
	USB_DEVICE_ENDPOINT_DESCRIPTOR(1,0,USB_TRANSFER_TYPE_BULK,64,0),
	USB_DEVICE_ENDPOINT_DESCRIPTOR(1,1,USB_TRANSFER_TYPE_BULK,64,0),
	USB_DEVICE_INTERFACE_DESCRIPTOR(1,1,0x03,0x01,0x01),
	USB_DEVICE_HID_DESCRIPTOR(1,sizeof(_usb_keyboard_hid_report)),
	USB_DEVICE_ENDPOINT_DESCRIPTOR(2,0,USB_TRANSFER_TYPE_INTERRUPT,8,10),
	USB_DEVICE_INTERFACE_DESCRIPTOR(2,2,0x08,0x06,0x50),
	USB_DEVICE_ENDPOINT_DESCRIPTOR(3,0,USB_TRANSFER_TYPE_BULK,64,0),
	USB_DEVICE_ENDPOINT_DESCRIPTOR(3,1,USB_TRANSFER_TYPE_BULK,64,0),
};



static const char*const _usb_device_string_table[]={
	"<STRING_MN>",
	"<STRING_PR>",
	"<STRING_SN>",
	NULL
};



static const usb_hid_report_t _usb_device_hid_report_table[]={
	{NULL,0},
	{_usb_keyboard_hid_report,sizeof(_usb_keyboard_hid_report)},
	{NULL,0},
};



static const usb_device_config_t _usb_device_config={
	USB_DECLARE_DEVICE_DESCRIPTOR(0x0000,0x0001,0x0000,1,2,3),
	USB_DECLARE_CONFIGURATION_DESCRIPTOR(_usb_device_configuration,sizeof(_usb_device_configuration)),
	USB_DECLARE_STRING_TABLE(_usb_device_string_table),
	USB_DECLARE_ENDPOINT_HANDLER(1,1,ep1_h2d_handler,NULL),
	USB_DECLARE_ENDPOINT_HANDLER(1,0,ep1_d2h_handler,NULL),
	USB_DECLARE_ENDPOINT_HANDLER(3,1,usb_msc_h2d_handler,&_msc_ctx),
	USB_DECLARE_ENDPOINT_HANDLER(3,0,usb_msc_d2h_handler,&_msc_ctx),
	USB_DECLARE_HID_REPORT_TABLE(_usb_device_hid_report_table),
};



int main(void){
	gpio_init(PICO_DEFAULT_LED_PIN);
	gpio_set_dir(PICO_DEFAULT_LED_PIN,GPIO_OUT);
	gpio_put(PICO_DEFAULT_LED_PIN,1);
	usb_init(&_usb_device_config);
	while (!(usb_get_device_flags()&USB_DEVICE_FLAG_CONFIGURED)){
		tight_loop_contents();
	}
	fat16_init(134217728,"LABEL",_fat16_file_descriptors,&_fat16_ctx);
	usb_transfer(USB_ENDPOINT_INDEX(1,1),NULL,64);
	usb_msc_init(USB_ENDPOINT_INDEX(3,1),USB_ENDPOINT_INDEX(3,0),134217728,"VENDOR","PRODUCT","REV",fat16_sector_callback,&_fat16_ctx,&_msc_ctx);
	while (!_msc_ctx.drive_acknowledged){
		tight_loop_contents();
	}
	usb_hid_keyboard_type_keys(USB_ENDPOINT_INDEX(2,0),"Drive mounted!",0);
	while (1){
		tight_loop_contents();
	}
	reset_usb_boot(0,0);
}
