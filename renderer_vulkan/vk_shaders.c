#include "vk_instance.h"
#include "vk_shaders.h"
#include "ref_import.h" 
// Vulkan has to be specified in a bytecode format which is called SPIR-V
// and is designed to be work with both Vulkan and OpenCL.
//
// The graphics pipeline is the sequence of the operations that take the
// vertices and textures of your meshes all way to the pixels in the
// render targets.




struct StageShaderModuleManager{
	//
	// Shader modules.
	//
	VkShaderModule single_texture_vs;
	VkShaderModule single_texture_clipping_plane_vs;
	VkShaderModule single_texture_fs;
	VkShaderModule multi_texture_vs;
	VkShaderModule multi_texture_clipping_plane_vs;
	VkShaderModule multi_texture_mul_fs;
	VkShaderModule multi_texture_add_fs;
};



static struct StageShaderModuleManager s_ShaderModules;


// The function will take a buffer with the bytecode and the size of the buffer as parameter
// and craete VkShaderModule from it

static void create_shader_module(const unsigned char* pBytes, const int count, VkShaderModule* pVkShaderMod)
{
	if (count % 4 != 0) {
		ri.Error(ERR_FATAL, "Vulkan: SPIR-V binary buffer size is not multiple of 4");
	}
	VkShaderModuleCreateInfo desc;
	desc.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	desc.pNext = NULL;
	desc.flags = 0;
	desc.codeSize = count;
	desc.pCode = (const uint32_t*)pBytes;
			   
	VK_CHECK( qvkCreateShaderModule(vk.device, &desc, NULL, pVkShaderMod) );
}


// The VkShaderModule object is just a dumb wrapper around the bytecode buffer
// The shaders aren't linked to each other yet and they haven't even been given
// a purpose yet.
void vk_loadShaderModules(void)
{
    extern unsigned char single_texture_vert_spv[];
    extern int single_texture_vert_spv_size;
    
    create_shader_module(single_texture_vert_spv, single_texture_vert_spv_size,
            &s_ShaderModules.single_texture_vs);

    extern unsigned char single_texture_clipping_plane_vert_spv[];
    extern int single_texture_clipping_plane_vert_spv_size;
    
    create_shader_module(single_texture_clipping_plane_vert_spv, single_texture_clipping_plane_vert_spv_size,
            &s_ShaderModules.single_texture_clipping_plane_vs);

    extern unsigned char single_texture_frag_spv[];
    extern int single_texture_frag_spv_size;
    
    create_shader_module(single_texture_frag_spv, single_texture_frag_spv_size,
            &s_ShaderModules.single_texture_fs);

    extern unsigned char multi_texture_vert_spv[];
    extern int multi_texture_vert_spv_size;
    
    create_shader_module(multi_texture_vert_spv, multi_texture_vert_spv_size,
            &s_ShaderModules.multi_texture_vs);

    extern unsigned char multi_texture_clipping_plane_vert_spv[];
    extern int multi_texture_clipping_plane_vert_spv_size;
    create_shader_module(multi_texture_clipping_plane_vert_spv, multi_texture_clipping_plane_vert_spv_size,
            &s_ShaderModules.multi_texture_clipping_plane_vs);

    extern unsigned char multi_texture_mul_frag_spv[];
    extern int multi_texture_mul_frag_spv_size;
    create_shader_module(multi_texture_mul_frag_spv, multi_texture_mul_frag_spv_size,
            &s_ShaderModules.multi_texture_mul_fs);

    extern unsigned char multi_texture_add_frag_spv[];
    extern int multi_texture_add_frag_spv_size;
    create_shader_module(multi_texture_add_frag_spv, multi_texture_add_frag_spv_size,
            &s_ShaderModules.multi_texture_add_fs);
}



void vk_specifyShaderModule(const enum Vk_Shader_Type shader_type, const VkBool32 isClippingPlane,
                            VkShaderModule* const vs, VkShaderModule* const fs)
{
    // Specify the shader module containing the shader code, and the function
    // to invoke. This means that it's possible to combine multiple fragment
    // shaders into a single shader module and use different entry points 
    // to differnentiate between their behaviors
    // In this case we'll stick to the standard main.

    if(isClippingPlane)
    {
        switch(shader_type)
        {
            case ST_MULTI_TEXURE_ADD:
            {
                *vs = s_ShaderModules.multi_texture_clipping_plane_vs;
                *fs = s_ShaderModules.multi_texture_add_fs;
            }break;

            case ST_MULTI_TEXURE_MUL:
            {
                *vs = s_ShaderModules.multi_texture_clipping_plane_vs;
                *fs = s_ShaderModules.multi_texture_mul_fs;
            }break;

            case ST_SINGLE_TEXTURE:
            {
                *vs = s_ShaderModules.single_texture_clipping_plane_vs;
                *fs = s_ShaderModules.single_texture_fs;
            }break;
        }
    }
    else
    {
        switch(shader_type)
        {
            case ST_MULTI_TEXURE_ADD:
            {
                *vs = s_ShaderModules.multi_texture_vs;
                *fs = s_ShaderModules.multi_texture_add_fs;
            }break;

            case ST_MULTI_TEXURE_MUL:
            {
                *vs = s_ShaderModules.multi_texture_vs;
                *fs = s_ShaderModules.multi_texture_mul_fs;
            }break;

            case ST_SINGLE_TEXTURE:
            {
                *vs = s_ShaderModules.single_texture_vs;
                *fs = s_ShaderModules.single_texture_fs;
            }break;
        }

    }
}



void vk_destroyShaderModules(void)
{
	qvkDestroyShaderModule(vk.device, s_ShaderModules.single_texture_vs, NULL);
	qvkDestroyShaderModule(vk.device, s_ShaderModules.single_texture_clipping_plane_vs, NULL);
	qvkDestroyShaderModule(vk.device, s_ShaderModules.single_texture_fs, NULL);
	qvkDestroyShaderModule(vk.device, s_ShaderModules.multi_texture_vs, NULL);
	qvkDestroyShaderModule(vk.device, s_ShaderModules.multi_texture_clipping_plane_vs, NULL);
	qvkDestroyShaderModule(vk.device, s_ShaderModules.multi_texture_mul_fs, NULL);
	qvkDestroyShaderModule(vk.device, s_ShaderModules.multi_texture_add_fs, NULL);

    memset(&s_ShaderModules, 0, sizeof(s_ShaderModules));
}
