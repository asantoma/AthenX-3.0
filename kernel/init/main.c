
#include "ide.h"
#include "fat_filelib.h"
#include "multiboot.h"
#include "kernel.h"
#include "layout.h"
#include "stdint.h"
#include "vesa.h"
#include "vmm.h"
#include "pmm.h"
#include "io_ports.h"
// #include "stb_images.h"
#include "img.h"
#include "command.h"
#include "errno.h"
#include "../info.c"
#include "termianl.h"
#include "clock.h"
#include "printf.h"
#include "rsdp.h"
#include "acpi.h"
#include "vga.h"
#include "idt.h"
#include "gdt.h"
#include "debug_term.h"
#include "string.h"
#include "pmm.h"
#include "vmm.h"
#include "pci_dev.h"
#include "timer.h"
#include "keyboard.h"
#include "stdlib.h"
#include "kheap.h"
#include "scheduler.h"
#include "dwarf.h"
#include <stdio.h>
#include "ini/ini.h"
struct BootConfig {
    char version_number[20];
    char program_path[256];
    char bin_path[256];
    char error_log[256];
};


int fill_program_list(int num_programs,Entry *entries);
/**
 * Function Name: init
 * Description: Initializes the kernel and system components.
 *
 * Parameters:
 *   magic (unsigned long) - Magic number passed by bootloader.
 *   addr (unsigned long) - Address of multiboot information structure.
 *
 * Return:
 *   void
 */
