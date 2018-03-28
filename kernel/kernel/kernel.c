#include <stdbool.h> 
#include <stddef.h>
#include <stdint.h>

// GrainX libc
#include <stdio.h>
#include <math.h>

// GrainX kernel libs
#include <kernel/vga_console.h>
#include <kernel/ps2.h>
#include <kernel/tty.h>
#include <kernel/ata.h>
#include <kernel/malloc.h>
#include <kernel/ext2.h>
#include <kernel/elf.h>

extern void restart(void);
extern void real_mode(void);

extern uint32_t multiboot_memory_low;
extern uint32_t multiboot_memory_high;
extern uint32_t multiboot_info;

extern uint32_t kernel_end;
extern uint32_t kernel_base;

char *ptr;

char buff[256];
char l_cmd[256];
char wdir[256];

uint32_t usr_mem_MB = 0;

void print_logo()
{
	puts("  _____           _      __   __\n");
	puts(" / ____|         (_)     \\ \\ / /\n");
	puts("| |  __ _ __ __ _ _ _ __  \\ V / \n");
	puts("| | |_ | '__/ _` | | '_ \\  > <  \n");
	puts("| |__| | | | (_| | | | | |/ . \\ \n");
	puts(" \\_____|_|  \\__,_|_|_| |_/_/ \\_\\\n\n");
	return;
}

void print_reboo()
{
	puts("  -------------- \n");
	puts("   re-BOO ghost! \n");
	puts("  -------------- \n");
	puts("     \\   \n");
	puts("      \\ \n");
	puts("      .-. \n");
	puts("     (o o) \n");
	puts("     | O \\ \n");
	puts("      \\   \\ \n");
	puts("       `~~~' \n");
	return;
}

void print_mem()
{
	printf("User memory: %d Mb\n", usr_mem_MB);
	printf("Lower memory: %d Kb\n", multiboot_memory_low);
	printf("Kernel base: %d\n", (uint32_t) &kernel_base);
	printf("Kernel end : %d\n", (uint32_t) &kernel_end);
	uintptr_t end = (uintptr_t) &kernel_end;
	uintptr_t start = (uintptr_t) &kernel_base;
	printf("Kernel size: %d\n", (uint32_t) (end - start));
	return;
}

void console_data_dump(uint8_t *buff, size_t len) 
{
	for (size_t i = 0; i < len; i++) {
		if (buff[i] == 0) {
			printf("0");
		} else {
			printf("%c", buff[i]);
		}
	}
	puts("\n\ndone.\n");
}

