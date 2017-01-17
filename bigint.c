#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bigint.h"

/*
功能：
	比较两个64位数据的大小
参数：
	data1：
	data2：
返回：
	-1/data1小; 0/data1=data2; 	1/data1大
*/
int Compare64(unsigned int data1[], unsigned int data2[])
{
	if ( data1[0]==data2[0] )
	{
		if ( data1[1]<data2[1] )
			return -1;
		else if ( data1[1]>data2[1] )
			return 1;
		return 0;
	}
	else if ( data1[0]<data2[0] )
		return -1;
	else //if ( data1[0]>data2[0] )
		return 1;
}

/*
功能：
	C64=A64+B32
参数：
	a：
	b：
	c：
返回：
	0/正常; 1/溢出
*/
int Add32(unsigned int a[], unsigned int b, unsigned int c[])
{
	unsigned int		temp;

	temp = a[1] + b;
	c[0] = a[0];
	c[1] = temp;
	if ( temp<a[1] ) //overflow
	{
		if ( c[0]==0xFFFFFFFF )
		{
			c[0]=0;
			return 1;
		}
		c[0] += 1;
	}
	return 0;
}

/*
功能：
	C64=A64+B64
参数：
	a：
	b：
	c：
返回：
	0/正常; 1/溢出
*/
int Add64(unsigned int a[], unsigned int b[], unsigned int c[])
{
	unsigned int		temp1, temp0;

	temp1 = a[1] + b[1];
	temp0 = a[0] + b[0];
	if ( temp1<a[1] ) //overflow
		temp0 += 1;
	c[0] = temp0;
	c[1] = temp1;
	if ( c[0]<a[0] ) //overflow
		return 1;
	return 0;
}

/*
功能：
	C64=A64-B32
参数：
	a：
	b：
	c：
返回：
	0/正常; 1/溢出
*/
int Substract32(unsigned int a[], unsigned int b, unsigned int c[])
{
	unsigned int		temp;

	if ( a[1]<b )
	{
		if ( a[0]<1 ) // a<b
		{
			c[0]=0;
			c[1]=b-a[1];
			return 1;
		}
		temp = 0xFFFFFFFF - b;
		temp += a[1] + 1;
		c[0] = a[0] - 1;
	}
	else
	{
		temp = a[1] - b;
		c[0] = a[0];
	}
	return 0;
}

/*
功能：
	C64=A64-B64
参数：
	a：
	b：
	c：
返回：
	0/正常; 1/溢出
*/
int Substract64(unsigned int a[], unsigned int b[], unsigned int c[])
{
//	unsigned int		temp;
	int					ret;

	ret=Compare64(a, b);
	if ( ret==0 ) //a==b
	{
		c[0]=0;
		c[1]=0;
		return 0;
	}
	else if ( ret>0 ) //a>b
	{
		if ( a[1]<b[1] )
		{
			c[1] = 0xFFFFFFFF - b[1];
			c[1] += a[1] + 1;
			c[0] = a[0] - b[0] - 1;
		}
		else
		{
			c[1] = a[1] - b[1];
			c[0] = a[0] - b[0];
		}
		return 0;
	}
	else //if ( ret<0 ) //a<b
	{
		if ( b[1]<a[1] )
		{
			c[1] = 0xFFFFFFFF - a[1];
			c[1] += b[1] + 1;
			c[0] = b[0] - a[0] - 1;
		}
		else
		{
			c[1] = b[1] - a[1];
			c[0] = b[0] - a[0];
		}
		return 1;
	}
}

/*
功能：
	C64=A64*B32
参数：
	a：
	b：
	c：
返回：
	0/正常; 1/溢出
*/
int Multiply32(unsigned int a[], unsigned int b, unsigned int c[])
{
	unsigned int		d;
	unsigned int		e[2];
	int					ret=0;

	c[0] = 0;
	c[1] = 0;
	if ( b==0 )
		return 0;

	//寻找b的最高位
	d=0x80000000;
	while ( (d & b)==0 )
	{
		d >>= 1;
		if ( d==0 )
			return 0;
	}
	//用加法模拟乘法
	while ( d!=0 )
	{
		if ( (d & b)!=0 )
		{
			ret += Add64(c, a, e);
			c[0] = e[0];
			c[1] = e[1];
		}
		d >>= 1;
		if ( d!=0 )
		{
			ret += Add64(c, c, e);
			c[0] = e[0];
			c[1] = e[1];
		}
	}
	return ret;
}

/*
功能：
	C64=A64/B32
	C64=A64%B32
参数：
	a：
	b：
	c：
	d：
返回：
	0/正常; 1/溢出
*/
int Divide32(unsigned int a[], unsigned int b, unsigned int c[], unsigned int d[])
{
	unsigned int		x[2], y[2], z[2], u[2];
	unsigned int		e[2], f[2];
	int					ret=0;

	c[0] = 0;
	c[1] = 0;
	d[0] = 0;
	d[1] = 0;

	x[0] = a[0];
	x[1] = a[1];
	y[0] = 0;
	y[1] = b;

	if ( b==0 ) //除数为0
		return 0;

	ret=Compare64(x, y);
	if ( ret<0 ) //a<b
		return 0;
	if ( ret==0 ) //a==b
	{
		c[0] = 0;
		c[1] = 1;
		return 0;
	}

	e[0] = 0; //商
	e[1] = 0;
	while ( 1 == 1)
	{
		//z = x - y   // a-b
		Substract64(x, y, z); 
		f[0] = 0; 
		f[1] = 1; 
		ret = Compare64(z, y);
		while ( ret>=0 ) //while z>=y  //while c>=b
		{
			Add64(y, y, u); // y<<=1   //b<<1
			y[0]=u[0];
			y[1]=u[1];
			Add64(f, f, u); //f<<=1   
			f[0]=u[0];
			f[1]=u[1];
			Substract64(x, y, z); //z = x - y
			ret = Compare64(z, y);
		}
		Add64(e, f, u); //增加商
		e[0]=u[0];
		e[1]=u[1];
		x[0]=z[0]; //x=z,余数
		x[1]=z[1];
		y[0] = 0; //y=b, 原始除数
		y[1] = b;
		ret = Compare64(x, y); //if x<y break
		if ( ret<0 )
			break;
	}
	//e: 商
	c[0] = e[0];
	c[1] = e[1];
	//d: 余数
	d[0] = x[0];
	d[1] = x[1];
	return ret;
}

////////////////////////////////////
