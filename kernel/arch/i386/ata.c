#include <kernel/ata.h>

#define ATA_PRIMARY_IO 0x1F0
#define ATA_SECONDARY_IO 0x170

uint8_t ata_pm = 0; // Primary master exists
uint8_t ata_ps = 0; // Primary Slave exists

uint8_t *ide_buf = 0;

void ide_select_drive(uint8_t i)
{
	if(i == ATA_MASTER)
		outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xA0);
	else 
		outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xB0);
}

uint8_t ide_identify(uint8_t drive)
{
	uint16_t io = ATA_PRIMARY_IO;
	uint8_t bus = ATA_PRIMARY;

	ide_select_drive(drive);

	// ATA specs say these values must be zero before sending identify cmd
	outb(io + ATA_REG_SECCOUNT0, 0);
	outb(io + ATA_REG_LBA0, 0);
	outb(io + ATA_REG_LBA1, 0);
	outb(io + ATA_REG_LBA2, 0);

	// Send identify cmd
	outb(io + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);

	// Read status port
	uint8_t status = inb(io + ATA_REG_STATUS);

	if(status) {
		// Poll untill BSY clears 
		while(inb(io + ATA_REG_STATUS) & ATA_SR_BSY != 0);
			pm_stat_read:		status = inb(io + ATA_REG_STATUS);

		if(status & ATA_SR_ERR) {
			printf("Error in IDE identify subroutine\n\r");
			return 0;
		}

		while(!(status & ATA_SR_DRQ)) 
			goto pm_stat_read;

		// Read the data from device
		for(int i = 0; i < 256; i++) {
			*(uint16_t *)(ide_buf + i * 2) = inportw(io + ATA_REG_DATA);
		}

		return 1;
	}
	return 0;
}

void ide_400ns_delay(uint16_t io)
{
	for(int i = 0;i < 4; i++)
		inb(io + ATA_REG_ALTSTATUS);
}

uint8_t ide_poll(uint16_t io)
{	
	for(int i=0; i< 4; i++)
		inb(io + ATA_REG_ALTSTATUS);

retry:;
	uint8_t status = inb(io + ATA_REG_STATUS);
	//printf("testing for BSY\n\r");
	if(status & ATA_SR_BSY) goto retry;
	//printf("BSY cleared\n\r");
retry2:	status = inb(io + ATA_REG_STATUS);
	if(status & ATA_SR_ERR) {
		printf("ERR set, device failure!\n\r");
		return 0;
	}
	//printf("testing for DRQ\n\r");
	if(!(status & ATA_SR_DRQ)) goto retry2;
	//printf("DRQ set, ready to PIO!\n\r");
	return 1;
}

// LBA 28 read max 137.4 GB
uint8_t ata_read_one(uint8_t *buf, uint32_t lba)
{
	uint16_t io = ATA_PRIMARY_IO;

	// PIO read cmd
	uint8_t cmd = 0xE0;

	// write upper 8 bits to drive head ata port offset 0x06
	outb(io + ATA_REG_HDDEVSEL, (cmd | (uint8_t)((lba >> 24 & 0x0F))));
	// write 0 to ata port offset 0x01 beacuse the doc said to 
	outb(io + 1, 0x00);
	// write sector count 1 to port offset 0x02
	outb(io + ATA_REG_SECCOUNT0, 1);
	// write lba address lower 8 bits to ata port offset 0x03
	outb(io + ATA_REG_LBA0, (uint8_t)((lba)));
	// write lba address mid 8 bits to ata port at offset 0x04
	outb(io + ATA_REG_LBA1, (uint8_t)((lba) >> 8));
	// write lba address high 8 bits to ata port at offset 0x05
	outb(io + ATA_REG_LBA2, (uint8_t)((lba) >> 16));
	// write ata PIO cmd to ata port at offset 0x07
	outb(io + ATA_REG_COMMAND, ATA_CMD_READ_PIO);

	// poll ide
	if (!ide_poll(io))
		return 0;

	// read data
	for(int i = 0; i < 256; i++) {
		uint16_t data = inportw(io + ATA_REG_DATA);
		*(uint16_t *)(buf + i * 2) = data;
	}
	ide_400ns_delay(io);
	return 1;
}

