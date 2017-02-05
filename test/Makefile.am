bin_PROGRAMS=testsuit
noinst_LIBRARIES=libtool.a
libtool_a_SOURCES=logfile.c logfile.h\
				  cond_thread.c cond_thread.h\
				  tcputil.c tcputil.h\
				  dbutils.c dbutils.h\
				  bigint.c bigint.h

testsuit_SOURCES=main.c\
				 pass_record.c pass_record.h\
				 whitelist.c whitelist.h
testsuit_LDADD=libtool.a