void init(unsigned long magic, unsigned long addr) {
    // Initialize Interrupt Descriptor Table
    idt_init();

    // Initialize Global Descriptor Table
    gdt_init();
    init_com1();
    timer_init();
    
    // Print kernel start message
    printf("\033[1;34mStarting kernel\n"); // Set text color to blue
    printf("Hello World!\033[0m\n"); // Reset text color to default

    // Initialize VESA graphics mode
    int width = 1024;
    int height = 768;
    int ret = vesa_init(width, height, 32);
    printf_com("Ret = %d\n", ret);
    if (ret < 0) {
        // Try different resolutions if initialization fails
        width = 800;
        height = 600;
        ret = vesa_init(width, height, 32);
        printf_com("-\tScreen resolution: %dx%d\n", width, height);
        if (ret < 0) {
            width = 720;
            height = 480;
            ret = vesa_init(width, height, 32);
            if (ret < 0) {
                printf_com("\n\n\033[1;31mPrinting all modes:\n\n"); // Set text color to red
                vbe_print_available_modes();
            }
        }
    }

    // Initialize terminal with specified resolution
    init_terminal(width, height);
    init_debug_terminal(width, height);

    
    printf_com("Made it here\n");
    int acpi = initAcpi();
    int acpi2 = acpiEnable();
    if(acpi2 != acpi)
    {
        if(acpi2 < acpi)
        {
            acpi = acpi2;
        }
        else if (acpi < acpi2)
        {
            acpi = acpi;
        }
        {
            /* code */
        }
        
    }

    // Print system information
    printf("\033[33mCurrent Date and Time:\n"); // Set text color to yellow
    print_date();
    printf("\033[0m");
    printf("   ");
    printf("\033[32mBooting AthenX-3.0\n"); // Set text color to green
    printf("\033[0m");
    printf("Version: %s - %d.%d.%d\n", VERSION_STRING, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    printf("Compile version %d\n", VERSION_COMPILE);
    printf("Please stand by\n");
    printf("Booting: ");
    int draw_x = get_terminal_postion_x();
    int draw_y = get_terminal_postion_y();
    set_terminal_postion_x(draw_x + 120); // Adjust position for loading bar

    // Display loading bar
    int total_steps = 10;
    int current_step = 1;
    draw_loading_bar(++current_step, total_steps, draw_x, draw_y, VBE_RGB(255, 0, 0), 2);

    MULTIBOOT_INFO *mboot_info;
    if (magic == MULTIBOOT_BOOTLOADER_MAGIC) {
        // Cast addr to MULTIBOOT_INFO pointer
        mboot_info = (MULTIBOOT_INFO *)addr;

        // Clear kernel memory map structure
        memset(&g_kmap, 0, sizeof(KERNEL_MEMORY_MAP));

        // Get kernel memory map
        if (get_kernel_memory_map(&g_kmap, mboot_info) < 0) {
            printf("\033[1;31merror: failed to get kernel memory map\n"); // Set text color to red
            return -1;
        }
        // Calculate allocation size for memory
    }

    // Initialize ATA drivers
    ata_init();
    draw_loading_bar(++current_step, total_steps, draw_x, draw_y, VBE_RGB(255, 0, 0), 2);

    // Initialize FAT file system
    fl_init();
    if (fl_attach_media(ide_read_sectors_fat, ide_write_sectors_fat) != FAT_INIT_OK)
    {
        printf("\033[1;31mERROR: Failed to init file system\n"); // Set text color to red
        return -1;
    }
    draw_loading_bar(++current_step, total_steps, draw_x, draw_y, VBE_RGB(255, 0, 0), 2);

    // Initialize Scheduler
    InitScheduler();
    draw_loading_bar(++current_step, total_steps, draw_x, draw_y, VBE_RGB(255, 0, 0), 2);

    // Initialize Physical Memory Manager
    size_t size = (g_kmap.available.size / 2) + 10;
    uint32_t pmm_start = (uint32_t)g_kmap.available.start_addr;
    asm("cli");
    init_pmm_page(pmm_start, g_kmap.available.size);
    draw_loading_bar(++current_step, total_steps, draw_x, draw_y, VBE_RGB(255, 0, 0), 2);

    // Initialize Virtual Memory Manager
    init_vmm();
    LOG_LOCATION;

    // Collect pages for Physical Memory Manager
    pmm_collect_pages(mboot_info);

    // Map VESA memory
    map_vesa();
    LOG_LOCATION;

    // Initialize kernel heap
    init_kheap(g_kmap.available.size);
    draw_loading_bar(++current_step, total_steps, draw_x, draw_y, VBE_RGB(255, 0, 0), 2);
    LOG_LOCATION;
    init_fs();
    LOG_LOCATION;

    // Scan PCI devices
    pci_scan();
    draw_loading_bar(++current_step, total_steps, draw_x, draw_y, VBE_RGB(255, 0, 0), 2);

    // Initialize timer
    timer_init();
    draw_loading_bar(++current_step, total_steps, draw_x, draw_y, VBE_RGB(255, 0, 0), 2);

    LOG_LOCATION;

    int ret_buf = vesa_init_buffers();
    printf_com("%d\n", ret_buf);
    char *test_malloc = malloc(1024 * 1024 * 1024);
    if (test_malloc == NULL) {
        printf("\033[1;31mCouldn't allocate test memory\n"); // Set text color to red
    } else {
        printf_debug("\033[1;32mSuccessfully allocated memory of 1GB\n"); // Set text color to green
    }
    LOG_LOCATION;

    // Free allocated memory
    const char *test_b = "This allocated memory";
    strcpy(test_malloc, test_b);
    if (strcmp(test_b, test_malloc) != 0) {
        printf("\033[1;31mMapping of test allocation may have failed\n"); // Set text color to red
    }
    free(test_malloc);
    draw_loading_bar(++current_step, total_steps, draw_x, draw_y, VBE_RGB(255, 0, 0), 2);

    LOG_LOCATION;

    struct BootConfig config;
    load_boot_config("./init/ini.ini", &config);

    int num_programs;
    int num_program_dirs;
    fl_count_files(config.program_path, &num_program_dirs, &num_programs);
    Entry *programs = malloc(sizeof(Entry) * num_programs);
    int num_programs_check;
    fl_populate_file_list(config.program_path, programs, &num_programs_check);
    if (num_programs_check != num_programs) {
        printf("\033[1;31mError getting list of programs\n"); // Set text color to red
    } else {
        fill_program_list(num_programs, programs);
    }
    keyboard_init();
    draw_loading_bar(++current_step, total_steps, draw_x, draw_y, VBE_RGB(255, 0, 0), 2);

    printf("\033[1;32mSystem Initialization Complete\n"); // Set text color to green
    printf("System Info:\n");

    uint64_t ram_size = mboot_info->mem_low + mboot_info->mem_high;
    printf("-\tRAM size: %u Kilobytes\n", ram_size);
    printf("-\tAllocation heap size: %u Kilobytes\n", g_kmap.available.size / 1024);

    char cpu_name[49];
    char architecture[5];
    unsigned int family, model, stepping;
    get_cpu_info(cpu_name, architecture, &family, &model, &stepping);
    printf("-\tCPU Name: %s\n", cpu_name);
    printf("-\tArchitecture: %s\n", architecture);
    printf("-\tFamily: %d, Model: %d, Stepping: %d\n", family, model, stepping);
    printf("-\tScreen resolution: %dx%d\n", width, height);

    // const char *acpi_status = (acpi == 0) ? "true" : "false";
    if(acpi != 0)
    {
        
    }
    const char *acpi_status = (acpi == 0) ? "\x1b[32mtrue\x1b[0m":"\x1b[31mFALSE\x1b[0m";
    printf("-\tACPI enabled: %s\n", acpi_status);

    printf("\033[0m"); // Reset text formatting and colors

    printf("Device Info:\n");
    print_pci_devices();

    STI();

    printf("Starting shell\n");
    CreateProcess(command_line);
    CreateProcess(loop_timer);
    PerformButler();
    STI();
}

int fill_program_list(int num_programs,Entry *entries)
{
    executables_path= (char**)malloc(num_programs * sizeof(char*));
      if (executables_path == NULL) {
        perror("Memory allocation failed");
        return -1;
    }
    for (size_t i = 0; i < num_programs; i++)
    {
        printf("\n\n");
        executables_path[i] = (char*)malloc(strlen(entries[i].name));
        strcpy(executables_path[i],entries[i].name);
        printf("Executable %s found\n",executables_path[i]);
        if (executables_path[i] == NULL) {
            perror("Memory allocation failed");
            return -1;
        }
    }
    
}

int print_pci_devices()
{
    // Print the number of devices for each device type if there are more than 0 devices
    if (get_num_unclassified_devices() > 0) {
        printf("-\tNumber of Unclassified Devices: %d\n", get_num_unclassified_devices());
    }
    if (get_num_mass_storage_devices() > 0) {
        printf("-\tNumber of Mass Storage Devices: %d\n", get_num_mass_storage_devices());
    }
    if (get_num_network_devices() > 0) {
        printf("-\tNumber of Network Devices: %d\n", get_num_network_devices());
    }
    if (get_num_display_devices() > 0) {
        printf("-\tNumber of Display Devices: %d\n", get_num_display_devices());
    }
    if (get_num_multimedia_devices() > 0) {
        printf("-\tNumber of Multimedia Devices: %d\n", get_num_multimedia_devices());
    }
    if (get_num_memory_devices() > 0) {
        printf("-\tNumber of Memory Devices: %d\n", get_num_memory_devices());
    }
    if (get_num_bridge_devices() > 0) {
        printf("-\tNumber of Bridge Devices: %d\n", get_num_bridge_devices());
    }
    if (get_num_simple_comm_devices() > 0) {
        printf("-\tNumber of Simple Communications Devices: %d\n", get_num_simple_comm_devices());
    }
    if (get_num_base_system_periph_devices() > 0) {
        printf("-\tNumber of Base System Peripheral Devices: %d\n", get_num_base_system_periph_devices());
    }
    if (get_num_input_devices() > 0) {
        printf("-\tNumber of Input Devices: %d\n", get_num_input_devices());
    }
    if (get_num_docking_devices() > 0) {
        printf("-\tNumber of Docking Devices: %d\n", get_num_docking_devices());
    }
    if (get_num_processor_devices() > 0) {
        printf("-\tNumber of Processor Devices: %d\n", get_num_processor_devices());
    }
    if (get_num_serial_bus_devices() > 0) {
        printf("-\tNumber of Serial Bus Devices: %d\n", get_num_serial_bus_devices());
    }
    if (get_num_wireless_devices() > 0) {
        printf("-\tNumber of Wireless Devices: %d\n", get_num_wireless_devices());
    }
    if (get_num_intelligent_io_devices() > 0) {
        printf("-\tNumber of Intelligent IO Devices: %d\n", get_num_intelligent_io_devices());
    }
    if (get_num_satellite_devices() > 0) {
        printf("-\tNumber of Satellite Communication Devices: %d\n", get_num_satellite_devices());
    }
    if (get_num_encryption_devices() > 0) {
        printf("-\tNumber of Encryption Devices: %d\n", get_num_encryption_devices());
    }
    if (get_num_signal_processing_devices() > 0) {
        printf("-\tNumber of Signal Processing Devices: %d\n", get_num_signal_processing_devices());
    }
    if (get_num_processing_accel_devices() > 0) {
        printf("-\tNumber of Processing Accelerator Devices: %d\n", get_num_processing_accel_devices());
    }
    if (get_num_non_essential_devices() > 0) {
        printf("-\tNumber of Non-Essential Instrumentation Devices: %d\n", get_num_non_essential_devices());
    }
    if (get_num_coprocessor_devices() > 0) {
        printf("-\tNumber of Coprocessor Devices: %d\n", get_num_coprocessor_devices());
    }
    return 0;
}
// boot_config.c

// Structure to hold boot configuration data

// Callback function to handle parsed INI file data
static int config_handler(void *user, const char *section, const char *name,
                          const char *value) {
    struct BootConfig *config = (struct BootConfig *)user;

    if (strcmp(section, "version") == 0) {
        if (strcmp(name, "version_number") == 0) {
            strncpy(config->version_number, value, sizeof(config->version_number));
        }
    } else if (strcmp(section, "paths") == 0) {
        if (strcmp(name, "program_path") == 0) {
            strncpy(config->program_path, value, sizeof(config->program_path));
        } else if (strcmp(name, "bin_path") == 0) {
            strncpy(config->bin_path, value, sizeof(config->bin_path));
        }
    } else if (strcmp(section, "logs") == 0) {
        if (strcmp(name, "error_log") == 0) {
            strncpy(config->error_log, value, sizeof(config->error_log));
        }
    }

    return 1; // Continue parsing
}

int load_boot_config(const char *config_file,struct BootConfig *boot_config) {
    struct BootConfig config;

    // Initialize config with default values
    memset(boot_config, 0, sizeof(struct BootConfig));

    // Parse the INI file
    if (ini_parse(config_file, config_handler, boot_config) < 0) {
        fprintf(stderr, "Error: Can't load '%s'\n", config_file);
        return -1;
    }

    // Now, 'config' contains the parsed boot configuration data
    // printf("AthenX Version: %s\n", boot_config->version_number);
    // printf("Program Path: %s\n", boot_config->program_path);
    // printf("Bin Path: %s\n", boot_config->bin_path);
    // printf("Error Log: %s\n",boot_config->error_log);
    // // boot_config = &config;
    // You can use the parsed data as needed for booting AthenX

    return 0;
}
