// Include necessary header files
#include "stdint-gcc.h"
#include "vga.h"
#include "multiboot.h"
#include "kernel.h"
#include "scanf.h"
#include "idt.h"
#include "gdt.h"
#include "printf.h"
#include "vesa.h"
#include "termianl.h"
#include "kernel.h"
#include "timer.h"
#include "scheduler.h"
#include "cpu.h"
#include "command.h"
#include "pageing.h"
#include "ide.h"
#include "fat_access.h"
#include "fat_filelib.h"
#include "syscall.h"
#include "vmm.h"
#include "io_ports.h"
#include "pagepmm.h"
#include "pageing.h"
#include "pmm.h"
#include "kheap.h"
#include "stdbool.h"
#include "elf.h"
#include "elf_exe.h"
#include "exe.h"
#include "cursor.h"
#include "logging.h"
#include "stdio.h"
#include "stdlib.h"

void command_line(void);
void loop(void);
char pch = 'A';
int generate_random_number();
static int generate_random_number_asm();
void Process(void)
{
    char ch = pch ++;
    while (true) {
        printf("%c",ch);
        sleep(500 + (ch * 100));

        if ((ch % 2) == 0) {
            printf("%c",ch);
            printf('!');
            TerminateProcess();
        }
    }
}
void loop_timer()
{
    // 
    size_t ticks = get_ticks();
    size_t target = 50;
    size_t old_tick = ticks;
    while(1)
    {
        if(ticks > old_tick+target)
        {   
            update_cursor();
            old_tick = ticks;

        }
        else
        {
            ticks = get_ticks();

      

        }
        
    }
    
    
}

KERNEL_MEMORY_MAP g_kmap;
int get_kernel_memory_map(KERNEL_MEMORY_MAP *kmap, MULTIBOOT_INFO *mboot_info) {
    uint32 i;

    if (kmap == NULL) return -1;
    kmap->kernel.k_start_addr = (uint32)&__kernel_section_start;
    kmap->kernel.k_end_addr = (uint32)&__kernel_section_end;
    kmap->kernel.k_len = ((uint32)&__kernel_section_end - (uint32)&__kernel_section_start);

    kmap->kernel.text_start_addr = (uint32)&__kernel_text_section_start;
    kmap->kernel.text_end_addr = (uint32)&__kernel_text_section_end;
    kmap->kernel.text_len = ((uint32)&__kernel_text_section_end - (uint32)&__kernel_text_section_start);

    kmap->kernel.data_start_addr = (uint32)&__kernel_data_section_start;
    kmap->kernel.data_end_addr = (uint32)&__kernel_data_section_end;
    kmap->kernel.data_len = ((uint32)&__kernel_data_section_end - (uint32)&__kernel_data_section_start);

    kmap->kernel.rodata_start_addr = (uint32)&__kernel_rodata_section_start;
    kmap->kernel.rodata_end_addr = (uint32)&__kernel_rodata_section_end;
    kmap->kernel.rodata_len = ((uint32)&__kernel_rodata_section_end - (uint32)&__kernel_rodata_section_start);

    kmap->kernel.bss_start_addr = (uint32)&__kernel_bss_section_start;
    kmap->kernel.bss_end_addr = (uint32)&__kernel_bss_section_end;
    kmap->kernel.bss_len = ((uint32)&__kernel_bss_section_end - (uint32)&__kernel_bss_section_start);

    kmap->system.total_memory = mboot_info->mem_low + mboot_info->mem_high;

    for (i = 0; i < mboot_info->mmap_length; i += sizeof(MULTIBOOT_MEMORY_MAP)) {
        MULTIBOOT_MEMORY_MAP *mmap = (MULTIBOOT_MEMORY_MAP *)(mboot_info->mmap_addr + i);
        if (mmap->type != MULTIBOOT_MEMORY_AVAILABLE) continue;
        // make sure kernel is loaded at 0x100000 by bootloader(see linker.ld)
        if (mmap->addr_low == kmap->kernel.text_start_addr) {
            // set available memory starting from end of our kernel, leaving 1MB size for functions exceution
            kmap->available.start_addr = kmap->kernel.k_end_addr + 1024 * 1024;
            kmap->available.end_addr = mmap->addr_low + mmap->len_low;
            // get availabel memory in bytes
            kmap->available.size = kmap->available.end_addr - kmap->available.start_addr;
            return 0;
        }
    }

    return -1;
}

/**
 * Function Name: kmain
 * Description: Kernel main function, entry point of the operating system.
 * 
 * Parameters:
 *   magic (unsigned long) - Magic number indicating Multiboot compatibility.
 *   addr (unsigned long) - Address of Multiboot information structure.
 * 
 * Return:
 *   void
 */
void kmain(unsigned long magic, unsigned long addr)
{
    init(magic,addr);
    STI();
   
}

void command_line(void)
{
    FILE *fp = fopen("/data.txt", "w");
    fclose(fp);
    FILE *f = fopen("/data.txt", "w");
    if(fp == NULL)
    {
        printf("Failed to open /dev/fmb\n");
    }
    else
    {
        printf("Writing to file \n\n");
        char *da = "hello world! you guys";
        fprintf(f,"%s",da);
        // fclose(f);
        // uint32_t *data = VBE_RGB(255,0,0);
        // fwrite(data,get_vbe_loaction(1000,50),100,fp);
    }
    // STI();
    // printf("Commnd command\n================================\n");
    // init_security();
    // login();
    printf("Welcome to Iris! A simple bash like shell\n");
    printf("Type help or iris for more information\n");

    LOG_LOCATION;
    // sleep(3);
    size_t command_buffer_size = 1024;
    char **history = (char**)malloc(1024);
    if(history == NULL)
    {
        perror("Failed to allocate for history buffer");

    }
    size_t cmd_count = 0;
    char *input_buffer = (char *)malloc(command_buffer_size+1);
    // printf("HERE\n");
    int buffer_pos = 0;
    char user[] = "Dev";
    memset(input_buffer,1,command_buffer_size);
    // free(input_buffer);
    printf("\n>");
    while(1)
    {
        fgets(input_buffer,command_buffer_size,stdin);
        if(input_buffer == NULL || input_buffer[0] == '\0')
        {
             memset(input_buffer, 0,command_buffer_size);
             printf("\n>");
    

        }
        else
        {   
            // printf("\n%s",input_buffer);
            cmd(input_buffer);
            // history[cmd_count] = (char*)malloc(strlen(input_buffer) + 1);
            // if(history[cmd_count] == NULL)
            // {
            //     perror("Failed to allocate memory for command history\n");
            // }
            // else
            // {
            // strcpy(history[cmd_count],input_buffer);

            // }
            // cmd_count++;
            memset(input_buffer, 0,command_buffer_size);
            printf(">");


        }   
       
        
    }
    // TerminateProcess();
}