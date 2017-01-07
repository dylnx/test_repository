#ifndef MESSAGE_H_BACSCDHL
#define MESSAGE_H_BACSCDHL

#include "common.h"
#include "argument.h"

typedef struct msg_header{
	char version;
	char type;
	u16 length;
	u16 id;
	char msg[0];
} MHeader;

typedef struct msg_headerA{
	char version;
	char type;
	u16 length;
	u32 devid;
	u32 msgid;
	u32 checksum;
	char msg[0];
}MHeaderA;

typedef struct msg_headerB{
	char version;
	char type;
	u16 length;
	u32 file_name_len;
//	u32 file_body_len;
	u32 devid;
	u32 pkg_index;
	u32 pkg_total;
	u32 checksum;
	char msg[0];
}MHeaderB;


#define ADD2MH_Bytes(HEADER, OFFSET, BUFF) OFFSET = Add2MHeader(HEADER, OFFSET, BUFF)
#define ADD2MH_Data(HEADER, OFFSET, BUFF) OFFSET = Add2MHeader1(HEADER, OFFSET, &BUFF, sizeof(BUFF))
#define NEW_BLANK_MESSAGE(TYPE) ({Mbyte s; s = GetNewMHeader(0, TYPE, 0, sizeof(MHeader)); s;})
#define NEW_MESSAGE(TYPE, LEN) ({Mbyte s; s = GetNewMHeader(0, TYPE, 0, LEN); s;})

//for MHeaderA
#define ADD2MHA_Bytes(HEADER, OFFSET, BUFF) OFFSET = Add2MHeaderA(HEADER, OFFSET, BUFF)
#define ADD2MHA_Data(HEADER, OFFSET, BUFF) OFFSET = Add2MHeaderA1(HEADER, OFFSET, &BUFF, sizeof(BUFF))
#define NEW_BLANK_MESSAGEA(TYPE, MID, DEVID) ({Mbyte s; s = GetNewMHeaderA(0, TYPE, sizeof(MHeaderA), MID, DEVID, 0); s;})
#define NEW_MESSAGEA(TYPE, LEN, MID, DEVID, CHECKSUM) ({Mbyte s; s = GetNewMHeaderA(0, TYPE, LEN, MID, DEVID, CHECKSUM); s;})

//for MHeaderB
#define ADD2MHB_Bytes(HEADER, OFFSET, BUFF) OFFSET = Add2MHeaderB(HEADER, OFFSET, BUFF)


int Add2MHeader(MHeader *header, u32 offset , Mbyte data);
int Add2MHeader1(MHeader *header, u32 offset, void *buff, int type_size);
Mbyte GetNewMHeader(u8 version, u8 type, u16 id, u16 len);

//for MHeaderA
int Add2MHeaderA(MHeaderA *header, u32 offset , Mbyte data);
int Add2MHeaderA1(MHeaderA *header, u32 offset, void *buff, int type_size);
Mbyte GetNewMHeaderA(u8 version, u8 type, u16 len, u32 msgid, u32 devid, u32 checksum);

//for MHeaderB
int Add2MHeaderB(MHeaderB *header, u32 offset , Mbyte data);
Mbyte GetNewMHeaderB(u8 version, u8 type, u16 len, u32 fname_len, u32 devid, u32 pkg_index, u32 pkg_total, u32 checksum);

void ntoh_MHeader(MHeader * header);
void hton_MHeader(MHeader * header);

void ntoh_ARGHeader(ARGHeader * header);
void hton_ARGHeader(ARGHeader * header);

u32 compute_checksum(Mbyte content);

#endif /* end of include guard: MESSAGE_H_BACSCDHL */