// LBA 28 PIO write
void ata_write_one(uint8_t *buf, uint32_t lba) 
{
	uint16_t io = ATA_PRIMARY_IO;

	// ata PIO write cmd
	uint8_t cmd = 0xE0;

	// write upper 8 bits to drive head ata port offset 0x06
	outb(io + ATA_REG_HDDEVSEL, (cmd | (uint8_t)((lba >> 24 & 0x0F))));
	// write 0 to ata port offset 0x01 beacuse the doc said to 
	outb(io + 1, 0x00);
	// write sector count 1 to port offset 0x02
	outb(io + ATA_REG_SECCOUNT0, 1);
	// write lba address lower 8 bits to ata port offset 0x03
	outb(io + ATA_REG_LBA0, (uint8_t)((lba)));
	// write lba address mid 8 bits to ata port at offset 0x04
	outb(io + ATA_REG_LBA1, (uint8_t)((lba) >> 8));
	// write lba address high 8 bits to ata port at offset 0x05
	outb(io + ATA_REG_LBA2, (uint8_t)((lba) >> 16));
	// write ata PIO cmd to ata port at offset 0x07
	outb(io + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);

	// poll ide
	ide_poll(io);

	char buff[400];

	// Read data
	for(int i = 0; i < 256; i++) {
		uint16_t data = *(uint16_t*)(buf + i * 2);
		outportw(io + ATA_REG_DATA, data);

		// todo -> fix me need 2 ns delay 
		for (uint8_t delay = 0; delay < 30; delay++){}

	}
	ide_400ns_delay(io);

	// Cache flush
	//outb(io + ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH);
	//ide_poll(io);

	return;
}

void ata_read(uint8_t *buff, uint32_t lba, uint32_t numsects)
{
	for(uint32_t i = 0; i < numsects; i++) {
		if (!ata_read_one(buff, lba + i))
			printf("ATA: FAILED TO READ LBA %d\n", lba + i);
		buff += 512;
	}
}

void ata_write(uint8_t *buff, uint32_t lba, uint32_t numsects) 
{
	for (uint32_t i = 0; i < numsects; i++) {
		ata_write_one(buff, lba + i);
		buff += 512;
		// TODO implement a real delay
		printf("made pass %d\n", i);
	}
}

// probe for ata primary master and slave drives
void ata_probe()
{
	// Check primary bus master drive
	if(ide_identify(ATA_MASTER)) {
		char buff[256];
		ata_pm = 1;
		// process the IDENTIFY data
		char *str = buff;
		for(int i = 0; i < 40; i += 2) {
			str[i] = ide_buf[ATA_IDENT_MODEL + i + 1];
			str[i + 1] = ide_buf[ATA_IDENT_MODEL + i];
		}
		str[40] = '\0';
		printf("-> Master drive found! \r\n--> %s\n\r\n\r", (const char *) str);
	} else {
		printf("-> Master drive not found.\n\r");
	}
	// Check primary bus slave drive
	if(ide_identify(ATA_SLAVE)) {
		char buff[256];
		ata_ps = 1;
		// process the IDENTIFY data
		char *str = buff;
		for(int i = 0; i < 40; i += 2) {
			str[i] = ide_buf[ATA_IDENT_MODEL + i + 1];
			str[i + 1] = ide_buf[ATA_IDENT_MODEL + i];
		}
		str[40] = '\0';
		printf("-> Slave drive found! \r\n--> %s\n\r\n\r", (const char *) str);
	} else {
		printf("-> Slave drive not found.\n\r");
	}
	printf("ATA probe done!\n\r");
}