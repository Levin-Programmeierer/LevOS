#include "drivers/keyboard.h"
#include "cpu/interrupts.h"
#include <stdint.h>
#include "drivers/terminal.h"
#include "drivers/cmos.h"
#include "drivers/ata.h"
#include "filesystem/fat32.h"
#include <stdint.h>

#define COMMAND_BUFFER_SIZE 128
#define COMMAND_SIZE 32
#define PARAMETER_SIZE 96

void process_command(void);
void split_command(const char *input, char *command, char *parameters);
int strcmp(const char *a, const char *b);

char command_buffer[COMMAND_BUFFER_SIZE];
char command[COMMAND_SIZE];
char argument[PARAMETER_SIZE];
unsigned int command_length = 0;

uint16_t background = 0x0;
static ata_pio_device_t shell_ata;
static fat32_fs_t shell_fat32;
static fat32_result_t shell_fat32_status = FAT32_EBADFS;
static uint8_t cat_buffer[32768];

void shell_filesystem_init(void)
{
    ata_pio_init(&shell_ata);
    if (!ata_pio_is_present(&shell_ata))
        shell_fat32_status = FAT32_EIO;
    else
        shell_fat32_status = fat32_mount(&shell_fat32, &shell_ata.block);
}

static void filesystem_error(fat32_result_t result)
{
    print("filesystem: ", RED);
    print(fat32_result_string(result), YELLOW);
    if (result == FAT32_EIO) {
        print(" (", YELLOW);
        print(ata_pio_error_string(ata_pio_last_error(&shell_ata)), YELLOW);
        print(")", YELLOW);
    }
}

static int print_directory_entry(const fat32_dirent_t *entry, void *context)
{
    (void)context;
    print(entry->name, WHITE);
    if ((entry->attributes & 0x10) != 0)
        putchar('/');
    putchar('\n');
    return 0;
}

void shell(void){
    //clear(background);
    print("----------------------------------LevShell 0.1----------------------------------", (BLUE << 4) | WHITE); // 12 characters long
    if (shell_fat32_status != FAT32_OK) {
        print("\nfilesystem: ", RED);
        if (!ata_pio_is_present(&shell_ata)) {
            print(ata_pio_error_string(ata_pio_last_error(&shell_ata)), YELLOW);
            print(" (attach a hard disk image as the primary master)", YELLOW);
        } else {
            print(fat32_result_string(shell_fat32_status), YELLOW);
        }
    }
    print("\n> ", (background << 4) | WHITE);
    for (;;) {
        char c = keyboard_getchar();

        if (c == 0) {
            __asm__ volatile ("hlt");
            continue;
        }

        if (c == '\n') {
            command_buffer[command_length] = '\0';
            process_command();
            command_length = 0;
            print("\n> ", (background << 4) | WHITE);
        }
        else if (c == '\b') {
            if (command_length > 0) {
            command_length--;
            command_buffer[command_length] = '\0';
            putchar('\b');
            }
        }
        else if (command_length < COMMAND_BUFFER_SIZE - 1) {
            command_buffer[command_length] = c;
            command_length++;
            putchar(c);
        }
    }
}

void process_command(void){
    split_command(command_buffer, command, argument);
    putchar('\n');
    if (strcmp(command, "help") == 0) {
        print("Available commands:", YELLOW);
        print("\nhelp -- this command", WHITE);
        print("\nclear -- clears the screen", WHITE);
        print("\nabout -- shows information about this OS", WHITE);
        print("\nls [path] -- list a FAT32 directory", WHITE);
        print("\ncat path -- print a FAT32 file", WHITE);
    }
    else if (strcmp(command, "clear") == 0) {
        clear(background);
        print("----------------------------------LevShell 0.1----------------------------------", (BLUE << 4) | WHITE);
    }
    else if(strcmp(command, "echo") == 0){
        print(argument, 0x0F);
    } else if(strcmp(command, "clock") == 0){
        //seconds
        select_cmos_reg(0x00);
        uint8_t seconds = read_cmos();
        //minutes
        select_cmos_reg(0x02);
        uint8_t minutes = read_cmos();
        //hours
        select_cmos_reg(0x04);
        uint8_t hours = read_cmos();
        //Day of month
        select_cmos_reg(0x07);
        uint8_t monthday = read_cmos();
        // Month
        select_cmos_reg(0x08);
        uint8_t month = read_cmos();
        // Year -- first 2 digits
        select_cmos_reg(0x09);
        uint8_t year = read_cmos();
        // century
        select_cmos_reg(0x32);
        uint8_t century = read_cmos();

        print("Year: ", YELLOW);
        print_hex_byte(century);
        print_hex_byte(year);
        print("\nDate: ", YELLOW);
        print_hex_byte(month);
        putchar('/');
        print_hex_byte(monthday);
        print("\nTime: ", YELLOW);
        print_hex_byte(hours);
        putchar(':');
        print_hex_byte(minutes);
        putchar(':');
        print_hex_byte(seconds);

    }
    else if(strcmp(command, "about") == 0){
        print("--LevOS - Shell 0.1--", YELLOW);
        print("\nWork in progress\n", RED);
        print(".", (WHITE << 4));
        print(".", (RED << 4));
        print(".", (YELLOW << 4));
        print(".", (BLUE << 4));
        print(".", (GREEN << 4));
        print(".", (CYAN << 4));
        print(".", (BROWN << 4));
        print("\n.", (LIGHT_BLUE << 4));
        print(".", (LIGHT_CYAN << 4));
        print(".", (LIGHT_RED << 4));
        print(".", (LIGHT_GREEN << 4));
        print(".", (GRAY << 4));
        print(".", (DARK_GRAY << 4));
        print(".\n", (LIGHT_PURPLE << 4));
    }
    else if (strcmp(command, "ls") == 0) {
        fat32_result_t result = fat32_list_directory(
            &shell_fat32, argument[0] == '\0' ? "/" : argument,
            print_directory_entry, (void *)0);
        if (result != FAT32_OK)
            filesystem_error(result);
    }
    else if (strcmp(command, "cat") == 0) {
        uint32_t bytes_read = 0;
        fat32_result_t result;
        if (argument[0] == '\0') {
            print("cat: missing path", RED);
        } else {
            result = fat32_read_file(&shell_fat32, argument, cat_buffer,
                                     sizeof(cat_buffer), &bytes_read);
            if (result != FAT32_OK) {
                filesystem_error(result);
            } else {
                for (uint32_t i = 0; i < bytes_read; ++i)
                    putchar((char)cat_buffer[i]);
            }
        }
    }

    else {
        print("Unknown command: ", RED);
        print(command, YELLOW);
    }
}

int strcmp(const char *a, const char *b){
    int i = 0;

    while (a[i] != '\0' && b[i] != '\0') {
        if (a[i] != b[i]) {
            return 1;
        }

        i++;
    }

    return (a[i] != b[i]);
}

void split_command(const char *input, char *command, char *parameters)
{
    int command_index = 0;
    int parameter_index = 0;
    unsigned int i = 0;

    while (i < command_length && input[i] == ' ')
        ++i;
    while (i < command_length && input[i] != ' ') {
        if (command_index < COMMAND_SIZE - 1)
            command[command_index++] = input[i];
        ++i;
    }
    command[command_index] = '\0';
    while (i < command_length && input[i] == ' ')
        ++i;
    while (i < command_length) {
        if (parameter_index < PARAMETER_SIZE - 1)
            parameters[parameter_index++] = input[i];
        ++i;
    }
    parameters[parameter_index] = '\0';
}