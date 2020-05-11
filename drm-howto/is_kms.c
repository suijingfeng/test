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


static int prefer_shadow(int fd)
{
	uint64_t value = 0;
	int ret =0;
	int prefer_shadow = 0;

	ret = drmGetCap(fd, DRM_CAP_DUMB_PREFER_SHADOW, &value);
	if (0 == ret)
	{
		prefer_shadow = !!value;
	}

	fprintf(stdout, "preferr shadow : '%s'\n", prefer_shadow ? "Yes" : "No");
}


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


static int prime_capable(int fd)
{
	uint64_t value = 0;

	return ((drmGetCap(fd, DRM_CAP_PRIME, &value) == 0) &&
			(value & DRM_PRIME_CAP_EXPORT));
}


static int isAsyncPresentCapable(int fd)
{
	uint64_t value = 0;
	int ret = 0;
	ret = drmGetCap(fd, DRM_CAP_ASYNC_PAGE_FLIP, &value);

	if (ret == 0 && value == 1)
	{
		return 1;
	}
	return 0;
}

/*
 * Check that what we opened was a master or a master-capable FD
 * by setting the version of the interface we'll use to talk to it.
 */
static int is_master(int fd)
{
	drmSetVersion sv;

	sv.drm_di_major = 1;
	sv.drm_di_minor = 4;
	sv.drm_dd_major = -1;
	sv.drm_dd_minor = -1;

	return drmSetInterfaceVersion(fd, &sv) == 0;
}


static void GetDRMVersion(int fd)
{
	drmVersionPtr v = drmGetVersion(fd);

	fprintf(stdout, "fd: '%d'\n", fd);
	fprintf(stdout, "name: '%s'\n", v->name);
	fprintf(stdout, "date: '%s'\n", v->date);
	fprintf(stdout, "desc: '%s'\n", v->desc);
	fprintf(stdout, "version: %d.%d.%d\n", 
		v->version_major, v->version_minor, v->version_patchlevel);

	drmFreeVersion(v);
}





/*
 * We simply use /dev/dri/card0 here but the user can specify another path
 * on the command line.
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
	fd = open(card, O_RDWR | O_CLOEXEC, 0);
	if (fd < 0)
	{
		ret = -errno;
		fprintf(stderr, "cannot open '%s': %m\n", card);
		return ret;
	}

    //
    char * deviceName = drmGetDeviceNameFromFd(fd);
    fprintf(stdout, "drmGetDeviceNameFromFd(%d) = '%s'\n", fd, deviceName);



    if( drmIsMaster(fd) )
            fprintf(stdout, "%d is master. \n", fd);
    else
            fprintf(stdout, "%d is NOT master. \n", fd);



    char * deviceName2 = drmGetDeviceNameFromFd(fd);
    fprintf(stdout, "drmGetDeviceNameFromFd2(%d) = '%s'\n", fd, deviceName2);


	GetDRMVersion(fd);

	// After opening the file, we also check for the DRM_CAP_DUMB_BUFFER capability.
	// If the driver supports this capability, we can create simple memory-mapped
	// buffers without any driver-dependent code. As we want to avoid any radeon,
	// nvidia, intel, etc. specific code, we depend on DUMB_BUFFERs here.
	if (drmGetCap(fd, DRM_CAP_DUMB_BUFFER, &has_dumb) < 0 || (has_dumb == 0))
	{
		fprintf(stdout, "drm device '%s' does not support dumb buffers\n", card);
	}
	else
	{
		fprintf(stdout, "drm device '%s' support dumb buffers\n", card);
	}

	if(is_kms(fd))
	{
		fprintf(stdout, "Card '%s' is a KMS device.\n", card);
	}
	else
	{
		fprintf(stdout, "Card '%s' is NOT a KMS device.\n", card);
	}

	if(prime_capable(fd))
	{
		fprintf(stdout, "Card '%s' is PRIME capable.\n", card);
	}
	else
	{
		fprintf(stdout, "Card '%s' is NOT PRIME capable.\n", card);
	}

	if(is_master(fd))
	{
		fprintf(stdout, "Card '%s' is master-capable FD.\n", card);
	}
	else
	{
		fprintf(stdout, "Card '%s' is NOT master-capable FD.\n", card);
	}

	if(isAsyncPresentCapable(fd))
	{
		fprintf(stdout, "Card '%s' is Async Present Capable.\n", card);
	}
	else
	{
		fprintf(stdout, "Card '%s' is NOT Async Present Capable.\n", card);
	}

	prefer_shadow(fd);

	ret = 0;
	close(fd);

	return ret;
}
