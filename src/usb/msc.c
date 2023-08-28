#include <string.h>
#include <usb/msc.h>
#include <usb/usb.h>



static void _copy_padded(const char* str,uint8_t length,uint8_t* out){
	while (*str&&length){
		*out=*str;
		str++;
		out++;
		length--;
	}
	while (length){
		*out=0;
		out++;
		length--;
	}
}



static void _prepare_status(usb_msc_handler_context_t* msc_ctx,const usb_scsi_command_t* command,uint32_t length,_Bool success){
	msc_ctx->status.signature=0x53425355;
	msc_ctx->status.tag=command->tag;
	msc_ctx->status.residue=command->size-length;
	msc_ctx->status.status=(success?USB_MSC_STATUS_PASSED:USB_MSC_STATUS_FAILED);
	msc_ctx->send_status=1;
}



static void _send_status(usb_msc_handler_context_t* msc_ctx){
	usb_transfer(msc_ctx->d2h_endpoint,&(msc_ctx->status),sizeof(usb_scsi_status_t));
	msc_ctx->send_status=0;
}



static void _send_response(usb_msc_handler_context_t* msc_ctx,const usb_scsi_command_t* command,const void* buffer,uint32_t length){
	if (length>command->size){
		length=command->size;
	}
	_prepare_status(msc_ctx,command,length,1);
	usb_transfer(msc_ctx->d2h_endpoint,buffer,length);
}



static void _chunk_stream_callback(void* ctx,uint32_t offset,uint8_t length,void* out){
	usb_msc_handler_context_t* msc_ctx=ctx;
	uint32_t lba=msc_ctx->current_lba_offset+offset/USB_MSC_SECTOR_SIZE;
	if (msc_ctx->current_sector_lba!=lba){
		msc_ctx->current_sector_lba=lba;
		msc_ctx->sector_callback(msc_ctx->sector_callback_ctx,lba,msc_ctx->current_sector);
	}
	memcpy(out,msc_ctx->current_sector+(offset%USB_MSC_SECTOR_SIZE),length);
}



void usb_msc_init(uint8_t h2d_endpoint,uint8_t d2h_endpoint,uint32_t size,const char* vendor,const char* product,const char* revision,usb_msc_sector_callback_t sector_callback,void* sector_callback_ctx,usb_msc_handler_context_t* ctx){
	ctx->h2d_endpoint=h2d_endpoint;
	ctx->d2h_endpoint=d2h_endpoint;
	ctx->size=size;
	ctx->vendor=vendor;
	ctx->product=product;
	ctx->revision=revision;
	ctx->send_status=0;
	ctx->waiting_for_write=0;
	ctx->drive_acknowledged=0;
	ctx->current_sector_lba=0xffffffff;
	ctx->sector_callback=sector_callback;
	ctx->sector_callback_ctx=sector_callback_ctx;
	usb_transfer(h2d_endpoint,NULL,64);
}



void usb_msc_h2d_handler(void* ctx,const uint8_t* buffer,uint16_t length){
	usb_msc_handler_context_t* msc_ctx=ctx;
	if (msc_ctx->waiting_for_write){
		_send_status(msc_ctx);
		msc_ctx->waiting_for_write=0;
		msc_ctx->drive_acknowledged=1;
		return;
	}
	if (length<15){
		return;
	}
	const usb_scsi_command_t* command=(const usb_scsi_command_t*)buffer;
	if (command->signature!=0x43425355||!command->length){
		return;
	}
	if (command->command[0]==USB_SCSI_CMD_TEST_UNIT_READY){
		_prepare_status(msc_ctx,command,0,1);
		_send_status(msc_ctx);
	}
	else if (command->command[0]==USB_SCSI_CMD_INQUIRY){
		uint8_t buffer[36]={
			0x00,
			0x80, // is_removable
			0x02,
			0x02,
			0x1f, // additional_length
			0x00,
			0x00,
			0x00
		};
		_copy_padded(msc_ctx->vendor,8,buffer+8);
		_copy_padded(msc_ctx->product,16,buffer+16);
		_copy_padded(msc_ctx->revision,4,buffer+32);
		_send_response(msc_ctx,command,buffer,36);
	}
	else if (command->command[0]==USB_SCSI_CMD_MODE_SENSE_6){
		uint8_t buffer[4]={
			0x03,
			0x00,
			0x00,
			0x00
		};
		_send_response(msc_ctx,command,buffer,4);
	}
	else if (command->command[0]==USB_SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL){
		_prepare_status(msc_ctx,command,0,1);
		_send_status(msc_ctx);
	}
	else if (command->command[0]==USB_SCSI_CMD_READ_CAPACITY_10){
		uint32_t max_lba=msc_ctx->size/USB_MSC_SECTOR_SIZE-1;
		uint8_t buffer[8]={
			max_lba>>24,
			max_lba>>16,
			max_lba>>8,
			max_lba,
			USB_MSC_SECTOR_SIZE>>24,
			(USB_MSC_SECTOR_SIZE>>16)&0xff,
			(USB_MSC_SECTOR_SIZE>>8)&0xff,
			USB_MSC_SECTOR_SIZE&0xff,
		};
		_send_response(msc_ctx,command,buffer,8);
	}
	else if (command->command[0]==USB_SCSI_CMD_REQUEST_SENSE){
		uint8_t buffer[18]={
			0xf0,
			0x00,
			0x00, // SCSI_SENSE_NONE
			0x00,
			0x00,
			0x00,
			0x00,
			0x0a, // sense_len
			0x00,
			0x00,
			0x00,
			0x00,
			0x00, // sense_code
			0x00, // sense_qualifier
			0x00,
			0x00,
			0x00,
			0x00
		};
		_send_response(msc_ctx,command,buffer,18);
	}
	else if (command->command[0]==USB_SCSI_CMD_READ_10||command->command[0]==USB_SCSI_CMD_WRITE_10){
		if (command->length<9){
_fail:
			_prepare_status(msc_ctx,command,0,0);
			_send_status(msc_ctx);
			return;
		}
		uint32_t lba=(command->command[2]<<24)|(command->command[3]<<16)|(command->command[4]<<8)|command->command[5];
		uint16_t block_count=(command->command[7]<<8)|command->command[8];
		if ((lba+block_count)*USB_MSC_SECTOR_SIZE>msc_ctx->size){
			goto _fail;
		}
		uint32_t transfer_length=block_count*USB_MSC_SECTOR_SIZE;
		if (transfer_length>command->size){
			transfer_length=command->size;
		}
		_prepare_status(msc_ctx,command,transfer_length,1);
		if (!transfer_length){
			_send_status(msc_ctx);
		}
		else if (command->command[0]==USB_SCSI_CMD_READ_10){
			msc_ctx->current_lba_offset=lba;
			usb_stream_t stream={
				_chunk_stream_callback,
				msc_ctx
			};
			usb_transfer(msc_ctx->d2h_endpoint|USB_TRANSFER_FLAG_STREAM,&stream,transfer_length);
		}
		else{
			msc_ctx->waiting_for_write=transfer_length;
			usb_transfer(msc_ctx->h2d_endpoint,NULL,transfer_length);
		}
	}
}



void usb_msc_d2h_handler(void* ctx,const uint8_t* buffer,uint16_t length){
	usb_msc_handler_context_t* msc_ctx=ctx;
	if (msc_ctx->send_status){
		_send_status(msc_ctx);
	}
	else{
		usb_transfer(msc_ctx->h2d_endpoint,NULL,64);
	}
}
