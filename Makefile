TARGET = SimpleMP3
OBJS = main.o
CFLAGS = -O2 -G0 -Wall
LIBS = -lpspmp3 -lpspaudio -lpsputility -lpspdebug -lpspdisplay -lpspge -lpspctrl
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Simple MP3
PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
