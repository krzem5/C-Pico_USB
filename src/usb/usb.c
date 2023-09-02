#include <hardware/irq.h>
#include <hardware/regs/usb.h>
#include <hardware/resets.h>
#include <hardware/structs/usb.h>
#include <string.h>
#include <usb/common.h>
#include <usb/usb.h>



#define usb_hw_set ((usb_hw_t*)hw_set_alias_untyped(usb_hw))
#define usb_hw_clear ((usb_hw_t*)hw_clear_alias_untyped(usb_hw))



static uint8_t _usb_device_ep0_buffer[128];
static usb_device_t _usb_device;



static inline volatile uint32_t* _get_endpoint_control(uint8_t index){
	if (index<2){
		return NULL;
	}
	struct usb_device_dpram_ep_ctrl* ep_ctrl=usb_dpram->ep_ctrl+(index>>1)-1;
	return ((index&1)?&(ep_ctrl->out):&(ep_ctrl->in));
}



static inline volatile uint32_t* _get_buffer_control(uint8_t index){
	struct usb_device_dpram_ep_buf_ctrl* ep_buf_ctrl=usb_dpram->ep_buf_ctrl+(index>>1);
	return ((index&1)?&(ep_buf_ctrl->out):&(ep_buf_ctrl->in));
}



static inline volatile uint8_t* _get_data_buffer(uint8_t index){
	return (index>1?usb_dpram->epx_data+((index-2)<<6):usb_dpram->ep0_buf_a);
}



static void _continue_transfer(uint8_t index){
	volatile usb_endpoint_t* endpoint=_usb_device.endpoints+index;
	uint8_t buffer_length=(endpoint->length>endpoint->max_packet_length?endpoint->max_packet_length:endpoint->length);
	endpoint->length-=buffer_length;
	uint32_t out=buffer_length|USB_BUF_CTRL_AVAIL|((_usb_device.endpoint_pids&(1<<index))?USB_BUF_CTRL_DATA1_PID:USB_BUF_CTRL_DATA0_PID);
	_usb_device.endpoint_pids^=1<<index;
	if (!(index&1)){
		if (endpoint->is_stream){
			endpoint->stream_callback(endpoint->stream_ctx,endpoint->offset,buffer_length,(void*)_get_data_buffer(index));
		}
		else{
			memcpy((void*)_get_data_buffer(index),endpoint->buffer+endpoint->offset,buffer_length);
		}
		endpoint->offset+=buffer_length;
		out|=USB_BUF_CTRL_FULL;
		if (!endpoint->length){
			out|=USB_BUF_CTRL_LAST;
		}
	}
	*_get_buffer_control(index)=out;
}



