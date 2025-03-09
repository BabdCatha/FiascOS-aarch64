#include "fat.h"
#include "sd.h"
#include "display.h"

uint8_t mbr[512];
uint8_t bpb[512];

fat_bpb_t* fat_bpb;

int fat_loadpartition(void){

    int active_partition = -1;
    uint32_t LBA_first_sector = -1;
    uint32_t LBA_last_sector = -1;
    uint32_t number_sectors = -1;

    //Reading the bpb from sector 0 of the SD card
    sd_readblock(0x00000000, mbr, 1);
    if(mbr[511] != 0xaa || mbr[510] != 0x55){
        tty_printf("[FAT] Error: MBR not found at sector 0\n");
        return -1;
    }else{
        tty_printf("[FAT] MBR loaded\n");
    }

    partition_entry_t* partitions = (partition_entry_t*)&mbr[0x1be];

    //We loop over the 4 possible partitions, to try to identify the one to boot from
    int i = 0;
    for(i = 0; i < 4; i++){
        switch(partitions[i].partition_type){
            case 0x00:
                tty_printf("[FAT] Found no partition at position %d", i);
                break;
            case 0x0c:
                tty_printf("[FAT] Found LBA FAT32 partition at position %d", i);
                break;
            default:
                tty_printf("[FAT] Found partition of type %x at position %d", partitions[i].partition_type, i);
                break;
        }

        //Checking if the partition is marked as active
        if(partitions[i].status >= 0x80){
            tty_printf(" (active)\n");
            if(active_partition != -1){
                tty_printf("[FAT] Warning: several active partitions detected !");
            }else{
                active_partition = i;
            }
        }else{
            tty_printf("\n");
        }
        
    }

    if(active_partition != -1){
        tty_printf("[FAT] Loading partition %d\n", active_partition);
    }else{
        tty_printf("[FAT] No active partition found, defaulting to partition 0\n");
        active_partition = 0;
    }

    //Finding the address of the first sector of the partition
    //The array magic has to be done as the packed structure does not align the LBA_first_sector
    //field with a 32-bit boundary. This causes crashes when trying to access the affected fields
    //TODO: This should have been fixed with the -mstrict-align compiler flag.
    if(partitions[active_partition].LBA_first_sector != 0x00000000){
        //Using the LBA stored data
        tty_printf("[FAT] Using LBA addressing\n");
        LBA_first_sector = (partitions[active_partition].LBA_first_sector[0]) +
                           (partitions[active_partition].LBA_first_sector[1] << 8) +
                           (partitions[active_partition].LBA_first_sector[2] << 16) +
                           (partitions[active_partition].LBA_first_sector[3] << 24);
        number_sectors = (partitions[active_partition].number_sectors[0]) +
                         (partitions[active_partition].number_sectors[1] << 8) +
                         (partitions[active_partition].number_sectors[2] << 16) +
                         (partitions[active_partition].number_sectors[3] << 24);
        LBA_last_sector = LBA_first_sector + 512 * number_sectors; //TODO: Is it really always 512 ??

    }else{
        //TODO: Handle CHS stored addresses
        tty_printf("[FAT] Error: Can't handle CHS-only adressing\n");
        return(-1);
    }

    tty_printf("[FAT] First sector address: %x\n", LBA_first_sector);
    tty_printf("[FAT] Last sector address: %x\n", LBA_last_sector);
    tty_printf("[FAT] Total partition size: %d bytes\n", 512 * number_sectors);

    //Loading the BPB of the partition being loaded
    sd_readblock(LBA_first_sector, bpb, 1);

    //Populating the struct describing the basic FAT BPB
    fat_bpb = (fat_bpb_t*)bpb;

    tty_printf("[FAT] OEM Identifier: %c%c%c%c%c%c%c%c\n", fat_bpb->oem_name[0],
                                                           fat_bpb->oem_name[1], 
                                                           fat_bpb->oem_name[2], 
                                                           fat_bpb->oem_name[3], 
                                                           fat_bpb->oem_name[4], 
                                                           fat_bpb->oem_name[5], 
                                                           fat_bpb->oem_name[6], 
                                                           fat_bpb->oem_name[7]);

    //Calculating important pieces of information
    int total_sectors = (fat_bpb->total_sectors_16 == 0) ? fat_bpb->total_sectors_32 : fat_bpb->total_sectors_16;
    int fat_size = (fat_bpb->table_size_16 == 0) ? ((fat_extbpb_32_t*)(fat_bpb->extended_section))->table_size_32 : fat_bpb->table_size_16;
    int root_dir_sectors = ((fat_bpb->root_entry_count * 32) + (fat_bpb->bytes_per_sector - 1)) / fat_bpb->bytes_per_sector;
    int first_data_sector = fat_bpb->reserved_sector_count + (fat_bpb->table_count * fat_size) + root_dir_sectors;
    int first_fat_sector = fat_bpb->reserved_sector_count;
    int data_sectors = total_sectors - (fat_bpb->reserved_sector_count + (fat_bpb->table_count * fat_size) + root_dir_sectors);
    int total_clusters = data_sectors / fat_bpb->sectors_per_cluster;
    int bytes_per_sector = fat_bpb->bytes_per_sector;
    int sectors_per_cluster = fat_bpb->sectors_per_cluster;
    int cluster_size = bytes_per_sector*sectors_per_cluster;

    tty_printf("[FAT] Total number of sectors: %d\n", total_sectors);
    tty_printf("[FAT] Size of the FAT: %d\n", fat_size);
    tty_printf("[FAT] Sectors in the root directory: %d\n", root_dir_sectors);
    tty_printf("[FAT] First data sector: 0x%x\n", first_data_sector);
    tty_printf("[FAT] First FAT sector: 0x%x\n", first_fat_sector);
    tty_printf("[FAT] Number of data sectors: %d\n", data_sectors);
    tty_printf("[FAT] Total number of clusters: %d\n", total_clusters);
    tty_printf("[FAT] Bytes per sector: %d\n", bytes_per_sector);
    tty_printf("[FAT] Sectors per cluster: %d\n", sectors_per_cluster);
    tty_printf("[FAT] Cluster size: %d\n", cluster_size);

    //Calculating the start of the root directory
    int root_cluster_32 = ((fat_extbpb_32_t*)(fat_bpb->extended_section))->root_cluster;
    int first_sector_of_root_cluster = ((root_cluster_32 - 2) * fat_bpb->sectors_per_cluster) + first_data_sector;

    //Loading the first sector of the root directory
    
    uint8_t cluster[2048];
    uint8_t FAT_table[512];
    fatdir_std_t* directory_entry;

    bool last_cluster = false;

    //Loading the first cluster
    sd_readblock(first_sector_of_root_cluster + LBA_first_sector, cluster, sectors_per_cluster);
    uint32_t current_cluster_number = root_cluster_32;

    while(!last_cluster){

        //Reading the directory info from the loaded cluster
        directory_entry = (fatdir_std_t*)cluster;
        int entry_in_cluster = 0;
        for(entry_in_cluster = 0; entry_in_cluster < 64; entry_in_cluster++){

            //We leave if we reach the final file in the directory
            if(directory_entry->name[0] == 0x00){
                break;
            }

            //Else, we print its name
            if(directory_entry->name[0] != 0xe5 && directory_entry->attr[0] != 0x0f){
                tty_printf("[FAT] First filename: %c%c%c%c%c%c%c%c.%c%c%c\n", directory_entry->name[0], 
                                                                        directory_entry->name[1],
                                                                        directory_entry->name[2],
                                                                        directory_entry->name[3],
                                                                        directory_entry->name[4],
                                                                        directory_entry->name[5],
                                                                        directory_entry->name[6],
                                                                        directory_entry->name[7],
                                                                        directory_entry->ext[0],
                                                                        directory_entry->ext[1],
                                                                        directory_entry->ext[2]);
            }

            directory_entry += 1;

        }

        //Loading the next cluster

        tty_printf("Current cluster number: %d\n", current_cluster_number);

        unsigned int fat_offset = current_cluster_number * 4;
        unsigned int fat_sector = first_fat_sector + (fat_offset / bytes_per_sector);
        unsigned int ent_offset = fat_offset % bytes_per_sector;

        //Loading the correct sector of the FAT into memory
        sd_readblock(fat_sector + LBA_first_sector, FAT_table, 1);

        uint32_t table_value = *(uint32_t*)&FAT_table[ent_offset];
        table_value = table_value & 0x0fffffff;
        current_cluster_number = table_value;

        tty_printf("Ent offset: %d, %x\n", ent_offset, ent_offset);
        tty_printf("Table value: %x\n", table_value);

        if(table_value >= 0x0ffffff8){
            last_cluster = true;
            break;
        }else{
            uint32_t sector_to_load = ((table_value - 2) * fat_bpb->sectors_per_cluster) + first_data_sector;
            sd_readblock(sector_to_load + LBA_first_sector, cluster, sectors_per_cluster);
        }

        wait_micro_st(1000000);

    }

    return 0;
}

int fat_loadcluster(unsigned int cluster_number, uint8_t* buffer){



    return 0;
}