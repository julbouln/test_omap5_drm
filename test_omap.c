#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>

#include <drm.h>

#include "xf86drm.h"
#include "xf86drmMode.h"

#include "omap_drmif.h"
#include "omap_drm.h"

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
	int omap_fd = open_card("/dev/dri/card1");
	struct omap_device *omap_dev = omap_device_new(omap_fd);
	if (!omap_dev) {
		return -1;
	}

	drmModePlaneRes *plane_resources;
	drmModePlane *ovr;

	plane_resources = drmModeGetPlaneResources(omap_fd);
	if (!plane_resources) {
		fprintf(stderr, "drmModeGetPlaneResources failed: %s\n",
		        strerror(errno));
		return -1;
	}

	if (plane_resources->count_planes < 1) {
		fprintf(stderr, "not enough planes\n");
		drmModeFreePlaneResources(plane_resources);
		return -1;
	}

	printf("PLANES: %d\n", plane_resources->count_planes);

	for (int p = 0; p < plane_resources->count_planes; p++) {
		ovr = drmModeGetPlane(omap_fd, plane_resources->planes[p]);
		if (!ovr) {
			fprintf(stderr, "drmModeGetPlane failed: %s\n",
			        strerror(errno));
			drmModeFreePlaneResources(plane_resources);
			return -1;
		}

		drmModeObjectPropertiesPtr props;
		props = drmModeObjectGetProperties(omap_fd, ovr->plane_id,
		                                   DRM_MODE_OBJECT_PLANE);


		if (props) {
			int i;

			for (i = 0; i < props->count_props; i++) {
				drmModePropertyPtr this_prop;
				this_prop = drmModeGetProperty(omap_fd, props->props[i]);
				printf("PROP: plane_id:%d name:%s count_values:%d\n", ovr->plane_id, this_prop->name, this_prop->count_values);
//				for(int j=0;j<this_prop->count_values;j++) {
					printf(" VALUE %d %d\n",i,props->prop_values[i]);
//				}
			}
		}
	}
	omap_device_del(omap_dev);
	close(omap_fd);

}