// kernel entry point
uint32_t kernel_main(void) 
{
	// <----- Initialize VGA terminal ----->
	vga_term_init();

	// <----- Initialize memory manager ----->
	mm_init(&kernel_end);	
	// Output amount of user memory
	usr_mem_MB = (multiboot_memory_high / 1024.0);
	puts("\nMem:\n");
	printf("->User memory: %s Mb\n", u32_to_str(usr_mem_MB, buff, 30));
	printf("->Lower memory: %s Kb\n\n", u32_to_str(multiboot_memory_low, buff, 30));

	// <----- Initialize ata driver ----->
	puts("Probing primary ATA bus master / slave drives:\n");
	ata_probe();

	// <----- Initialize ext2 fs driver ----->
	ext2_probe();

	// start user prompt
	puts("\nStarting user console.\n");

	// set term dir to root dir
	memset(wdir, 0, 256);
	memcpy(wdir, "/", strlen("/"));

	// print GrainX logo
	print_logo();

	printf("Enter help for cmd list.\n%s >>> ", wdir);

	uint32_t mem_start = heap_addr();

	while(true) {
		memset((char *) buff, 0, 256);
		char *out = tty_get_user_input(buff, 100);

		puts("\n");

		if (memcmp(out, "!!", 2))
			out = (char *) memcpy(buff, l_cmd, 256);
		if (memcmp(out, "reboot", strlen("reboot")))
			return 0;
		else if (memcmp(out, "reboo", strlen("reboo")))
			print_reboo();
		else if (memcmp(out, "echo", strlen("echo")))
			printf("%s", (const char *) &out[5]);
		else if (memcmp(out, "logo", strlen("logo")))
			print_logo();
		else if (memcmp(out, "mem", strlen("mem")))
			print_mem();
		else if (memcmp(out, "hang", strlen("hang")))
			return 1;
		else if (memcmp(out, "clear", strlen("clear")))
			vga_term_clear();
		else if (memcmp(out, "fd", strlen("fd")))
			ata_probe();
		else if (memcmp(out, "write", strlen("write"))) {
			printf("file '%s'\n", &out[6]);
			char *fn = &out[6];
			printf("Enter data:\n>>> ");
			// Get data to write from usr
			char data[256];
			memset((char *) data, '\0', 256);
			const char *data_out = tty_get_user_input(data, 250);
			if (memcmp(data_out, "exit", 4))
				continue;
			data[strlen(data_out)] = '\n';
			// Write text to file	
			ext2_write_file(fn, data_out, strlen(data_out));
		} else if (memcmp(out, "read", strlen("read"))) {
			// Read a file off hard disk
			char *filename = &out[5];
			char path[256];
			memset(path, 0, 256);
			memcpy(path, wdir, 256);
			if (path[strlen(path) - 1] != '/')
				path[strlen(path)] = '/';
			memcpy(&path[strlen(path)], filename, strlen(filename));
			printf("file '%s'\n", path);
			char file_buff[1024 * 4];
			size_t len = read(path, file_buff, 1000);
			printf("size = %d\n", (uint32_t) len);
			//printf("\n%x\n", file_buff);
			console_data_dump(file_buff, len);
		} else if (memcmp(out, "load bin ", strlen("load bin"))) {
			char *filename = &out[strlen("load bin ")];
			printf("loading: %s\n", filename);
			uint8_t *data = malloc(2048);
			read(filename, data, 2048);
			console_data_dump(data, 10);
			load_elf(data);
		} else if (memcmp(out, "ls", strlen("ls"))) {
			list(wdir);
		} else if (memcmp(out, "cd ", strlen("cd "))) {
			// Change working directory
			if (out[3] == '/') {
				memset(wdir, 0, 256);
				memcpy(wdir, &out[3], strlen(&out[2]));
			} else if (out[3] + out[4] == ('.' * 2)) {
				for (uint8_t i = strlen(wdir); i > 0; i--) {
					if (wdir[i] != '/')
						wdir[i] = 0;
					else
						break;
				}
			} else {
				memcpy(&wdir[strlen(wdir)], &out[3], strlen(&out[3]));
			}
		} else if (memcmp(out, "heap stat", strlen("heap stat"))) {
			printf("%d% of heap used\n", (uint32_t) heap_used());
		} else if (memcmp(out, "cheap", strlen("cheap"))) {
			mm_init(&kernel_end);
			printf("heap cleared.\n");
		} else if (memcmp(out, "print heap ", strlen("print heap "))) {
			char *usr_in = &out[strlen("print heap ")];
			uint32_t blk = atoi(usr_in, strlen(usr_in));
			print_heap(blk);	
		} else if (memcmp(out, "malloc", strlen("malloc"))) {
			char *usr_in = &out[strlen("malloc ")];
			uint32_t len = atoi(usr_in, strlen(usr_in));
			ptr = malloc(len);
		} else if (memcmp(out, "free", 4)) {
			free(ptr);		
		} else if (memcmp(out, "wdir", 4)) {
			printf("Working dir: %s\n", wdir);	
		} else if (memcmp(out, "help", 4)) {
			puts("reboot - reboot OS\n");
			puts("ls - list working dir\n");
			puts("cd - change working directory\n");
			puts("echo [string] - echo usr text\n");
			puts("logo - print GrainX logo\n");
			puts("mem - kernel mem inf\n");
			puts("hang - hang OS\n");
			puts("clear - clear terminal\n");
			puts("fd - find disk\n");
			puts("write - write text to file\n");
			puts("read - read text from file\n");
			puts("heap stat - print heap usage stats\n");
			puts("print heap [int] - print out 256 byte chunk of heap at [int] * 256\n");
			puts("malloc [int] - alloc [int] bytes onto the heap\n");
			puts("free - remove last malloc from heap\n");
			puts("wdir - print current working dir\n");
		} else if (*out) {
			puts("cmd not found.\n");
		}
		printf("\n%s >>> ", wdir);
		// cpy cmd into last cmd buff
		memcpy(l_cmd, buff, 256);
	}
}
