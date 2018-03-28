#include <kernel/ext2.h>

static ext2_priv_data *priv;

static uint8_t *root_buff = 0;
static uint8_t *block_buff = 0;

// Read ext2 block from disk
void ext2_read_block(uint8_t *buff, uint32_t block) 
{
	// 1 sector = 512 bytes
    uint32_t sectors_per_block = priv->sectors_per_block;
    if (!sectors_per_block)
        sectors_per_block = 1;
    ata_read(buff, block * sectors_per_block, sectors_per_block);
}

void ext2_write_block(uint8_t *buff, uint32_t block)
{
	uint32_t sectors_per_block = priv->sectors_per_block;
	if(!sectors_per_block)
        sectors_per_block = 1;
	ata_write(buff, block * sectors_per_block, sectors_per_block);
}

uint8_t ext2_probe() 
{
    printf("Probing primary master drive for ext2 file system.\n");
    uint8_t *buff = malloc(1024);

    ata_read(buff, 2, 2);
    superblock_t *sb = (superblock_t *) buff;

    if (sb->ext2_sig != EXT2_SIG)
        printf("Invalid EXT2 signature.\n");
    else
        printf("valid EXT2 sig!!!\n");

    // Output info about fs
    printf("total inodes: %d\n", sb->inodes);
    printf("total blocks: %d\n", sb->blocks);
    printf("unallocated blocks: %d\n", sb->unallocatedblocks);
    printf("unallocated inodes: %d\n", sb->unallocatedinodes);
    printf("ext2 fs state: %d\n", (uint32_t) sb->state);
    printf("inodes per block group: %d\n", sb->inodes_in_blockgroup);

	if (!priv)
    	priv = (ext2_priv_data *) malloc(sizeof(ext2_priv_data));

    memcpy(&priv->sb, sb, sizeof(superblock_t));

    // Calculate block size
    uint32_t block_size = 1024 << sb->blocksize_hint;
    printf("block size: %d\n", block_size);

    priv->blocksize = block_size;
    priv->inodes_per_block = block_size / sizeof(inode_t);
    priv->sectors_per_block = block_size / 512;

    // Find number of block groups
    uint32_t number_of_bgs0 = sb->blocks / sb->blocks_in_blockgroup;

    if (!number_of_bgs0)
        number_of_bgs0 = 1;
    
    priv->number_of_bgs = number_of_bgs0;

    // calc the location of the block group descriptor (BGDT) can be found directrly affter the SB
    uint32_t block_bgdt = sb->superblock_id + (sizeof(superblock_t) / block_size);
    priv->first_bgd = block_bgdt;

    free(buff);

    return 0;
}

void ext2_read_inode(inode_t *inode_buff, uint32_t inode)
{
    uint32_t bg = (inode - 1) / priv->sb.inodes_in_blockgroup;
    uint32_t i = 0;

    // now we have which block group the inode is in load that desc
    uint8_t *buff = (uint8_t *) malloc(priv->blocksize);

    ext2_read_block(buff, priv->first_bgd);

    block_group_desc_t *bgd = (block_group_desc_t *) buff;

    for (i = 0; i < bg; i++)
        bgd++;
    
    uint32_t index = (inode - 1) % priv->sb.inodes_in_blockgroup;
    uint32_t block = (index * sizeof(inode_t)) / priv->blocksize;

    ext2_read_block(buff, bgd->block_of_inode_table + block);

    inode_t* _inode = (inode_t *) buff;
    index = index % priv->inodes_per_block;

    for (i = 0; i < index; i++)
        _inode++;

    memcpy(inode_buff, _inode, sizeof(inode_t));
    free((void *) buff);
}

void ext2_write_inode(inode_t *inode_buff, uint32_t ii)
{
	uint32_t bg = (ii - 1) / priv->sb.inodes_in_blockgroup;
	uint32_t i = 0;

	/* Now we have which BG the inode is in, load that desc */
	uint8_t *buff = (uint8_t *) malloc(priv->blocksize);

	ext2_read_block(buff, priv->first_bgd);

	block_group_desc_t *bgd = (block_group_desc_t *) buff;
	/* Seek to the BG's desc */
	for(i = 0; i < bg; i++)
		bgd++;
	/* Find the index and seek to the inode */
	uint32_t index = (ii - 1) % priv->sb.inodes_in_blockgroup;
	//printf("Index of our inode is %d\n", index);

	uint32_t block = (index * sizeof(inode_t))/ priv->blocksize;
	//printf("Relative: %d, Absolute: %d\n", block, bgd->block_of_inode_table + block);

	uint32_t final = bgd->block_of_inode_table + block;
	ext2_read_block(buff, final);
	inode_t* _inode = (inode_t *)buff;
	index = index % priv->inodes_per_block;
	for(i = 0; i < index; i++)
		_inode++;
	/* We have found the inode! */
	memcpy(_inode, inode_buff, sizeof(inode_t));
	ext2_write_block(buff, final);
	free((void *) buff);
}

