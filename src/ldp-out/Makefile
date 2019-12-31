# sub Makefile for ldp-out

%.d : %.cpp
	set -e; $(CXX) -MM $(CFLAGS) $< \
		| sed 's^\($*\)\.o[ :]*^\1.o $@ : ^g' > $@; \
		[ -s $@ ] || rm -f $@

OBJS = ldp.o framemod.o ldp-vldp.o ldp-vldp-audio.o ldp-vldp-gl.o

.SUFFIXES:	.cpp

all:	${OBJS}

include $(OBJS:.o=.d)

.cpp.o:
	${CXX} ${CFLAGS} -c $< -o $@

clean:
	rm -f ${OBJS} *.d

