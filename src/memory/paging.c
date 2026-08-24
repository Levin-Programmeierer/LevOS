#include "memory/paging.h"
#include "drivers/terminal.h"
#include "memory/pmm.h"

unsigned int page_directory[1024] __attribute__((aligned(4096)));
unsigned int page_table[1024] __attribute__((aligned(4096)));

void init_paging(void){
    for(int i = 0; i < 1024; i++){
        page_directory[i] = 0;
        page_table[i] = (i * 4096) | 0x03; // | bitwise OR to set config to 0x03 and * 4096 because of the size
    }
    page_directory[0] = (unsigned int)page_table | 0x03; // copy address to page directory and get the config right by bitwise or 0x03
    load_page_directory(page_directory);
}

void map_page(unsigned int virtual_address, unsigned int physical_address, unsigned char mode){
	if(mode == 'K'){
		unsigned int directory_index =
			(virtual_address >> 22) & 0x3FF;

		unsigned int table_index =
			(virtual_address >> 12) & 0x3FF;

		unsigned int aligned_address =
			physical_address & 0xFFFFF000;

		unsigned int *table;

		if((page_directory[directory_index] & 0x01) == 1){

			table = (unsigned int *)
				(page_directory[directory_index] & 0xFFFFF000);

		}else{

			unsigned int new_page_table_address =
				allocate_page();

			table = (unsigned int *)new_page_table_address;

			for(int i = 0; i < 1024; i++){

				table[i] = 0;

			}

			page_directory[directory_index] =
				new_page_table_address | 0x03;
	
		}

		table[table_index] = aligned_address | 0x03;
	}else if(mode == 'U'){
		unsigned int directory_index =
			(virtual_address >> 22) & 0x3FF;

		unsigned int table_index =
			(virtual_address >> 12) & 0x3FF;

		unsigned int aligned_address =
			physical_address & 0xFFFFF000;

		unsigned int *table;

		if((page_directory[directory_index] & 0x04) == 0x04){

			table = (unsigned int *)
				(page_directory[directory_index] & 0xFFFFF000);

		}else{

			unsigned int new_page_table_address =
				allocate_page();

			table = (unsigned int *)new_page_table_address;

			for(int i = 0; i < 1024; i++){

				table[i] = 0;

			}

			page_directory[directory_index] =
				new_page_table_address | 0x07;
	
		}

		table[table_index] = aligned_address | 0x07;
	}
}

void reload_page_directory(void){
    load_page_directory(page_directory);
}