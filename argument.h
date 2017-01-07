#ifndef ARGUMENT_H_WRFHNA6N
#define ARGUMENT_H_WRFHNA6N

// 读写器参数类型定义
#define TYPE_DeviceCapabilites	512
#define TYPE_RIPCapabilities	513

#define TYPE_InventorySpec	530
#define TYPE_InventorySetting	531
#define TYPE_StartTrigger	532
#define TYPE_PeriodicTrigger	533
#define TYPE_GPITrigger	20
#define TYPE_StopTrigger	534
#define TYPE_TagObservationTrigger	21
#define TYPE_AccessSpec	535
#define TYPE_AccessSetting	536

#define TYPE_ReaderConfiguration	570
#define TYPE_ReaderTime	50
#define TYPE_AntennaConfiguration	51
#define TYPE_GPIValue	52
#define TYPE_GPOValue	53

#define TYPE_TagReportSpec	600
#define TYPE_TagReportContentSelector	70
#define TYPE_TagReportData	601
#define TYPE_AntennaID	71
#define TYPE_FirstSeenTime	72
#define TYPE_LastSeenTime	73
#define TYPE_TagSeenCount	74
#define TYPE_EventReportSpec	602
#define TYPE_EventReportContentSeletor	75
#define TYPE_EventReport	603
#define TYPE_GPIEvent	76
#define TYPE_TagReportBufferOverFlowEvent 77
#define TYPE_InventorySpecEvent 78
#define TYPE_AntennaEvent	79
#define TYPE_ConnectionEvent	80
#define TYPE_HeartbeatSpec	81

#define TYPE_UHF_RIPCapabilities	100
#define TYPE_UHF_InventorySetting	650
#define TYPE_UHF_SortSpec	651
#define TYPE_UHF_QuerySpec	101
#define TYPE_UHF_TagSpec	652
#define TYPE_UHF_Read	102
#define TYPE_UHF_Write	653
#define TYPE_UHF_Erase	103
#define TYPE_UHF_Lock	104
#define TYPE_UHF_Kill	105
#define TYPE_UHF_ReadResult	654
#define TYPE_UHF_WriteResult 106
#define TYPE_UHF_EraseResult 107
#define TYPE_UHF_LockResult	108
#define TYPE_UHF_KillResult 109

#define TYPE_Custom	1000

#define TYPE_RIPStatus 1022
#define TYPE_FiledError 511
#define TYPE_ParameterError	1023


//   读写器错误消息的类型定义
#define RF_SUCCESS	0
#define RF_ARG_ERR	100
#define RF_DOMAIN_ERR	101
#define RF_ARG_LEAK	102
#define RF_DOMAIN_LEAK 103
#define RF_ARG_FULL	104
#define RF_DOMAIN_FULL 105
#define RF_ARG_UNKNOWN	106
#define RF_DOMAIN_UNKNOWN	107
#define RF_MSG_UNKNOWN	108
#define RF_VERSION_UNKNOWN	109

#define RF_ARG_ARG_ERR	200
#define RF_ARG_DOMAIN_ERR 201
#define RF_ARG_ARG_LEAK	202
#define RF_ARG_DOMAIN_LEAK 203
#define RF_ARG_ARG_FULL	204
#define RF_ARG_DOMAIN_FULL	205
#define RF_ARG_ARG_UNKNOWN	206
#define RF_ARG_DOMAIN_UNKNOWN 207

#define RF_DOAMIN_ILLEGAL	300

#define RF_READER_ERR	400

#define MAX_ERR_SIZE 1024

typedef struct arg_header{
	u16 type;
	u16 length;
	char arg[0];
} ARGHeader;

// 工具函数
u8* GetNewHeader(u16 type, u16 len);
u8* GetNewHeader_nolen(u16 type, u16 len);
u32 Add2ARGHeader4(ARGHeader* header, u32 offset, Mbyte buff); //for bit array
u32 Add2ARGHeader3(ARGHeader* header, u32 offset, Mbyte buff); //for bit array
u32 Add2ARGHeader2(ARGHeader* header, u32 offset, Mbyte buff);//for byte array
u32 Add2ARGHeader1(ARGHeader* header, u32 offset, void * buff, u32 type_size); //for normal variable
u32 Add2ARGHeader(ARGHeader* header, u32 offset, Mbyte buff);//for sub argument
Mbyte Byte2Data(Mbyte byte, void* data, u32 datasize);
Mbyte Byte2Data1(Mbyte byte, void* data, u32 datasize);

