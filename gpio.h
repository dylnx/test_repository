#ifndef NORCO_H
#define NORCO_H

#define GPIO_NODE 	"/dev/norco_gpio"

int init_gpio();
int get_gpio(int no, int *val);
int set_gpio(int no, int val);
int close_gpio();

typedef enum
{
	GPIO_1 = 0,
	GPIO_2,			
	GPIO_3,			
	GPIO_4,			
	GPIO_5,			
	GPIO_6,			
	GPIO_7,			
	GPIO_8,			
	GPIO_9,			
	GPIO_10,		
	GPIO_11,		
	GPIO_12,		
	GPIO_13,		
	GPIO_14,		
	GPIO_15,		
	GPIO_16,		
	GPIO_17,		
	GPIO_18,		
	GPIO_19,		
	GPIO_20,		
	GPIO_21,		
	GPIO_22,
	GPIO_23,		
	GPIO_24,		
	GPIO_25,		
	GPIO_26,		
	GPIO_27,		
	GPIO_28,		
	GPIO_29,		
	GPIO_30,		
	GPIO_31,		
	GPIO_32,
	GPIO_33,		
	GPIO_34,		
	GPIO_35,		
	GPIO_36,		
	GPIO_37,		
	GPIO_38,		
	GPIO_39,
} DEV_GPIO;

typedef enum
{
	NORCO_GPIO = 1010,
	
} NORCO_DEV;

typedef enum
{
	ACTION_MIN=0,
	ACTION_ENABLE_INPUT,
	ACTION_ENABLE_OUTPUT,
	ACTION_DISENABLE_INPUT,
	ACTION_DISENABLE_OUTPUT,
	ACTION_GET_VALUE,
	ACTION_SET_VALUE,
} NORCO_DEV_ACTION;

typedef struct
{
	DEV_GPIO gpio;
	NORCO_DEV_ACTION action;
	int option;
} NORCO_DEV_PARAM;

#endif // XHEJDEVICEDEF_H


