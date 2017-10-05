ROOTDIR = $(shell pwd)
#INSTALL_DIR = $(ROOTDIR)/bin
#CROSS_COMPILE = armv6-aspeed-linux-gnueabi-
CC              = $(CROSS_COMPILE)gcc
INCLUDE         = 
OBJS            =
LDLIBS          = -lcrypt
EXTRA_FLAGS     = 
EXEC_NAME       = check_password

all:            $(EXEC_NAME)

$(EXEC_NAME):   $(EXEC_NAME).o
	$(CC) $(MYLDFLAGS) $(OBJS) $(EXEC_NAME).o -o $@ $(LDLIBS) $(EXTRA_FLAGS)

$(EXEC_NAME).o: $(OBJS) $(EXEC_NAME).c
	$(CC) $(MYCFLAGS) -c $(EXEC_NAME).c

clean:
	rm -f $(EXEC_NAME).o $(EXEC_NAME)