uint32_t ext2_read_directory(char *filename, ext2_dir *o_dir)
{
	ext2_dir *dir = o_dir;
    inode_t *inode = (inode_t *) malloc(sizeof(inode_t));

	while(dir->inode != 0) {
        uint32_t file_inode = dir->inode;
        size_t name_len = dir->namelength + 1;
        char name[name_len];
		memcpy(name, &dir->reserved + 1, dir->namelength);
		name[dir->namelength] = 0;

		if(filename && strcmp(filename, name) == 0) {
            // File found
			ext2_read_inode(inode, dir->inode);
			free((void *) inode);
			return file_inode;
		}
		dir = (ext2_dir *)((uint32_t)dir + dir->size);
	}
	free((void *) inode);
	return 0;
}

void list_in_dir(ext2_dir *o_dir) 
{
	ext2_dir *dir = o_dir;
    inode_t *inode = (inode_t *) malloc(sizeof(inode_t));

	while(dir->inode != 0) {
        uint32_t file_inode = dir->inode;
        size_t name_len = dir->namelength + 1;
        char name[name_len];
		memcpy(name, &dir->reserved + 1, dir->namelength);
		name[dir->namelength] = 0;

		if (name[0] != '$') {
			ext2_read_inode(inode, dir->inode);
    		if ((inode->type & 0xF000) != INODE_TYPE_DIRECTORY) {
				// if inode is a file
				vga_term_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        		printf("%s ", name, dir->inode);
			} else {
				vga_term_set_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK);
        		printf("%s ", name, dir->inode);
			}
		}
		vga_term_set_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK);
		dir = (ext2_dir *)((uint32_t)dir + dir->size);
	}
	printf("\n");
	free((void *) inode);
	return;
}

void list(char *path)
{
    inode_t *inode = (inode_t *) malloc(sizeof(inode_t));
    uint8_t *buff = (uint8_t *) malloc(priv->blocksize);

    size_t fn_len = strlen(path) + 1;
    char *filename_h = (char *) malloc(fn_len);
	char *o_filename_h = filename_h;
	memcpy(filename_h, path, fn_len);

	size_t n = strsplit(filename_h, '/');
	filename_h++; // skip the first '/'

	uint32_t retnode = 0;

	// Read inode 2 -> root dir into inode
	ext2_read_inode(inode, 2);
	if (memcmp(path, "/\0", 2)) {
		uint32_t b = inode->dbp[0];
		ext2_read_block(buff, b);
		list_in_dir((ext2_dir *) buff);
	} else { 
		/* Now, loop through the DPB's and see if it contains this filename */
		while(n--) {
			for(int i = 0; i < 12; i++) {
				uint32_t b = inode->dbp[i];

				if(!b) 
                    break;

				ext2_read_block(buff, b);
				// if made it to last dir list files in dir
				if (!n) {
					list_in_dir((ext2_dir *) buff);
					free((void *) o_filename_h);
                    free((void *) inode);
                    free((void *) buff);
					return;
				}
				uint32_t rc = ext2_read_directory(filename_h, (ext2_dir *) buff);
				if(!rc) {
					printf("INVALID PATH.\n");
					free((void *) o_filename_h);
                    free((void *) inode);
                    free((void *) buff);
					return;
				} else {
					/* inode now contains that inode
					 * get out of the for loop and continue traversing
					 */
					retnode = rc;
					ext2_read_inode(inode, rc);
					goto fix;
				}
			}
			fix:;
			uint32_t s = strlen(filename_h);
			filename_h += s + 1;
		}
	}
	free((void *) o_filename_h);
    free((void *) inode);
    free((void *) buff);
	return;
}

