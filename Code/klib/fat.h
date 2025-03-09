#include "types.h"

#define FAT_LOAD_SUCCESS 0

// the BIOS Parameter Block (in Volume Boot Record)
//https://wiki.osdev.org/FAT#FAT_32_and_exFAT
//https://stackoverflow.com/questions/20798534/fat32-size-of-the-root-directory
typedef struct fat_extbpb_32
{
	//extended fat32 stuff
	unsigned int		table_size_32;
	unsigned short		extended_flags;
	unsigned short		fat_version;
	unsigned int		root_cluster;
	unsigned short		fat_info;
	unsigned short		backup_BS_sector;
	unsigned char 		reserved_0[12];
	unsigned char		drive_number;
	unsigned char 		reserved_1;
	unsigned char		boot_signature;
	unsigned int 		volume_id;
	unsigned char		volume_label[11];
	unsigned char		fat_type_label[8];

}__attribute__((packed)) fat_extbpb_32_t;

typedef struct fat_extbpb_16
{
	//extended fat12 and fat16 stuff
	unsigned char		bios_drive_num;
	unsigned char		reserved1;
	unsigned char		boot_signature;
	unsigned int		volume_id;
	unsigned char		volume_label[11];
	unsigned char		fat_type_label[8];
	
}__attribute__((packed)) fat_extbpb_16_t;

typedef struct fat_bpb
{
	unsigned char 		bootjmp[3];
	unsigned char 		oem_name[8];
	unsigned short 	    bytes_per_sector;
	unsigned char		sectors_per_cluster;
	unsigned short		reserved_sector_count;
	unsigned char		table_count;
	unsigned short		root_entry_count;
	unsigned short		total_sectors_16;
	unsigned char		media_type;
	unsigned short		table_size_16;
	unsigned short		sectors_per_track;
	unsigned short		head_side_count;
	unsigned int 		hidden_sector_count;
	unsigned int 		total_sectors_32;
	
	//this will be cast to it's specific type once the driver actually knows what type of FAT this is.
	unsigned char		extended_section[54];
	
}__attribute__((packed)) fat_bpb_t;

// directory entry structure
typedef struct {
    char            name[8];
    char            ext[3];
    char            attr[9];
    unsigned short  ch;
    unsigned int    attr2;
    unsigned short  cl;
    unsigned int    size;
} __attribute__((packed)) fatdir_std_t;

typedef struct {
    uint8_t         order;
    uint16_t        name_1[5];
    uint8_t         attr;
    uint8_t			long_entry_type;
	uint8_t 		checksum;
	uint16_t 		name_2[6];
	uint16_t 		unused;
	uint16_t 		name_3[2];
} __attribute__((packed)) fatdir_lfn_t;

//Partition entry structure
typedef struct {
	uint8_t 		status;
	uint8_t			CHS_first_sector[3];
	uint8_t			partition_type;
	uint8_t			CHS_last_sector[3];
	uint8_t			LBA_first_sector[4];
	uint8_t 		number_sectors[4];
} __attribute__((packed)) partition_entry_t;

int fat_loadpartition(void);
void fat_listdirectory(void);