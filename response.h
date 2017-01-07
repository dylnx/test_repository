#ifndef RESPONSE_H_DS59TYX1
#define RESPONSE_H_DS59TYX1


void packet_dispatch(void *s);
int TagReportDataHandle(MConn *session, callback processor, void *args);
int EventReportHandle(MConn *session, list events);
int get_packet_type(void *s);

typedef int (*tag_handle_on_recevied)(struct tag* tag);
#endif /* end of include guard: RESPONSE_H_DS59TYX1 */