static void _usb_irq(void){
	uint32_t status=usb_hw->ints;
	if (status&USB_INTS_BUFF_STATUS_BITS){
		for (uint8_t i=0;i<(USB_NUM_ENDPOINTS<<1);i++){
			if (!(usb_hw->buf_status&(1<<i))){
				continue;
			}
			usb_hw_clear->buf_status=1<<i;
			if ((_usb_device.endpoints+i)->length){
				_continue_transfer(i);
				continue;
			}
			if (!i){
				if (_usb_device.flags&USB_DEVICE_FLAG_SWITCH_ADDRESS){
					_usb_device.flags&=~USB_DEVICE_FLAG_SWITCH_ADDRESS;
					usb_hw->dev_addr_ctrl=_usb_device.new_address;
				}
				usb_transfer(USB_ENDPOINT_INDEX(0,1),NULL,0);
				continue;
			}
			if (i==1){
				continue;
			}
			if (!(_usb_device.config->endpoints+i)->handler){
				continue;
			}
			(_usb_device.config->endpoints+i)->handler((_usb_device.config->endpoints+i)->ctx,(const uint8_t*)_get_data_buffer(i),(*_get_buffer_control(i))&USB_BUF_CTRL_LEN_MASK);
		}
	}
	if (status&USB_INTS_SETUP_REQ_BITS){
		usb_hw_clear->sie_status=USB_SIE_STATUS_SETUP_REC_BITS;
		const volatile usb_setup_packet_t* packet=(const volatile usb_setup_packet_t*)(usb_dpram->setup_packet);
		_usb_device.endpoint_pids|=1;
		if ((packet->bmRequestType&USB_ENDPOIND_DIRECTION_IN)&&packet->bRequest==USB_REQUEST_GET_DESCRIPTOR){
			uint8_t type=packet->wValue>>8;
			uint8_t index=packet->wValue;
			if (type==USB_DESCRIPTOR_TYPE_DEVICE){
				usb_device_descriptor_t device_descriptor={
					sizeof(usb_device_descriptor_t),
    				USB_DESCRIPTOR_TYPE_DEVICE,
					0x0110,
					0,
					0,
					0,
					64,
					_usb_device.config->vendor_id,
					_usb_device.config->product_id,
					_usb_device.config->revision_id,
					_usb_device.config->manufacturer_string_id,
					_usb_device.config->product_string_id,
					_usb_device.config->serial_string_id,
					1
				};
				usb_transfer(USB_ENDPOINT_INDEX(0,0),&device_descriptor,sizeof(usb_device_descriptor_t));
			}
			else if (type==USB_DESCRIPTOR_TYPE_CONFIG){
				usb_configuration_descriptor_t configuration_descriptor={
					sizeof(usb_configuration_descriptor_t),
					USB_DESCRIPTOR_TYPE_CONFIG,
					_usb_device.config->configuration_descriptor_length+sizeof(usb_configuration_descriptor_t),
					_usb_device.configuration_descriptor_interface_count,
					1,
					0,
					0xc0,
					0x32
				};
				if (packet->wLength<configuration_descriptor.wTotalLength){
					usb_transfer(USB_ENDPOINT_INDEX(0,0),&configuration_descriptor,sizeof(usb_configuration_descriptor_t));
				}
				else{
					memcpy(_usb_device_ep0_buffer,&configuration_descriptor,sizeof(usb_configuration_descriptor_t));
					memcpy(_usb_device_ep0_buffer+sizeof(usb_configuration_descriptor_t),_usb_device.config->configuration_descriptor,_usb_device.config->configuration_descriptor_length);
					usb_transfer(USB_ENDPOINT_INDEX(0,0),_usb_device_ep0_buffer,configuration_descriptor.wTotalLength);
				}
			}
			else if (type==USB_DESCRIPTOR_TYPE_STRING){
				uint16_t length=2;
				_usb_device_ep0_buffer[1]=USB_DESCRIPTOR_TYPE_STRING;
				if (!index){
					_usb_device_ep0_buffer[2]=0x09;
					_usb_device_ep0_buffer[3]=0x04;
					length+=2;
				}
				else{
					index--;
					const char* str=(index>=_usb_device.string_table_length?"???":_usb_device.config->string_table[index]);
					while (*str&&length<128){
						_usb_device_ep0_buffer[length]=*str;
						_usb_device_ep0_buffer[length+1]=0;
		 				str++;
						length+=2;
					}
				}
				_usb_device_ep0_buffer[0]=length;
				usb_transfer(USB_ENDPOINT_INDEX(0,0),_usb_device_ep0_buffer,(length>packet->wLength?packet->wLength:length));
			}
			else if (type==USB_DESCRIPTOR_TYPE_HID_REPORT){
				const usb_hid_report_t* report=_usb_device.config->hid_report_table+packet->wIndex;
				if (packet->wIndex<_usb_device.configuration_descriptor_interface_count&&_usb_device.config->hid_report_table&&report->report){
					usb_transfer(USB_ENDPOINT_INDEX(0,0),report->report,(report->length>packet->wLength?packet->wLength:report->length));
				}
			}
		}
		else if ((packet->bmRequestType&USB_ENDPOIND_DIRECTION_IN)&&packet->bRequest==USB_REQUEST_MSC_GET_MAX_LUN){
			uint8_t value=0;
			usb_transfer(USB_ENDPOINT_INDEX(0,0),&value,1);
		}
		else if (!(packet->bmRequestType&USB_ENDPOIND_DIRECTION_IN)){
			if (packet->bRequest==USB_REQUEST_SET_ADDRESS){
				_usb_device.new_address=packet->wValue&0xff;
				_usb_device.flags|=USB_DEVICE_FLAG_SWITCH_ADDRESS;
			}
			else if (packet->bRequest==USB_REQUEST_SET_CONFIGURATION){
				_usb_device.flags|=USB_DEVICE_FLAG_CONFIGURED;
			}
			usb_transfer(USB_ENDPOINT_INDEX(0,0),NULL,0);
		}
	}
	if (status&USB_INTS_BUS_RESET_BITS){
		usb_hw_clear->sie_status=USB_SIE_STATUS_BUS_RESET_BITS;
		usb_hw->dev_addr_ctrl=0;
		_usb_device.flags=0;
	}
}



