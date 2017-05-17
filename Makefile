CC := gcc
CLIENTOBJS := lib/vroom.pb.o lib/vrms_client.o lib/vrms_geometry.o lib/esm.o
SERVEROBJS := lib/vroom.pb.o lib/vrms_server.o lib/opengl_stereo.o lib/ogl_shader_loader.o lib/esm.o
CFLAGS := -Wall -Werror -ggdb
EXTCOM := -lrt -lev -lprotobuf-c -lconfig -lm

EXTGL := -lpthread -lGL -lGLU -lglut
INCLUDEDIRS :=
LINKDIRS :=
PREPROC :=

rpi_egl : EXTGL := -lbcm_host -lEGL -lGLESv2
rpi_egl : INCLUDEDIRS := -I/opt/vc/include
rpi_egl : LINKDIRS := -L/opt/vc/lib
rpi_egl : PREPROC := -DRASPBERRYPI

all: server client

server: src/server.c $(SERVEROBJS)
	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) -Iinclude $(INCLUDEDIRS) $(EXTCOM) $(EXTGL) -o $@ $(SERVEROBJS) src/array-heap.c $<

client: src/client.c $(CLIENTOBJS)
	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) -Iinclude $(INCLUDEDIRS) $(EXTCOM) -o $@ $(CLIENTOBJS) $<

lib/vroom.pb.o: src/vroom.pb-c.c include/vroom.pb-c.h
	$(CC) $(CFLAGS) $(PREPROC) -Iinclude -c -o $@ $<

lib/vrms_server.o: src/vrms_server.c
	$(CC) $(CFLAGS) $(PREPROC) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

lib/vrms_client.o: src/vrms_client.c
	$(CC) $(CFLAGS) $(PREPROC) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

lib/vrms_geometry.o: src/vrms_geometry.c
	$(CC) $(CFLAGS) $(PREPROC) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

lib/esm.o: src/esm.c
	$(CC) $(CFLAGS) $(PREPROC) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

lib/opengl_stereo.o: src/opengl_stereo.c
	$(CC) $(CFLAGS) $(PREPROC) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

lib/ogl_shader_loader.o: src/ogl_shader_loader.c
	$(CC) $(CFLAGS) $(PREPROC) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

src/vroom.pb-c.c: vroom-protobuf
include/vroom.pb-c.h: vroom-protobuf
vroom-protobuf: vroom.proto
	protoc-c --c_out=. vroom.proto
	mv vroom.pb-c.c src/
	mv vroom.pb-c.h include/

clean:
	rm -f lib/*
	rm -f server client
