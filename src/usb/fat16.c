#include <pico/time.h>
#include <string.h>
#include <usb/fat16.h>
#include <usb/msc.h>



#define CLUSTER_SIZE 4096

#define MAX_ROOT_DIRECTORY_ENTRIES 256
#define ROOT_DIRECTORY_SECTORS (MAX_ROOT_DIRECTORY_ENTRIES*32/USB_MSC_SECTOR_SIZE)



static void _copy_padded(const char* str,uint8_t length,uint8_t* out){
	while (*str&&length){
		*out=*str;
		str++;
		out++;
		length--;
	}
	while (length){
		*out=' ';
		out++;
		length--;
	}
}



static void _init_directory_entry(fat16_directory_entry_t* entry,const char* name,uint32_t cluster,uint32_t size,uint16_t date,uint16_t time){
	_copy_padded(name,11,entry->name);
    entry->attributes=FAT16_ATTRIBUTE_READONLY|FAT16_ATTRIBUTE_ARCHIVE;
    entry->creation_time_fraction=0;
    entry->creation_time=time;
    entry->creation_date=date;
    entry->last_access_date=date;
    entry->cluster_hi=cluster>>16;
    entry->last_modified_time=time;
    entry->last_modified_date=date;
    entry->cluster_lo=cluster;
    entry->size=size;
}



void fat16_init(uint32_t size,const char* label,const fat16_file_descriptor_t* files,fat16_context_t* out){
	out->size=size;
	out->label=label;
	out->files=files;
	for (out->file_count=0;(files+out->file_count)->name[0];out->file_count++);
	out->serial_number=time_us_32();
	out->files_read=0;
	out->sector_count=size/USB_MSC_SECTOR_SIZE;
	out->sectors_per_fat=(size/CLUSTER_SIZE+USB_MSC_SECTOR_SIZE-1)*2/USB_MSC_SECTOR_SIZE;
}



