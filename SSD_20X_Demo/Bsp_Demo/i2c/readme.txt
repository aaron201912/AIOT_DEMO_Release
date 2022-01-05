
使用方法：
编译
 CC=arm-linux-gnueabihf-gcc i2c.c -o main
测试：
以ssd201上的touchpanel来测试


//home # ./main i2c_write A16D16 0X5D 0x8140 0x1234
//write data 0x1234 to slave_addr 0x5d reg_addr 0x8140
///home # ./main i2c_read A16D16  0x5D 0X8140
//read data 0x34.0x12 from slave_addr 0x5d reg_addr 0x8140



