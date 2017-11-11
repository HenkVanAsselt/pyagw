#makefile for APRS Server 2.0.9
CC = g++
CPU = -m386
DEPENDS = utils.o history.o serialp.o cpqueue.o aprsString.o mic_e.o queryResp.o
#
aprsd: aprsd.o  $(DEPENDS) constant.h
	$(CC) $(CPU) -o  aprsd aprsd.cpp $(DEPENDS) lib/validate.o /usr/lib/libpthread.so /usr/lib/libcrypt.so
aprsd.o: aprsd.cpp $(DEPENDS)
	$(CC) $(CPU) -c aprsd.cpp
cpqueue.o: utils.o cpqueue.cpp cpqueue.h constant.h
	$(CC) $(CPU) -c cpqueue.cpp
serialp.o: serialp.cpp serialp.h	constant.h
	$(CC) $(CPU) -c serialp.cpp
history.o: utils.o history.cpp history.h constant.h
	$(CC) $(CPU) -c history.cpp
utils.o: utils.h	utils.cpp utils.h	constant.h
	$(CC) $(CPU) -c utils.cpp
aprsString.o: aprsString.cpp aprsString.h  constant.h
	$(CC) $(CPU) -c aprsString.cpp
mic_e.o: mic_e.cpp mic_e.h
	$(CC) $(CPU) -c mic_e.cpp
	
queryResp.o: queryResp.cpp queryResp.h constant.h
	$(CC) $(CPU) -c queryResp.cpp
	
clean:
	rm *.o
	rm aprsd

install:
	INSTALL
       	
