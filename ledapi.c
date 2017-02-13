
#include "ledapi.h"
#include "LedUtils.h"
#include "ChangeChar.h"

int width_door = 128;
int high_door = 64;
int width_not_door = 96;
int high_not_door = 48;
int size_door = 24;
int size_not_door = 16;

int show_init(char *led_ip,int led_port,bool be_door)
{
    char str1[] = "\u6b22\u8fce\u5149\u4e34";//文本内容为："欢迎光临"
    char str[20];
    int color = 65280; // yellow:65535; green:65280; red:255;

    int len = strlen(str1);

    unicode_gb2312(str1,len,str,20);
    len = strlen(str);

    int left = 0, top = 0, width = 0, high = 0, size = 0, width_for_show = 0, high_for_show = 0, font = FONT_SET_16;
    if (be_door)
    {
        width = width_door;
        high = high_door;
        size = size_door;
        font = FONT_SET_24;
    }
    else
    {
        width = width_not_door;
        high = high_not_door;
        size = size_not_door;
        font = FONT_SET_16;
    }
    left = (width - len * (size/2))/2;
    top = (high - size)/2;
    width_for_show = len * (size/2);
    high_for_show = size;

    BYTE stream[65536];
    MakeRoot(stream);
    AddChapter(0x7fffffff, PLAY_MODE_WAIT);
    AddRegion(0, 0, width_door, high_door);
    AddLeaf(1000, PLAY_MODE_WAIT);
    AddStrings(left, top, width_for_show, high_for_show, 1);
    AddChildString(str, font, color, 1, 0, 1, 0, 0, 0, 0);

    DWORD send_size;
    BYTE buffer[6144];
    DWORD i, K;
    WORD port;

    int s = InitSocket();
    //========
    send_size=DoBeginPacket(buffer, 0);
    len = SocketWrite(s,buffer,send_size,led_ip,led_port);
    //========
    K=GetDataPacketCount(stream);
    for (i=1; i<=K; i++)
        send_size=DoDataPacket(stream, i, buffer, 0);
    len = SocketWrite(s,buffer,send_size,led_ip,led_port);
    //========
    send_size=DoEndPacket(buffer, K+1, 0);
    len = SocketWrite(s,buffer,send_size,led_ip,led_port);
    CloseSocket(s);
    return 1;
}

/*
功能
    车牌显示,TCP的短连接方式
参数
    led_ip:
    led_pord:
    pai:
    be_entry:
*/
int show_chepai(char *led_ip,int led_port,const char * pai,bool be_entry)
{
    char str1[] = "\u5141\u8bb8\u901a\u884c";   //文本内容为 “允许通行”
    char str2[] = "\u7981\u6b62\u901a\u884c";   //文本内容为 “禁止通行“
    int color = 255;
    char str[20];
    int len = 0;
    int font = FONT_SET_16;

    BYTE stream[65536];
    MakeRoot(stream);
    AddChapter(0x7fffffff, PLAY_MODE_WAIT);

    if (be_entry)
    {
        len = strlen(str1);
        unicode_gb2312(str1,len,str,20);
        color = 255;
    }
    else
    {
        len = strlen(str2);
        unicode_gb2312(str2,len,str,20);
        color = 255;
    }
    // line2: entry info

    AddRegion(0, 0, 64, 32);
    AddLeaf(1000, PLAY_MODE_WAIT);

    
    AddStrings(0, 0, 64, 32, 1);
    AddChildString(str, font, color, 1, 0, 1, 0, 0, 0, 0);

    // line1: che pai
    if (len > 12)
        len = 12;

    len = strlen(pai);
    unicode_gb2312(pai,len,str,20);

    AddRegion(0, 16, 64, 32);
    AddLeaf(1000, PLAY_MODE_WAIT);

    AddStrings(0, 0, 64, 32, 1);
    AddChildString(str, font, color, 1, 0, 1, 0, 0, 0, 0);

    // send
    DWORD send_size;
    BYTE buffer[6144];
    DWORD i, K;
    WORD port;

    int s = InitSocket();
    //========
    send_size=DoBeginPacket(buffer, 0);
    len = SocketWrite(s,buffer,send_size,led_ip,led_port);
    //usleep(100*1000);
    //========
    K=GetDataPacketCount(stream);
    for (i=1; i<=K; i++)
        send_size=DoDataPacket(stream, i, buffer, 0);
    len = SocketWrite(s,buffer,send_size,led_ip,led_port);
    //usleep(100*1000);
    //========
    send_size=DoEndPacket(buffer, K+1, 0);
    len = SocketWrite(s,buffer,send_size,led_ip,led_port);
    //usleep(100*1000);
    CloseSocket(s);
    return 0;
}
int show_camera(char *led_ip,int led_port,bool be_door,const char * pai,bool be_entry)
{
    char str1[] = "\u4e34\u65f6\u653e\u884c";       // can entry
    char str2[] = "\u672a\u6388\u6743\u8f66\u8f86"; // can't entry

    int color = 255;
    char str[20];
    int len = 0;
    int width = 0, high = 0, size = 0, font = FONT_SET_16, left = 0, 
		top = 0, width_for_show = 0, high_for_show = 0;
    if (be_door)
    {
        width = width_door;
        high = high_door;
        size = size_door;
        font = FONT_SET_24;
    }
    else
    {
        width = width_not_door;
        high = high_not_door;
        size = size_not_door;
        font = FONT_SET_16;
    }

    BYTE stream[65536];
    MakeRoot(stream);
    AddChapter(0x7fffffff, PLAY_MODE_WAIT);
    AddRegion(0, 0, width_door, high_door);
    AddLeaf(1000, PLAY_MODE_WAIT);

    if (be_entry)
    {
        len = strlen(str1);
        unicode_gb2312(str1,len,str,20);
        color = 65280;
    }
    else
    {
        len = strlen(str2);
        unicode_gb2312(str2,len,str,20);
        color = 255;
    }

    // line2: entry info
    len = strlen(str);

    left = (width - len * (size/2))/2;
    top = high/2 + (high/2 - size)/2;
    width_for_show = len * (size/2);
    high_for_show = size;
    AddStrings(left, top, width_for_show, high_for_show, 1);
    AddChildString(str, font, color, 1, 0, 1, 0, 1, 2000, 5000);
    // line1: che pai
    len = strlen(pai);
    if (len > 12)
        len = 12;
    unicode_gb2312(pai,len,str,20);
    len = strlen(str);

    left = (width - len * (size/2))/2;
    top = (high/2 - size)/2;
    width_for_show = len * (size/2);
    high_for_show = size;

    AddStrings(left, top, width_for_show, high_for_show, 1);
    AddChildString(str, font, color, 1, 0, 1, 0, 0, 0, 0);

    // send
    DWORD send_size;
    BYTE buffer[6144];
    DWORD i, K;
    WORD port;

    int s = InitSocket();

    send_size=DoBeginPacket(buffer, 0);

    len = SocketWrite(s,buffer,send_size,led_ip,led_port);

    K=GetDataPacketCount(stream);

    for (i=1; i<=K; i++)
        send_size=DoDataPacket(stream, i, buffer, 0);
    len = SocketWrite(s,buffer,send_size,led_ip,led_port);

    send_size=DoEndPacket(buffer, K+1, 0);

    len = SocketWrite(s,buffer,send_size,led_ip,led_port);

    CloseSocket(s);
    return 1;
}



