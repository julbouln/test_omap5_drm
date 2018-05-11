#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <drm.h>

#include "xf86drm.h"
#include "etnaviv_drmif.h"
#include "etnaviv_drm.h"

#include "omap_drmif.h"
#include "omap_drm.h"

#include "viv2d.h"

#define REPEAT 1024*1024

#define ALIGN(val, align)	(((val) + (align) - 1) & ~((align) - 1))

/* Return 1 if the difference is negative, otherwise 0.  */
int timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1)
{
	long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
	result->tv_sec = diff / 1000000;
	result->tv_usec = diff % 1000000;

	return (diff < 0);
}

void timeval_print(struct timeval *tv)
{
	char buffer[30];
	time_t curtime;

	printf("%ld.%06ld", tv->tv_sec, tv->tv_usec);
	curtime = tv->tv_sec;
	strftime(buffer, 30, "%m-%d-%Y  %T", localtime(&curtime));
	printf(" = %s.%06ld\n", buffer, tv->tv_usec);
}

int open_card(const char *card) {
	int fd;
	drmVersionPtr version;
	fd = open(card, O_RDWR);
	if (fd < 0)
		return 1;

	version = drmGetVersion(fd);
	if (version) {
		printf("Version: %d.%d.%d\n", version->version_major,
		       version->version_minor, version->version_patchlevel);
		printf("  Name: %s\n", version->name);
		printf("  Date: %s\n", version->date);
		printf("  Description: %s\n", version->desc);
		drmFreeVersion(version);
	}
	return fd;
}

int main(int argc, char *argv[]) {
	struct etna_device *etna_dev;

	int repeat = 1024 * 1024;
	int bufsize = 4096;

	if (argc > 1) {
		repeat = atoi(argv[1]);
	}
	if (argc > 2) {
		bufsize = atoi(argv[2]);
	}

	int etna_fd = open_card("/dev/dri/card0");
	etna_dev = etna_device_new(etna_fd);
	if (!etna_dev) {
		return -1;
	}

	int omap_fd = open_card("/dev/dri/card1");

	struct omap_device *omap_dev = omap_device_new(omap_fd);
	if (!omap_dev) {
		return -1;
	}
viv2d_device v2d;
v2d.fd = etna_fd;
v2d.dev = etna_dev;

	struct timeval tvBegin, tvEnd, tvDiff;

	gettimeofday(&tvBegin, NULL);
	for (int i = 0; i < repeat; i++) {
		char *buf = (char *)malloc(bufsize);
		free(buf);
	}
	gettimeofday(&tvEnd, NULL);

	timeval_subtract(&tvDiff, &tvEnd, &tvBegin);
	printf("malloc for create %d buffers of %d bytes: %ld.%06ld\n", repeat, bufsize, tvDiff.tv_sec, tvDiff.tv_usec);

	gettimeofday(&tvBegin, NULL);
	for (int i = 0; i < repeat; i++) {
		struct etna_bo *bo;
		bo = etna_bo_new(etna_dev, bufsize, ETNA_BO_UNCACHED);
		etna_bo_del(bo);
	}
	gettimeofday(&tvEnd, NULL);

	timeval_subtract(&tvDiff, &tvEnd, &tvBegin);
	printf("etnaviv for create %d buffers of %d bytes: %ld.%06ld\n", repeat, bufsize, tvDiff.tv_sec, tvDiff.tv_usec);

	gettimeofday(&tvBegin, NULL);
	for (int i = 0; i < repeat; i++) {
		char *buf = (char *)aligned_alloc(4096, ALIGN(bufsize,4096));
		struct etna_bo *bo;
		bo = etna_bo_from_usermem_prot(etna_dev, buf, ALIGN(bufsize,4096), (ETNA_USERPTR_READ | ETNA_USERPTR_WRITE));
		etna_bo_del(bo);
		free(buf);
	}
	gettimeofday(&tvEnd, NULL);

	timeval_subtract(&tvDiff, &tvEnd, &tvBegin);
	printf("etnaviv usermem for create %d buffers of %d bytes: %ld.%06ld\n", repeat, bufsize, tvDiff.tv_sec, tvDiff.tv_usec);

	gettimeofday(&tvBegin, NULL);
	for (int i = 0; i < repeat; i++) {
		struct omap_bo *bo;
		bo = omap_bo_new(omap_dev, bufsize, OMAP_BO_WC);
		omap_bo_del(bo);
	}
	gettimeofday(&tvEnd, NULL);

	// diff
	timeval_subtract(&tvDiff, &tvEnd, &tvBegin);
	printf("omap for create %d buffers of %d bytes: %ld.%06ld\n", repeat, bufsize, tvDiff.tv_sec, tvDiff.tv_usec);

	etna_device_del(etna_dev);
	omap_device_del(omap_dev);
	close(etna_fd);
	close(omap_fd);
	return 0;
}