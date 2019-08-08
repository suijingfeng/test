#include "tr_cvar.h"

#include "VKimpl.h"
#include "vk_instance.h"
#include "ref_import.h" 

// have to use this crap for backward Compatibility.
// I want keep it locally, as it belong to OpenGL, not vulkan

typedef struct vidmode_s
{
    const char *description;
    int         width, height;
	float		pixelAspect;		// pixel width / height
} vidmode_t;


static glconfig_t glConfig;
static cvar_t* r_customwidth;
static cvar_t* r_customheight;
static cvar_t* r_customaspect;


static const vidmode_t r_vidModes[] =
{
	{ "Mode  0: 320x240",		320,	240,	1 },
	{ "Mode  1: 400x300",		400,	300,	1 },
	{ "Mode  2: 512x384",		512,	384,	1 },
	{ "Mode  3: 640x480 (480p)",	640,	480,	1 },
	{ "Mode  4: 800x600",		800,	600,	1 },
	{ "Mode  5: 960x720",		960,	720,	1 },
	{ "Mode  6: 1024x768",		1024,	768,	1 },
	{ "Mode  7: 1152x864",		1152,	864,	1 },
	{ "Mode  8: 1280x1024",		1280,	1024,	1 },
	{ "Mode  9: 1600x1200",		1600,	1200,	1 },
	{ "Mode 10: 2048x1536",		2048,	1536,	1 },
	{ "Mode 11: 856x480",		856,	480,	1 }, // Q3 MODES END HERE AND EXTENDED MODES BEGIN
	{ "Mode 12: 1280x720 (720p)",	1280,	720,	1 },
	{ "Mode 13: 1280x768",		1280,	768,	1 },
	{ "Mode 14: 1280x800",		1280,	800,	1 },
	{ "Mode 15: 1280x960",		1280,	960,	1 },
	{ "Mode 16: 1360x768",		1360,	768,	1 },
	{ "Mode 17: 1366x768",		1366,	768,	1 }, // yes there are some out there on that extra 6
	{ "Mode 18: 1360x1024",		1360,	1024,	1 },
	{ "Mode 19: 1400x1050",		1400,	1050,	1 },
	{ "Mode 20: 1400x900",		1400,	900,	1 },
	{ "Mode 21: 1600x900",		1600,	900,	1 },
	{ "Mode 22: 1680x1050",		1680,	1050,	1 },
	{ "Mode 23: 1920x1080 (1080p)",	1920,	1080,	1 },
	{ "Mode 24: 1920x1200",		1920,	1200,	1 },
	{ "Mode 25: 1920x1440",		1920,	1440,	1 },
    { "Mode 26: 2560x1080",		2560,	1080,	1 },
    { "Mode 27: 2560x1600",		2560,	1600,	1 },
	{ "Mode 28: 3840x2160 (4K)",	3840,	2160,	1 }
};
static const int s_numVidModes = 29;


void R_DisplayResolutionList_f( void )
{
	uint32_t i;

	ri.Printf( PRINT_ALL, "\n" );
	for ( i = 0; i < s_numVidModes; i++ )
	{
		ri.Printf( PRINT_ALL, "%s\n", r_vidModes[i].description );
	}
	ri.Printf( PRINT_ALL, "\n" );
}


void R_SetWinMode(int mode, unsigned int width, unsigned int height, unsigned int hz)
{
	if (mode == -2)
	{
        // use desktop video resolution
        glConfig.vidWidth = width;
        glConfig.vidHeight = height;
        glConfig.windowAspect = (float)width / (float)height;
        glConfig.displayFrequency = hz;
        glConfig.isFullscreen = 1;

        vk.renderArea.offset.x = 0;
        vk.renderArea.offset.y = 0;
        vk.renderArea.extent.width = width;
        vk.renderArea.extent.height = height;
    }
	else if ( mode == -1 )
    {
		glConfig.vidWidth = r_customwidth->integer;
		glConfig.vidHeight = r_customheight->integer;
        glConfig.windowAspect = (float)width / (float)height;
        glConfig.displayFrequency = hz;
        glConfig.isFullscreen = 1;

        vk.renderArea.offset.x = 0;
        vk.renderArea.offset.y = 0;
        vk.renderArea.extent.width = r_customwidth->integer;
        vk.renderArea.extent.height = r_customheight->integer;
	}
    else if ( (mode < s_numVidModes) && (mode > 3))
    {
        glConfig.vidWidth = r_vidModes[mode].width;
        glConfig.vidHeight = r_vidModes[mode].height;
        glConfig.windowAspect = (float)r_vidModes[mode].width / ( r_vidModes[mode].height * r_vidModes[mode].pixelAspect );
        glConfig.displayFrequency = hz;
        glConfig.isFullscreen = 0;
        
        vk.renderArea.offset.x = 0;
        vk.renderArea.offset.y = 0;
        vk.renderArea.extent.width = glConfig.vidWidth;
        vk.renderArea.extent.height = glConfig.vidHeight;
    }
    

    if ( mode < -2 || mode >= s_numVidModes || glConfig.vidWidth <=0 || glConfig.vidHeight<=0)
    {

        glConfig.vidWidth = 640;
        glConfig.vidHeight = 480;
        glConfig.displayFrequency = 60;
        glConfig.windowAspect = 1.333333f;

        vk.renderArea.offset.x = 0;
        vk.renderArea.offset.y = 0;
        vk.renderArea.extent.width = 640;
        vk.renderArea.extent.height = 480;
	}

	ri.Printf(PRINT_ALL,  "MODE: %d, %d x %d, refresh rate: %dhz\n",
        mode, glConfig.vidWidth, glConfig.vidHeight, glConfig.displayFrequency);
}


