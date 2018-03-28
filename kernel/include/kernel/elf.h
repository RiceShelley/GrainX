#ifndef __ELF__H
#define __ELF__H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <kernel/malloc.h>
#include <kernel/vga_console.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    uint8_t identity[16];
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint32_t entry;
    uint32_t ph_offset;
    uint32_t sh_offset;
    uint32_t flags;
    uint16_t header_size;
    uint16_t ph_size;
    uint16_t ph_num;
    uint16_t sh_size;
    uint16_t sh_num;
    uint16_t strtab_index;
}__attribute__((packed)) elf_header;

typedef struct {
    uint32_t type;
    uint32_t offset;
    uint32_t virtual_address;
    uint32_t physical_address;
    uint32_t file_size;
    uint32_t mem_size;
    uint32_t flags;
    uint32_t align;
}__attribute__((packed)) elf_phead;

int load_elf(uint8_t *data);

#endif