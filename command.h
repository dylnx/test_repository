#ifndef COMMAND_H_KDPB2ZF3
#define COMMAND_H_KDPB2ZF3

#define GET_VERSION	1
#define GET_VERSION_RESPONSE	2

#define GET_CAPABILITIES	10
#define GET_CAPABILITIES_RESPONSE	11

#define SET_INVENTORY_SPEC	20
#define SET_INVENTORY_SPEC_RESPONSE	21
#define GET_INVENTORY_SPEC	22
#define GET_INVENTORY_SPEC_RESPONSE	23
#define SET_ACCESS_SPEC	24
#define SET_ACCESS_SPEC_RESPONSE	25
#define GET_ACCESS_SPEC	26
#define GET_ACCESS_SPEC_RESPONSE	27

#define GET_CONFIG	40
#define GET_CONFIG_RESPONSE	41
#define SET_CONFIG	42
#define SET_CONFIG_RESPONSE	43
#define CLOSE_CONNECTION	44
#define CLOSE_CONNECTION_RESPONSE	45
#define FIRMWARE_UPGRADE	46
#define FIRMWARE_UPGRADE_RESPONSE	47
#define REBOOT	48
#define REBOOT_RESPONSE	49
#define RESET	50
#define RESET_RESPONSE	51

#define SET_REPORT_SPEC	70
#define SET_REPORT_SPEC_RESPONSE 71
#define GET_REPORT_SPEC	72
#define GET_REPORT_SPEC_RESPONSE 73
#define GET_TAG_REPORT	74
#define TAG_REPORT	75
#define EVENT_REPORT	76
#define HEARTBEAT	77
#define HEARTBEAT_ACK	78

#define CUSTOM_MESSAGE	254
#define ERROR_MESSAGE	255


Mbyte reboot_reader();
Mbyte get_inventory_status();
Mbyte set_inventory_status(u8 status);
//Mbyte set_inventory_spec(u8 status, u8 continued);
Mbyte set_inventory_spec(u8 status, u8 continued, u32 lastime, Mbyte antenna);
Mbyte set_inventory_spec1(u8 blt, u8 code,  Mbyte antenna);
Mbyte set_inventory_spec_detail(int workmode, int protocol, int frequence, int lasttime, Mbyte antenna, void *data);

Mbyte set_access_status(u8 status);
Mbyte get_access_status();
Mbyte set_access_spec(u8 status);
Mbyte set_access_spec_multi(u8 status, Mbyte* settings, int total);

Mbyte get_reader_version();
Mbyte get_reader_capabilities(u8 type);

//for testing
Mbyte  read_two_storage();
Mbyte  read_user_data();
Mbyte read_tid_storage();
Mbyte read_single_storage(u8 area, u16 saddr, u16 len, u32 token);
Mbyte read_double_storage(u8 area1, u16 addr1, u16 len1, u32 token1,
		u8 area2, u16 addr2, u16 len2, u32 token2);
Mbyte  write_reader_data(u8 area, u16 address,u32 token, Mbyte data);

Mbyte  get_config(u8 type, u8 antennaid, u16 gpiid, u16 gpoid);
Mbyte set_config(Mbyte rconfig, Mbyte aconfig, Mbyte gpo, Mbyte custom);
Mbyte set_antenna_power(int power);

Mbyte  close_connection();
Mbyte  reset_reader();
Mbyte reboot_reader1();
Mbyte query_gpio_signal();
//Mbyte  set_reader_time();
#endif /* end of include guard: COMMAND_H_KDPB2ZF3 */
