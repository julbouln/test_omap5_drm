OBJS = write_bmp.o etnaviv.o viv2d.o
CFLAGS = -std=c99 -D_GNU_SOURCE -I/usr/include/drm

%.o: %.c
	$(CC) $(CFLAGS) $(EXTRA_FLAGS) -c -o $@ $<

all: test_malloc test_viv2d

test_malloc: $(OBJS) test_malloc.o
	gcc $(CFLAGS) $(OBJS) test_malloc.o -o test_malloc -ldrm -ldrm_omap
test_omap: $(OBJS) test_omap.o
	gcc $(CFLAGS) $(OBJS) test_omap.o -o test_omap -ldrm -ldrm_omap
test_viv2d: $(OBJS) test_viv2d.o
	gcc $(CFLAGS) $(OBJS) test_viv2d.o -o test_viv2d -L/usr/local/lib -ldrm -lc

clean:
	rm -f *.o
	rm -f test_malloc
	rm -f test_viv2d
	rm -f out/*