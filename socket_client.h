#include <stdbool.h>
#include <sys/types.h>

/*
typedef struct TPlateInfo_my
{
    BYTE CPRID;
    BYTE videoID;
    DWORD Time;
    DWORD nCredit;
    char PLateNum[13];
    BYTE PlateTyep;
    BYTE wzTyep;
    double fv;
    char logo[10];
    BYTE nouse[6];
    BYTE photoNUm;
}TPLATE_INFO_MY;
*/
typedef struct TPlateInfo_my
{
    unsigned char CPRID;
    unsigned char videoID;
    unsigned int Time;
    unsigned int nCredit;
    char PLateNum[13];
    unsigned char PlateTyep;
    unsigned char wzTyep;
    double fv;
    char logo[10];
    unsigned char nouse[6];
    unsigned char photoNUm;
}TPLATE_INFO_MY;

bool client_send_operinfo();
bool client_recv_whitelist();
bool client_recv_tempcarlist();
//bool client_recv_seatsnum(int gate_id);

int client_init_socket();
bool client_camera(int sock);