uint32_t ext2_find_file_inode(char *filename, inode_t *inode_buff)
{
    inode_t *inode = (inode_t *) malloc(sizeof(inode_t));
    uint8_t *buff = (uint8_t *) malloc(priv->blocksize);

    size_t fn_len = strlen(filename) + 1;
    char *filename_h = (char *) malloc(fn_len);
	char *o_filename_h = filename_h;
	memcpy(filename_h, filename, fn_len);

	size_t n = strsplit(filename_h, '/');
	filename_h++;

	uint32_t retnode = 0;

	if(n > 1) { 
		// inode 2 = root dir
		ext2_read_inode(inode, 2);
		while(--n) {
			printf("Looking for: %s\n", filename_h);
			// Traverse inode's direct block pointers
			for(int i = 0; i < 12; i++) {
				uint32_t b = inode->dbp[i];
				if(!b) 
                    break;

				ext2_read_block(buff, b);
				// IF n == 1 then were now looking for a file NOT a dir
				if (n == 1) {
					// Look for filename in dir
					uint32_t rc = ext2_read_directory(filename_h, (ext2_dir *) buff);
					retnode = rc;
					// If ext2_read_directory cannot find filename in dir then return 0
					if (!rc) {
						puts("NO SUCH FILE IN DIR.\n");
						free((void *) o_filename_h);
 	                    free((void *) inode);
 	                    free((void *) buff);
						return 0;
					}
					// SUCCESS we found the file -> read files inode
					ext2_read_inode(inode, rc);
					break;
				}
				// IF n != 1 then were looking for a directory
				uint32_t rc = ext2_read_directory(filename_h, (ext2_dir *) buff);
				if (rc) {
					retnode = rc;
					// We are good here read dir inode and start traversing it for the next inode
					ext2_read_inode(inode, rc);
					// Parse out what dir / file were looking for next
					uint32_t s = strlen(filename_h);
					filename_h += s + 1;
					break;
				} else {
					puts("BAD DIR IN PATH.\n");
					free((void *) o_filename_h);
                    free((void *) inode);
                    free((void *) buff);
					return 0;
				}
			}
		}
		memcpy(inode_buff, inode, sizeof(inode_t));
	} else {
		/* This means the file is in the root directory */
		ext2_read_root_directory(filename_h);
        printf("in find inode local inode struct size = %d\n", inode);
		memcpy(inode_buff, inode, sizeof(inode_t));
	}
	free((void *) o_filename_h);
    free((void *) inode);
    free((void *) buff);
	return retnode;
}

size_t ext2_read_file(char *filename, char *buff)
{
    // Get files inode in mem
    inode_t *inode = (inode_t *) malloc(sizeof(inode_t));
	size_t b_read = 0;

    if (!ext2_find_file_inode(filename, inode)) {
    	puts("FAILED TO FIND FILE INODE.\n");
        return 0;
    }
	printf("b size: %d\n", priv->blocksize);

    for (int i = 0; i < 12; i++) {
        uint32_t b = inode->dbp[i];

        if (b == 0) {
            printf("EOF\n");
            break;
        }

        if (b > priv->sb.blocks)
            printf("If I had a kern panic func i'd be calling it right now.\n");

        // Read ext2 block from fs and copy its contents into @param buff
        uint8_t *ext2_block = (uint8_t *) malloc(priv->blocksize);
        ext2_read_block(ext2_block, b);
		/*if (n < priv->blocksize) {
			memcpy(buff + i * (priv->blocksize), ext2_block, n);
			puts("done reading.\n");
			free((void *) inode);
			b_read += n;
			return b_read;
		}*/
        memcpy(buff + i * (priv->blocksize), ext2_block, priv->blocksize);
        free((void *) ext2_block);
		b_read += inode->size;
        printf("inode on read size = %d\n", inode->size);
    }

    free((void *) inode);
	printf("b read = %d\n", b_read);
    return b_read;
}

uint8_t ext2_read_root_directory(char *filename)
{
    // The root directory is always inode #2 so find BG and read the block
    inode_t *inode = (inode_t *) malloc(sizeof(inode_t));

	root_buff = (uint8_t *) malloc(priv->blocksize);

    ext2_read_inode(inode, 2);

	printf("path = %s\n", filename);

    // cycle through inodes direct block pointers
    for (int i = 0; i < 12; i++) {
        uint32_t b = inode->dbp[i];
        if (b == 0)
            break;

        ext2_read_block(root_buff, b);

        if (ext2_read_directory(filename, (ext2_dir *) root_buff)) {
			free((void *) inode);
			free((void *) root_buff);
            return 1;
		}
    }
	
	free((void *) inode);
	free((void *) root_buff);
    
	if (filename && (uint32_t) filename != 1)
        return 0;

    return 1;
}


