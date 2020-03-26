#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

static int is_kms(int fd)
{
    drmVersionPtr version;
    drmModeResPtr res;
    int has_connectors;

    version = drmGetVersion(fd);
    if (!version)
        return 0;
    drmFreeVersion(version);

    res = drmModeGetResources(fd);
    if (!res)
        return 0;

    has_connectors = res->count_connectors > 0;
    drmModeFreeResources(res);

    return has_connectors;
}

/*
 * We simply use /dev/dri/card0 here but the user can specify another path on
 * the command line.
 */

int main(int argc, char **argv)
{
	int ret, fd;
	const char *card;
	uint64_t has_dumb = 0 ;

	/* check which DRM device to open */
	if (argc > 1)
		card = argv[1];
	else
		card = "/dev/dri/card0";

	fprintf(stdout, "using card '%s'\n", card);

	// open the DRM device 
	fd = open(card, O_RDWR | O_CLOEXEC);
	if (fd < 0)
	{
		ret = -errno;
		fprintf(stderr, "cannot open '%s': %m\n", card);
		return ret;
	}

	// After opening the file, we also check for the DRM_CAP_DUMB_BUFFER capability.
	// If the driver supports this capability, we can create simple memory-mapped
	// buffers without any driver-dependent code. As we want to avoid any radeon,
	// nvidia, intel, etc. specific code, we depend on DUMB_BUFFERs here.
	if (drmGetCap(fd, DRM_CAP_DUMB_BUFFER, &has_dumb) < 0 || (has_dumb == 0))
	{
		fprintf(stdout, "drm device '%s' does not support dumb buffers\n", card);
		close(fd);
		return -EOPNOTSUPP;
	}
	else
	{
		fprintf(stdout, "drm device '%s' support dumb buffers\n", card);
	}


	if(is_kms(fd))
	{
		fprintf(stdout, " card '%s' is a KMS device.\n", card);
	}
	else
	{
		fprintf(stdout, " card '%s' is NOT a KMS device.\n", card);	
	}

	ret = 0;


	close(fd);

	return 0;
}
