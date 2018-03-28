#include <kernel/elf.h>


int is_elf(elf_header *elf)
{
    if (elf->identity[0] == 0x7f && memcmp(&elf->identity[1], "ELF", 3))
        return 1;
    return 0;
}

void load_elf_segment(uint8_t *data, elf_phead *phead) {
    uint32_t memsize = phead->mem_size; // Size in memory
    uint32_t filesize = phead->file_size; // Size in file
    uint32_t mempos = phead->virtual_address; // Offset in memory
    uint32_t filepos = phead->offset; // Offset in file

    /*uint32_t flags = MM_FLAG_READ;
    if(phead->flags & ELF_PT_W) flags |= MM_FLAG_WRITE;

    new_area(current->proc, mempos, mempos + memsize, \
        flags, MM_TYPE_DATA);

    if(memsize == 0) return;*/

    memcpy(mempos, &data[filepos], filesize);
    memset(mempos + filesize, 0, memsize - filesize);
}

int load_elf(uint8_t *data) {
    elf_header *elf = (elf_header *) data;
    if (!is_elf(elf)) {
        puts("INVALID ELF.\n");
        return 0;
    }
    puts("VALID ELF.\n");

    elf_phead *phead = (elf_phead *) &data[elf->ph_offset];
    
    for (uint32_t i = 0; i < elf->ph_num; i++) {
        if (phead[i].type == 1) {
            //load_elf_segment(data, &phead[i]);
            puts("made it here\n");
        }
    }

    return 1;
}