#ifndef __EXT2__H
#define __EXT2__H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <kernel/malloc.h>
#include <kernel/ata.h>
#include <kernel/vga_console.h>

#define EXT2_SIG 0xEF53

#define INODE_TYPE_FIFO 0x1000
#define INODE_TYPE_CHAR_DEV 0x2000
#define INODE_TYPE_DIRECTORY 0x4000
#define INODE_TYPE_BLOCK_DEV 0x6000
#define INODE_TYPE_FILE 0x8000
#define INODE_TYPE_SYMLINK 0xA000
#define INODE_TYPE_SOCKET 0xC000

typedef struct {
        uint32_t inodes;
        uint32_t blocks;
        uint32_t reserved_for_root;
        uint32_t unallocatedblocks;
        uint32_t unallocatedinodes;
        uint32_t superblock_id; // number of the block containing the superblock
        uint32_t blocksize_hint; // shift by 1024 to the left to find block size
        uint32_t fragmentsize_hint; // shift by 1024 to left
        uint32_t blocks_in_blockgroup;
        uint32_t frags_in_blockgroup;
        uint32_t inodes_in_blockgroup;
        uint32_t last_mount;
        uint32_t last_write;
        uint16_t mounts_since_last_check;
        uint16_t max_mounts_since_last_check;
        uint16_t ext2_sig; // 0xEF53
        uint16_t state;
        uint16_t op_on_err;
        uint16_t minor_version;
        uint32_t last_check;
        uint32_t max_time_in_checks;
        uint32_t os_id;
        uint32_t major_version;
        uint16_t uuid;
        uint16_t gid;
        uint8_t unused[940];
} __attribute__((packed)) superblock_t;

typedef struct {
        uint16_t type;
        uint16_t uid;
        uint32_t size; // size of file in bytes
        uint32_t last_access;
        uint32_t create_time;
        uint32_t last_modif;
        uint32_t delete_time;
        uint16_t gid;
        uint16_t hardlinks;
        uint32_t disk_sectors;
        uint32_t flags;
        uint32_t ossv1;
        uint32_t dbp[12]; // direct block pointer
        uint32_t singly_block;
        uint32_t doubly_block;
        uint32_t triply_block;
        uint32_t gen_no;
        uint32_t reserved1;
        uint32_t reserved2;
        uint32_t fragment_block;
        uint8_t ossv2[12];
} __attribute__((packed)) inode_t;

typedef struct {
        uint32_t block_of_block_usage_bitmap;
        uint32_t block_of_inode_usage_bitmap;
        uint32_t block_of_inode_table;
        uint16_t num_of_unalloc_block;
        uint16_t num_of_unalloc_inode;
        uint16_t num_of_dirs;
        uint8_t unused[14];
} __attribute__((packed)) block_group_desc_t;

typedef struct __ext2_priv_data {
        superblock_t sb;
        uint32_t first_bgd;
        uint32_t number_of_bgs;
        uint32_t blocksize;
        uint32_t sectors_per_block;
        uint32_t inodes_per_block;
} __attribute__((packed)) ext2_priv_data;

typedef struct __ext2_dir_entry {
        uint32_t inode;
        uint16_t size;
        uint8_t namelength;
        uint8_t reserved;
        /* name here */
} __attribute__((packed)) ext2_dir;


uint8_t ext2_probe();
uint8_t ext2_read_root_directory(char *filename);
size_t ext2_read_file(char *filename, char *buff);
uint8_t ext2_write_file(char *fn, char *buf, uint32_t len);
void list(char *path);

#endif