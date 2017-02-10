#wrote by hexp

C := gcc
CFLAGS =  -I../lib -g  -Wall
EXEFLAGS = -L ./ -L/lib  -lreader -lpthread -lsqlite3 -ldl



SOURCE_DIR = $(shell pwd)
CFILE_PATTERN = *.c
CFILE = $(shell cd $(SOURCE_DIR); ls $(CFILE_PATTERN))
OBJS =$(patsubst %.c, %.o, $(CFILE))
APPReadTags = ReadTags

$(APPReadTags):$(OBJS)
	rm -f $(APPReadTags)
	$(C) $(CFLAGS) -o  $@  $^ $(EXEFLAGS)


%.o : %.c
	$(C) $(CFLAGS) -c $< -o $@

.PHONY : clean

clean:
	rm -f *.o $(APPReadTags)

