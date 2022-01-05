#include <stdio.h>
#include <linux/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#define FILE_NAME "/dev/i2c-1"
enum data_typef{
A8D8 = 0,
A8D16 = 1,
A16D8 = 2,
A16D16 = 3,
};

static int i2c_write(int fd,unsigned char slave_addr, unsigned int reg_addr, unsigned int value,enum data_typef type)
{
    unsigned char outbuf[32];
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[1];

    messages[0].addr  = slave_addr;
    messages[0].flags = 0;
    messages[0].len   = sizeof(outbuf);
    messages[0].buf   = outbuf;

    /* The first byte indicates which register we‘ll write */
		switch(type)
		{
			case A8D8:
				outbuf[0] = (unsigned char)(reg_addr & 0xff);
				outbuf[1] = (unsigned char)(value & 0xff);
				messages[0].len   = 2;
				break;
			case A16D16:
				outbuf[1] = (unsigned char)(reg_addr & 0xff);
				outbuf[0] = (unsigned char)((reg_addr >>8) & 0xff);
				outbuf[3] = (unsigned char)((value >>8) & 0xff);
				outbuf[2] = (unsigned char)(value & 0xff);
				messages[0].len   = 4;

				break;
			case A8D16:
				outbuf[0] = (unsigned char)(reg_addr & 0xff);
				outbuf[2] = (unsigned char)((value >>8) & 0xff);
				outbuf[1] = (unsigned char)(value & 0xff);
				messages[0].len   = 3;
				break;
			case A16D8:
				outbuf[1] = (unsigned char)(reg_addr & 0xff);
				outbuf[0] = (unsigned char)((reg_addr >>8) & 0xff);
				outbuf[2] = (unsigned char)(value & 0xff);
				messages[0].len   = 3;
				break;
			default:
			printf("error \n");
			break;
		}


    /*
     * The second byte indicates the value to write.  Note that for many
     * devices, we can write multiple, sequential registers at once by
     * simply making outbuf bigger.
     */
    //outbuf[1] = value;

    /* Transfer the i2c packets to the kernel and verify it worked */
    packets.msgs  = messages;
    packets.nmsgs = 1;
    if(ioctl(fd, I2C_RDWR, &packets) < 0)
    {
        perror("Unable to send data");
        return 1;
    }

	printf("write data 0x%02x to slave_addr 0x%02x reg_addr 0x%02x\n", value, slave_addr, reg_addr);

    return 0;
}

static int i2c_read(int fd, unsigned char slave_addr, unsigned int reg_addr, unsigned char *value,enum data_typef type) 
{
    unsigned char inbuf[32], outbuf[32];
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];

    /*
     * In order to read a register, we first do a "dummy write" by writing
     * 0 bytes to the register we want to read from.  This is similar to
     * the packet in set_i2c_register, except it‘s 1 byte rather than 2.
     */

    messages[0].addr  = slave_addr;
    messages[0].flags = 0;
	if(type == A16D8|| type == A16D16)
	{
		messages[0].len   = 2;
		outbuf[1] = (unsigned char)(reg_addr & 0xff);
		outbuf[0] = (unsigned char)((reg_addr >>8) & 0xff);
	}
	else{
		messages[0].len   = 1;
		outbuf[0] = (unsigned char)(reg_addr & 0xff);
	}
    messages[0].buf   = outbuf;

    /* The data will get returned in this structure */
    messages[1].addr  = slave_addr;
    messages[1].flags = I2C_M_RD/* | I2C_M_NOSTART*/;
	if(type == A8D16|| type == A16D16)
	{
		messages[1].len   = 2;
	}
	else{
		messages[1].len   = 1;
	}

    messages[1].buf   = inbuf;

    /* Send the request to the kernel and get the result back */
    packets.msgs      = messages;
    packets.nmsgs     = 2;
    if(ioctl(fd, I2C_RDWR, &packets) < 0)
    {
        perror("Unable to send data");
        return 1;
    }
    *value = inbuf[0];
	*(value+1) = inbuf[1];
	printf("read data 0x%02x.0x%02x from slave_addr 0x%02x reg_addr 0x%02x\n", *value,*(value+1), slave_addr, reg_addr);

    return 0;
}

//home # ./main i2c_write A16D16 0X5D 0x8140 0x1234
//write data 0x1234 to slave_addr 0x5d reg_addr 0x8140
///home # ./main i2c_read A16D16  0x5D 0X8140
//read data 0x34.0x12 from slave_addr 0x5d reg_addr 0x8140
int main(int argc, char **argv)
{
    int fd;
    unsigned int slave_addr=0, reg_addr=0, value = 0;
	enum data_typef type;

    if (argc < 5){
        printf("Usage:\n%s r[w] type(A8D8,A8D16,A16D8,A16D16) addr reg_addr [value]\n",argv[0]);
        return 0;
    }

    fd = open(FILE_NAME, O_RDWR);
    if (!fd)
    {
        printf("can not open file %s\n", FILE_NAME);
        return 0;
    }

	if(!strcmp(argv[2],"A8D8"))
	    type = A8D8;
    else if(!strcmp(argv[2],"A8D16"))
		type = A8D16;
	else if(!strcmp(argv[2],"A16D8"))
		type = A16D8;
	else if(!strcmp(argv[2],"A16D16"))
		type = A16D16;
	else
		printf("Usage:\n%s r[w] type(A8D8,A8D16,A16D8,A16D16) addr reg_addr [value]\n",argv[0]);
    sscanf(argv[3], "%x", &slave_addr);
    sscanf(argv[4], "%x", &reg_addr);

    if(!strcmp(argv[1],"i2c_read"))
    {
        i2c_read(fd, slave_addr, reg_addr, (unsigned char*)&value,type);//gt911 touch
    }
    else if(argc>5&&!strcmp(argv[1],"i2c_write"))
    {
        sscanf(argv[5], "%x", &value);
        i2c_write(fd, slave_addr, reg_addr, value,type);
    }

    close(fd);
    return 0;
}