void fat16_sector_callback(void* ctx,uint32_t lba,void* out){
	fat16_context_t* fat_ctx=ctx;
	memset(out,0,USB_MSC_SECTOR_SIZE);
	uint8_t* buffer=out;
	if (!lba){
		buffer[USB_MSC_SECTOR_SIZE-72]=fat_ctx->serial_number;
		buffer[USB_MSC_SECTOR_SIZE-71]=fat_ctx->serial_number>>8;
		buffer[USB_MSC_SECTOR_SIZE-70]=fat_ctx->serial_number>>16;
		buffer[USB_MSC_SECTOR_SIZE-69]=fat_ctx->serial_number>>24;
		buffer[USB_MSC_SECTOR_SIZE-62]=0x0e;
		buffer[USB_MSC_SECTOR_SIZE-58]=0x01;
		buffer[USB_MSC_SECTOR_SIZE-54]=(fat_ctx->sector_count-1)&0xff;
		buffer[USB_MSC_SECTOR_SIZE-53]=((fat_ctx->sector_count-1)>>8)&0xff;
		buffer[USB_MSC_SECTOR_SIZE-52]=((fat_ctx->sector_count-1)>>16)&0xff;
		buffer[USB_MSC_SECTOR_SIZE-51]=(fat_ctx->sector_count-1)>>24;
		buffer[USB_MSC_SECTOR_SIZE-2]=0x55;
		buffer[USB_MSC_SECTOR_SIZE-1]=0xaa;
		return;
	}
	if (lba==1){
		buffer[0]=0xeb;
		buffer[1]=0x3c;
		buffer[2]=0x90;
		buffer[3]='M';
		buffer[4]='S';
		buffer[5]='W';
		buffer[6]='I';
		buffer[7]='N';
		buffer[8]='4';
		buffer[9]='.';
		buffer[10]='1';
		buffer[11]=USB_MSC_SECTOR_SIZE&0xff;
		buffer[12]=USB_MSC_SECTOR_SIZE>>8;
		buffer[13]=CLUSTER_SIZE/USB_MSC_SECTOR_SIZE;
		buffer[14]=0x01;
		buffer[15]=0x00;
		buffer[16]=0x02;
		buffer[17]=MAX_ROOT_DIRECTORY_ENTRIES&0xff;
		buffer[18]=MAX_ROOT_DIRECTORY_ENTRIES>>8;
		buffer[19]=0x00;
		buffer[20]=0x00;
		buffer[21]=0xf8;
		buffer[22]=fat_ctx->sectors_per_fat&0xff;
		buffer[23]=fat_ctx->sectors_per_fat>>8;
		buffer[24]=0x01;
		buffer[25]=0x00;
		buffer[26]=0x01;
		buffer[27]=0x00;
		buffer[28]=0x01;
		buffer[29]=0x00;
		buffer[30]=0x00;
		buffer[31]=0x00;
		buffer[32]=(fat_ctx->sector_count-1)&0xff;
		buffer[33]=((fat_ctx->sector_count-1)>>8)&0xff;
		buffer[34]=((fat_ctx->sector_count-1)>>16)&0xff;
		buffer[35]=(fat_ctx->sector_count-1)>>24;
		buffer[36]=0x00;
		buffer[37]=0x00;
		buffer[38]=0x29;
		buffer[39]=fat_ctx->serial_number;
		buffer[40]=fat_ctx->serial_number>>8;
		buffer[41]=fat_ctx->serial_number>>16;
		buffer[42]=fat_ctx->serial_number>>24;
		_copy_padded(fat_ctx->label,11,buffer+43);
		buffer[54]='F';
		buffer[55]='A';
		buffer[56]='T';
		buffer[57]='1';
		buffer[58]='6';
		buffer[59]=' ';
		buffer[60]=' ';
		buffer[61]=' ';
		buffer[62]=0xeb;
		buffer[63]=0xfe;
		return;
	}
	lba-=2;
	if (lba<fat_ctx->sectors_per_fat*2){
		if (lba>=fat_ctx->sectors_per_fat){
			lba-=fat_ctx->sectors_per_fat;
		}
		if (!lba){
			buffer[0]=0xf8;
			buffer[1]=0xff;
			buffer[2]=0xff;
			buffer[3]=0xff;
			for (uint32_t i=0;i<fat_ctx->file_count;i++){
				buffer[(i<<1)+4]=0xff;
				buffer[(i<<1)+5]=0xff;
			}
		}
		return;
	}
	lba-=fat_ctx->sectors_per_fat*2;
	if (lba<ROOT_DIRECTORY_SECTORS){
		if (!lba){
			fat16_directory_entry_t* entry=(fat16_directory_entry_t*)buffer;
			_copy_padded(fat_ctx->label,11,entry->name);
			entry->attributes=FAT16_ATTRIBUTE_VOLUME_LABEL|FAT16_ATTRIBUTE_ARCHIVE;
			for (uint32_t i=0;i<fat_ctx->file_count;i++){
				const fat16_file_descriptor_t* file=fat_ctx->files+i;
				_init_directory_entry(entry+i+1,file->name,i+2,file->size,file->date,file->time);
			}
		}
		return;
	}
	lba-=ROOT_DIRECTORY_SECTORS;
	uint32_t cluster=lba>>3;
	uint32_t cluster_offset=lba-(cluster<<3);
	const fat16_file_descriptor_t* file=fat_ctx->files+cluster;
	if (cluster<fat_ctx->file_count&&cluster_offset*USB_MSC_SECTOR_SIZE<file->size){
		fat_ctx->files_read=1;
		uint32_t length=file->size-cluster_offset*USB_MSC_SECTOR_SIZE;
		memcpy(buffer,file->data+cluster_offset*USB_MSC_SECTOR_SIZE,(length>USB_MSC_SECTOR_SIZE?USB_MSC_SECTOR_SIZE:length));
	}
}
