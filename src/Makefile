# throttled Makefile
# (C) 2010 - quest and lws

# Normal Compilation
CPPFLAGS = -O3 -Wno-deprecated
# Universal Binary for OS X
# CPPFLAGS = -O3 -arch ppc -arch ppc64 -arch i386 -arch x86_64 -isysroot /Developer/SDKs/MacOSX10.4u.sdk -mmacosx-version-min=10.4 -Wno-deprecated
CPP = c++

SRCS = throttled.cpp
INCS = throttled.h packetqueue.h wf2q+.h heap.h
ifeq ($(shell uname -s),FreeBSD)
LIBS = -pthread
else
LIBS = -lpthread
endif

APPNAME = throttled

throttled: ${SRCS} ${INCS}
	${CPP} -o ${APPNAME} ${CPPFLAGS} ${SRCS} ${LIBS}

clean:
	cd ..; rm ${APPNAME}