void R_InitDisplayResolution( void )
{
    // leilei - -2 is so convenient for modern day PCs

    r_customwidth = ri.Cvar_Get( "r_customwidth", "960", CVAR_ARCHIVE | CVAR_LATCH );
    r_customheight = ri.Cvar_Get( "r_customheight", "540", CVAR_ARCHIVE | CVAR_LATCH );
    r_customaspect = ri.Cvar_Get( "r_customaspect", "1.78", CVAR_ARCHIVE | CVAR_LATCH );
}

void glConfig_Clear(void)
{
    memset(&glConfig, 0, sizeof(glConfig));
}


//IN: a pointer to the glConfig struct
void glConfig_Get(glconfig_t * const pCfg)
{
	*pCfg = glConfig;
}

glconfig_t * glConfig_getAddressOf(void)
{
	return &glConfig;
}

void glConfig_Init(void)
{
    ri.Printf(PRINT_ALL,  "--- R_glConfigInit() ---\n");

    // These values force the UI to disable driver selection
	glConfig.driverType = GLDRV_ICD;
	glConfig.hardwareType = GLHW_GENERIC;

    // Only using SDL_SetWindowBrightness to determine if hardware gamma is supported
    glConfig.deviceSupportsGamma = qtrue;

    glConfig.textureEnvAddAvailable = 0; // not used
    glConfig.textureCompression = TC_NONE; // not used
	// init command buffers and SMP
	glConfig.stereoEnabled = 0;
	glConfig.smpActive = qfalse; // not used

    // hardcode it
    glConfig.colorBits = 32;
    glConfig.depthBits = 24;
    glConfig.stencilBits = 8;
}

// ==================================================
// ==================================================


void glConfig_FillString( void )
{
    ri.Printf( PRINT_ALL, "\nActive 3D API: Vulkan\n" );

    // To query general properties of physical devices once enumerated
    VkPhysicalDeviceProperties props;
    qvkGetPhysicalDeviceProperties(vk.physical_device, &props);

    uint32_t major = VK_VERSION_MAJOR(props.apiVersion);
    uint32_t minor = VK_VERSION_MINOR(props.apiVersion);
    uint32_t patch = VK_VERSION_PATCH(props.apiVersion);

    const char* device_type;
    if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        device_type = " INTEGRATED_GPU";
    else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        device_type = " DISCRETE_GPU";
    else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
        device_type = " VIRTUAL_GPU";
    else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU)
        device_type = " CPU";
    else
        device_type = " Unknown";

    const char* vendor_name = "unknown";
    if (props.vendorID == 0x1002) {
        vendor_name = "Advanced Micro Devices, Inc.";
    } else if (props.vendorID == 0x10DE) {
        vendor_name = "NVIDIA";
    } else if (props.vendorID == 0x8086) {
        vendor_name = "Intel Corporation";
    }


    char tmpBuf[128] = {0};
    snprintf(tmpBuf, 128, " Vk api version: %d.%d.%d ", major, minor, patch);
    strncpy( glConfig.version_string, tmpBuf, sizeof( glConfig.version_string ) );
	strncpy( glConfig.vendor_string, vendor_name, sizeof( glConfig.vendor_string ) );
    strncpy( glConfig.renderer_string, props.deviceName, sizeof( glConfig.renderer_string ) );
    strcat(glConfig.renderer_string, device_type);

    if (*glConfig.renderer_string && 
            glConfig.renderer_string[strlen(glConfig.renderer_string) - 1] == '\n')
         glConfig.renderer_string[strlen(glConfig.renderer_string) - 1] = 0;  
    
	/////////////////// extention /////////////////////

    uint32_t nDevExts = 0;

    // To query the extensions available to a given physical device
    VK_CHECK( qvkEnumerateDeviceExtensionProperties( vk.physical_device, NULL, &nDevExts, NULL) );

    assert(nDevExts > 0);

    VkExtensionProperties* pDevExt = 
        (VkExtensionProperties *) malloc(sizeof(VkExtensionProperties) * nDevExts);

    qvkEnumerateDeviceExtensionProperties(
            vk.physical_device, NULL, &nDevExts, pDevExt);

    int i = 0;
    uint32_t indicator = 0;
    
    // There much more device extentions, beyound UI driver info can display
    for (i = 0; i < nDevExts; ++i)
    {   
        uint32_t len = (uint32_t)strlen(pDevExt[i].extensionName);
        memcpy(glConfig.extensions_string + indicator, pDevExt[i].extensionName, len);
        indicator += len;
        glConfig.extensions_string[indicator++] = ' ';
    }
    free(pDevExt);
}