void usb_init(const usb_device_config_t* config){
	reset_block(RESETS_RESET_USBCTRL_BITS);
	unreset_block_wait(RESETS_RESET_USBCTRL_BITS);
	memset(usb_dpram,0,sizeof(usb_device_dpram_t));
	_usb_device.config=config;
	if (!config->string_table){
		_usb_device.string_table_length=0;
	}
	else{
		for (_usb_device.string_table_length=0;config->string_table[_usb_device.string_table_length];_usb_device.string_table_length++);
	}
	_usb_device.flags=0;
	_usb_device.endpoint_pids=1;
	for (uint8_t i=0;i<(USB_NUM_ENDPOINTS<<1);i++){
		(_usb_device.endpoints+i)->transfer_type=0;
		(_usb_device.endpoints+i)->max_packet_length=0;
		(_usb_device.endpoints+i)->buffer=NULL;
		(_usb_device.endpoints+i)->length=0;
	}
	_usb_device.endpoints->max_packet_length=64;
	(_usb_device.endpoints+1)->max_packet_length=64;
	_usb_device.configuration_descriptor_interface_count=0;
	for (uint16_t i=0;i<config->configuration_descriptor_length;i+=config->configuration_descriptor[i]){
		if (config->configuration_descriptor[i+1]==USB_DESCRIPTOR_TYPE_INTERFACE){
			_usb_device.configuration_descriptor_interface_count++;
		}
		else if (config->configuration_descriptor[i+1]==USB_DESCRIPTOR_TYPE_ENDPOINT){
			const usb_endpoint_descriptor_t* endpoint_desctiptor=(const usb_endpoint_descriptor_t*)(config->configuration_descriptor+i);
			uint8_t index=(endpoint_desctiptor->bEndpointAddress<<1)|(!(endpoint_desctiptor->bEndpointAddress&USB_ENDPOIND_DIRECTION_IN));
			(_usb_device.endpoints+index)->transfer_type=endpoint_desctiptor->bmAttributes;
			(_usb_device.endpoints+index)->max_packet_length=endpoint_desctiptor->wMaxPacketSize;
		}
	}
	irq_add_shared_handler(USBCTRL_IRQ,_usb_irq,PICO_SHARED_IRQ_HANDLER_HIGHEST_ORDER_PRIORITY);
	irq_set_enabled(USBCTRL_IRQ,1);
	usb_hw->muxing=USB_USB_MUXING_TO_PHY_BITS|USB_USB_MUXING_SOFTCON_BITS;
	usb_hw->pwr=USB_USB_PWR_VBUS_DETECT_BITS|USB_USB_PWR_VBUS_DETECT_OVERRIDE_EN_BITS;
	usb_hw->main_ctrl=USB_MAIN_CTRL_CONTROLLER_EN_BITS;
	usb_hw->sie_ctrl=USB_SIE_CTRL_EP0_INT_1BUF_BITS;
	usb_hw->inte=USB_INTS_BUFF_STATUS_BITS|USB_INTS_BUS_RESET_BITS|USB_INTS_SETUP_REQ_BITS;
	for (uint8_t i=0;i<(USB_NUM_ENDPOINTS<<1);i++){
		if ((_usb_device.endpoints+i)->transfer_type){
			*_get_endpoint_control(i)=EP_CTRL_ENABLE_BITS|EP_CTRL_INTERRUPT_PER_BUFFER|((_usb_device.endpoints+i)->transfer_type<<EP_CTRL_BUFFER_TYPE_LSB)|(((uint32_t)_get_data_buffer(i))-((uint32_t)usb_dpram));
		}
	}
	usb_hw_set->sie_ctrl=USB_SIE_CTRL_PULLUP_EN_BITS;
}



void usb_transfer(uint8_t index,const void* buffer,uint32_t length){
	_Bool is_stream=!!(index&USB_TRANSFER_FLAG_STREAM);
	index&=~USB_TRANSFER_FLAG_STREAM;
	volatile usb_endpoint_t* endpoint=_usb_device.endpoints+index;
	if (endpoint->length){
		return;
	}
	if (is_stream){
		const usb_stream_t* stream=buffer;
		endpoint->is_stream=1;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
		endpoint->stream_callback=stream->callback;
		endpoint->stream_ctx=stream->ctx;
#pragma GCC diagnostic pop
	}
	else{
		endpoint->is_stream=0;
		endpoint->buffer=buffer;
	}
	endpoint->offset=0;
	endpoint->length=length;
	_continue_transfer(index);
}



uint8_t usb_get_device_flags(void){
	return _usb_device.flags;
}
