#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "gpio.h"


static int gpio_fd;

int init_gpio();
int get_gpio(int no, int *val);
int set_gpio(int no, int val);

int init_gpio()
{
	gpio_fd = open(GPIO_NODE, O_WRONLY);
	if (gpio_fd < 0)
	{
		printf("open error \n");
		return -1;
	}
	return 0;
}
int get_gpio(int no, int *val)
{
	if ( gpio_fd <= 0 ) return -1;
	if ( no < 0 || no > 37 ) return -1;
	if ( val == NULL ) return -1;
	NORCO_DEV_PARAM dev;
	int gpio, action, option, ret;
	action = ACTION_ENABLE_INPUT; 
	option = 0; 

	gpio = no;
	
	dev.gpio = gpio;
	dev.action = action;
	dev.option = option;	
	ret = ioctl(gpio_fd, NORCO_GPIO, &dev);
	if (ret < 0) 
	{
		printf("error ioctl\n");
		return -1;
	}
	
	dev.gpio = gpio;
	dev.action = ACTION_GET_VALUE;
	dev.option = 0;	
	ret = ioctl(gpio_fd, NORCO_GPIO, &dev);
	if (ret < 0) 
	{
		printf("error ioctl\n");
		return -1;
	}
	//printf("INPUT-GET gpio=%d, level=%d \n", dev.gpio, dev.option);
	
	*val = dev.option;
	return 0;
}
int set_gpio(int no, int val)
{
	if ( gpio_fd <= 0 ) return -1;
	if ( no < 0 || no > 37 ) return -1;
	if ( val != 0 && val != 1 ) return -1;
	NORCO_DEV_PARAM dev;
	int gpio, action, option, ret;
	action = ACTION_ENABLE_OUTPUT; 
	option = 0; 

	gpio = no;
	
	dev.gpio = gpio;
	dev.action = action;
	dev.option = option;	
	ret = ioctl(gpio_fd, NORCO_GPIO, &dev);
	if (ret < 0) 
	{
		printf("error ioctl\n");
		return -1;
	}
	
	dev.gpio = gpio;
	dev.action = ACTION_SET_VALUE;
	dev.option = val;	
	ret = ioctl(gpio_fd, NORCO_GPIO, &dev);
	if (ret < 0) 
	{
		printf("error ioctl\n");
		return -1;
	}
	//printf("OUTPUT-SET gpio=%d, level=%d \n", dev.gpio, dev.option);
	
	return 0;
}
int close_gpio()
{
	if ( gpio_fd <= 0 ) return -1;
	close(gpio_fd);
	return 0;
}
/*
// ./gpio_test <port0-37> [0|1]
read pin0
#./gpio_test 0
set pin0 to value 1
#./gpio_test 0 1
set pin0 to value 0
#./gpio_test 0 0
*/

/*
int main(int argc, char *argv[])
{
	printf("hello gpio!\n");
    
	if(argc < 2 || argc > 3)
	{
		printf("%s <port0-37> [0|1] \n", argv[0]);
		return -1;
	}
    init_gpio();
	if ( argc == 2 )
	{
		int val;
		get_gpio(atoi(argv[1]),&val);
		
	}
	if ( argc == 3 )
	{
		set_gpio(atoi(argv[1]), atoi(argv[2]));
	}
	
	close_gpio();
	
	return 0;
}
*/