void ext2_alloc_block(uint32_t *out)
{
	/* Algorithm: Loop through block group descriptors,
	 * find which bg has a free block
	 * and set that.
	 */
	 ext2_read_block(root_buff, priv->first_bgd);
	 block_group_desc_t *bg = (block_group_desc_t *)root_buff;
	 for(int i = 0; i < priv->number_of_bgs; i++)
	 {
	 	if(bg->num_of_unalloc_block)
	 	{
	 		*out = priv->sb.blocks - bg->num_of_unalloc_block + 1;
	 		bg->num_of_unalloc_block --;
	 		ext2_write_block(root_buff, priv->first_bgd + i);

	 		printf("Allocated block %d\n",*out);

	 		ext2_read_block(root_buff, priv->sb.superblock_id);
			superblock_t *sb = (superblock_t *)root_buff;
			sb->unallocatedblocks --;
			ext2_write_block(root_buff, priv->sb.superblock_id);
			return;
	 	}
	 	bg++;
	 }
}


uint8_t ext2_write_file(char *fn, char *buf, uint32_t len)
{
	/* Steps to write to a file:
	 * - Locate and load the inode
	 * - Check if it is of type INODE_TYPE_FILE
	 * -- If no, bail out.
	 * - If inode->size == 0
	 * -- Allocate len / priv->blocksize amount of blocks.
	 * --- Write the buf to the blocks.
	 * - Else, check which block has the last byte, by
	 *   dividing inode->size by priv->blocksize.
	 * -- Load that block.
	 * -- Inside, the last byte is (inode->size)%priv->blocksize
	 * -- If len < priv->blocksize - (inode->size)%priv->blocksize
	 *    which means that the buf can fill the block.
	 * --- Write and return noerror.
	 * -- Else,
	 * --- Write the maximum possible bytes to the block.
	 * --- The next block doesn't exist. Allocate a new one.
	 * --- Write the rest to that block and repeat.
	 * ALSO, on write: adjust inode->size !!!
	 *
	 */


	inode_t *inode = (inode_t *) malloc(sizeof(inode_t));

	// Locate the inode
	uint32_t inode_id = ext2_find_file_inode(fn, inode);

    // Read inode
	ext2_read_inode(inode, inode_id);

	printf("%s's inode is %d : size = %d\n", fn, inode_id, inode->size);

    if(inode_id == 1) {
		free(inode);
        return 0;
	}

	if(inode->size == 0) {
        printf("INODE SIZE = 0\n");
		/* Allocate len / priv->blocksize amount of blocks. */
		uint32_t blocks_to_alloc = len / priv->blocksize;
		blocks_to_alloc++; /* Allocate atleast one! */
		if(blocks_to_alloc > 12)
		{
			/* @todo */
			printf("Sorry, can't write to files bigger than 12Kb :(\n");
			free((void *) inode);
			return 0;
		}
		for(int i = 0; i < blocks_to_alloc; i++)
		{
			uint32_t bid = 0;
			ext2_alloc_block(&bid);
			inode->dbp[i] = bid;
			printf("Set dbp[%d] to %d\n", i, inode->dbp[i]);
		}
		printf("Allocated %d blocks!\n", blocks_to_alloc);
		inode->size += len; // UPDATE the size
		/* Commit the inode to the disk */
		ext2_write_inode(inode, inode_id);
		/* Write the buf to the blocks. */
		for(int i = 0; i < blocks_to_alloc; i++)
		{
			/* We loop through the blocks and write. */
			ext2_read_block(root_buff, inode->dbp[i]);
			if(i + 1 < blocks_to_alloc) { // If not last block
				memcpy(root_buff, buf + i*priv->blocksize, priv->blocksize);
			} else {// If last block
				printf("Last block write %d => %d!\n", i, inode->dbp[i]);
				memcpy(root_buff, buf + i*priv->blocksize, len);
			}
			ext2_write_block(root_buff, inode->dbp[i]);
		}
		printf("Wrote the data to fresh blocks!\n");
		free((void *) inode);
		return 1;
	}

    // If block has space write new data into existing block

    // Find last data block
	uint32_t last_data_block = inode->size / priv->blocksize;
    // Find last bit of data in the last data block
	uint32_t last_data_offset = (inode->size) % priv->blocksize;

    root_buff = (uint8_t *) malloc(priv->blocksize);

	/* Load that block. */
	ext2_read_block(root_buff, inode->dbp[last_data_block]);
    //printf("last data block = %d\ncontents = '%s'\n", last_data_block, root_buff);

    memcpy(root_buff + last_data_offset, buf, len);
    //printf("last data block = %d\ncontents = '%s'\n", last_data_block, root_buff);
    ext2_write_block(root_buff, inode->dbp[last_data_block]);
    // Update inode size
    inode->size += len;
    printf("new inode size = %d\n", inode->size);
    // write node inode to drive
    ext2_write_inode(inode, inode_id);
    printf("new inode size = %d\n", inode->size);

	free((void *) inode);
	free((void *) root_buff);
	return 1;
}