#define ADD2ARG_BitsArray(HEADER, OFFSET, BUFF) OFFSET = Add2ARGHeader3(HEADER, OFFSET, BUFF)
#define ADD2ARG_BytesArray(HEADER, OFFSET, BUFF) OFFSET = Add2ARGHeader2(HEADER, OFFSET, BUFF)
#define ADD2ARG_DoubleBytesArray(HEADER, OFFSET, BUFF) OFFSET = Add2ARGHeader4(HEADER, OFFSET, BUFF)
#define ADD2ARG_Data(HEADER, OFFSET, BUFF) OFFSET = Add2ARGHeader1(HEADER, OFFSET, &BUFF, sizeof(BUFF))
#define ADD2ARG_Bytes(HEADER, OFFSET, BUFF) OFFSET = Add2ARGHeader(HEADER, OFFSET, BUFF)

#define De2BytesArray(BYTE, DATA, LEN) BYTE = Byte2Data1( BYTE, DATA, LEN)
#define De2Data(BYTE, DATA) BYTE = Byte2Data( BYTE, &DATA, sizeof(DATA))

// 盘点和访问相关函数
Mbyte InventorySpec(u8 status, Mbyte setting);
Mbyte InventorySetting(Mbyte antenna, Mbyte start_trigger, Mbyte stop_trigger, Mbyte ap_inventory_setting);
Mbyte StartTrigger(u8 type, Mbyte p_trigger, Mbyte g_trigger);
Mbyte StopTrigger(u8 type, u32 lasttime, Mbyte g_trigger, Mbyte t_trigger);
Mbyte UHF_InventorySetting(u8 FLT_flag, u16 freq_selector, Mbyte *sortspec, int sstotal, Mbyte queryspec);
Mbyte UHF_SortSpec(u8 storearea, u16 address, u16 len, Mbyte mask, u8 rule);
Mbyte UHF_QuerySpec(u8 ifcase, u8 BLT_cycle, u8 encode, u8 F_flag);
Mbyte PeriodicTrigger(u32 off, u32 cycle, Mbyte readertime);
Mbyte AccessSpec(u8 status, Mbyte* settings, int total);
Mbyte AccessSetting(u16 id, Mbyte tagspec, Mbyte commandspec);
Mbyte UHF_TagSpec(u16 address, u8 isMatch, Mbyte mask, Mbyte tagdata);
Mbyte UHF_Read(u8 storearea, u16 address, u16 len, u32 command);
Mbyte UHF_Write(u8 storearea, u16 address, u16 len, Mbyte data, u32 command);

Mbyte Byte2InventorySetting(Mbyte byte);
Mbyte Byte2InventorySpec(Mbyte byte);


// 标签报告相关函数
Mbyte TagReportSpec(u8 type, u16 N, Mbyte content_sel);
Mbyte TagReportContentSelector(u8 flag);
Mbyte Byte2TagReportData(Mbyte byte);
Mbyte Byte2AntennaID(Mbyte byte);
Mbyte Byte2FirstSeenTime(Mbyte byte);
Mbyte Byte2LastSeenTime(Mbyte byte);
Mbyte Byte2TagSeenCount(Mbyte byte);
Mbyte Byte2UHF_AccessResult(Mbyte byte);
Mbyte Byte2UHF_ReadResult(Mbyte byte);
Mbyte Byte2UHF_KillResult(Mbyte byte);
Mbyte Byte2UHF_LockResult(Mbyte byte);
Mbyte Byte2UHF_EraseResult(Mbyte byte);
Mbyte Byte2UHF_WriteResult(Mbyte byte);
Mbyte Byte2AccessSpec(Mbyte byte);


//  事件报告相关函数
Mbyte Byte2ConnectionEvent(Mbyte byte);
Mbyte Byte2AntennaEvent(Mbyte byte);
Mbyte Byte2InventorySpecEvent(Mbyte byte);
Mbyte Byte2TagReportBufferOverFlowEvent(Mbyte byte);
Mbyte Byte2GPIEvent(Mbyte byte);
Mbyte Byte2ReaderTime(Mbyte byte);
Mbyte Byte2EventReport(Mbyte byte);

Mbyte Byte2DeviceCapabilities(Mbyte byte);
Mbyte Byte2RIPCapabilities(Mbyte byte);
Mbyte Byte2Custom(Mbyte byte);



// 读写器配置相关函数
Mbyte Byte2ReaderConfiguration(Mbyte byte);
Mbyte Byte2AntennaConfiguration(Mbyte byte);
Mbyte Byte2GPIValue(Mbyte byte);
Mbyte Byte2GPOValue(Mbyte byte);
Mbyte ReaderConfiguration(Mbyte readertime);
Mbyte AntennaConfiguration(u8 antennaid, u16 sendrate);
Mbyte GPIValue(u16 gpiid, u8 data);
Mbyte GPOValue(u16 gpoid, u8 data);
Mbyte ReaderTime(u64 microsecond);

// 错误相关
Mbyte Byte2RIPStatus(Mbyte byte);

#endif /* end of include guard: ARGUMENT_H_WRFHNA6N */
