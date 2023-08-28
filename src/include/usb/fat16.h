#ifndef _USB_FAT16_H_
#define _USB_FAT16_H_ 1
#include <stdint.h>



#define FAT16_DATE(year,month,day) ((((year)-1980)<<9)|((month)<<5)|(day))
#define FAT16_TIME(hour,minute,second) (((hour)<<11)|((minute)<<5)|((second)>>1))

#define FAT16_ATTRIBUTE_READONLY 0x01
#define FAT16_ATTRIBUTE_HIDDEN 0x02
#define FAT16_ATTRIBUTE_SYSTEM 0x04
#define FAT16_ATTRIBUTE_VOLUME_LABEL 0x08
#define FAT16_ATTRIBUTE_DIR 0x10
#define FAT16_ATTRIBUTE_ARCHIVE 0x20



typedef struct __attribute__((packed)) _FAT16_DIRECTORY_ENTRY{
	uint8_t name[11];
	uint8_t attributes;
	uint8_t _padding;
	uint8_t creation_time_fraction;
	uint16_t creation_time;
	uint16_t creation_date;
	uint16_t last_access_date;
	uint16_t cluster_hi;
	uint16_t last_modified_time;
	uint16_t last_modified_date;
	uint16_t cluster_lo;
	uint32_t size;
} fat16_directory_entry_t;



typedef struct _FAT16_FILE_DESCRIPTOR{
	char name[11];
	uint8_t attributes;
	uint16_t date;
	uint16_t time;
	const char* data;
	uint32_t size;
} fat16_file_descriptor_t;



typedef struct _FAT16_CONTEXT{
	uint32_t size;
	const char* label;
	const fat16_file_descriptor_t* files;
	uint32_t file_count;
	uint32_t serial_number;
	volatile _Bool files_read;
	uint32_t sector_count;
	uint16_t sectors_per_fat;
} fat16_context_t;



void fat16_init(uint32_t size,const char* label,const fat16_file_descriptor_t* files,fat16_context_t* out);



void fat16_sector_callback(void* ctx,uint32_t lba,void* out);



#endif
