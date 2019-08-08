#include "VKimpl.h"
#include "ref_import.h"
#include "vk_instance.h"
#include "vk_descriptor_sets.h"


void vk_createDescriptorPool(uint32_t numDes, VkDescriptorPool* const pPool)
{
    // Like command buffers, descriptor sets are allocated from a pool. 
    // So we must first create the Descriptor pool.

    ri.Printf(PRINT_ALL, " Create descriptor pool. \n");

    VkDescriptorPoolSize pool_size;
    // A combined image-sampler object is a sampler and an image paired
    // together, the same sampler is always used to sample from the image,
    // which can be more efficient on some architectures.
    pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    // TODO: make sure this numDes is justed.
    pool_size.descriptorCount = numDes;

    VkDescriptorPoolCreateInfo desc;


    desc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    desc.pNext = NULL;
    // ... CREATE_FREE_DESCRIPTOR ... indicates the application may free
    // individual descriptors allocated from the poor, so that should
    // be prepared for that.
    // used by the cinematic images ???
    // VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT specifies that descriptor sets
    // can return their individual allocations to the pool, all of vkAllocateDescriptorSets,
    // vkFreeDescriptorSets, and vkResetDescriptorPool are allowed. Otherwise, descriptor
    // sets allocated from the pool must not be individually freed back to the pool,
    // i.e. only vkAllocateDescriptorSets and vkResetDescriptorPool are allowed.
    desc.flags = 0;
		// VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    // VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; 
    // The maxSets field specifies the maximum total number of sets
    // that may be allocated from the pool.
    // TODO: make sure this numDes is justed.

    desc.maxSets = numDes;
    
    // specify the number of resource descriptors for each type of resource
    // that may be stored in the set.
    desc.poolSizeCount = 1;
    // pPoolSizes is a pointer to an array of VkDescriptorPoolSize structures,
    // each containing a descriptor type and number of descriptors of 
    // that type to be allocated in the pool.
    desc.pPoolSizes = &pool_size;


    // The descriptors are allocated from pools called descriptor pools.
    // Because descriptors for different types of resources are likely to
    // have similar data structures on any given implementation, pooling
    // the allocations used to store descriptors allows drivers to make 
    // efficient use of memory.
    VK_CHECK( qvkCreateDescriptorPool(vk.device, &desc, NULL, pPool) );
}


void vk_reset_descriptor_pool(void)
{
	// To return all descriptor sets allocated from a given pool to the pool,
	// rather than freeing individual descriptor sets
	// Resetting a descriptor pool recycles all of the resources from all of
	// the descriptor sets allocated from the descriptor pool back to the 
	// descriptor pool, and the descriptor sets are implicitly freed.
	VK_CHECK( qvkResetDescriptorPool(vk.device, vk.descriptor_pool, 0) );
}



void vk_destroy_descriptor_pool(void)
{
	// Destroying a pool object implicitly frees all objects allocated from that pool. 
	// Specifically, destroying VkCommandPool frees all VkCommandBuffer objects that 
	// were allocated from it, and destroying VkDescriptorPool frees all 
	// VkDescriptorSet objects that were allocated from it.

    ri.Printf(PRINT_ALL, " Destroy vk.set_layout, vk.descriptor_pool. \n");

    NO_CHECK( qvkDestroyDescriptorSetLayout(vk.device, vk.set_layout, NULL) ); 
    // You don't need to explicitly clean up descriptor sets,
    // because they will be automaticall freed when the descripter pool
    // is destroyed.
   	NO_CHECK( qvkDestroyDescriptorPool(vk.device, vk.descriptor_pool, NULL) );  
}


void vk_createDescriptorSetLayout(VkDescriptorSetLayout * const pSetLayout)
{
    // Each set has a layout, which describes the order and types of 
    // resources in the set. Two sets with the same layout are considered
    // to be compatible and interchangeable.
    //
    // At any time, the application can bind a new descriptor set to
    // the command buffer in any point that has an indentical layout.
    // The same descriptor set layouts can be used to craete multiple
    // pipelines. 

    VkDescriptorSetLayoutBinding descriptor_binding;
    // Each resource accessible to a shader is given a binding number.
    // "0" is the binding number of this entry and corresponds to 
    // a resource of the same binding number in the shader stages
    descriptor_binding.binding = 0;
    // descriptorType is a VkDescriptorType specifying which type of
    // resource descriptors are used for this binding.
    descriptor_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    // 1 is the number of descriptors contained in the binding,
    // accessed in a shader as an array
    descriptor_binding.descriptorCount = 1;
    // stageFlags member is a bitmask of VkShaderStageFlagBits specifying 
    // which pipeline shader stages can access a resource for this binding
    descriptor_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    // pImmutableSamplers affects initialization of samplers. 
	// If descriptorType specifies a VK_DESCRIPTOR_TYPE_SAMPLER or 
    // VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER type descriptor,
	// then pImmutableSamplers can be used to initialize a set of
	// immutable samplers. Immutable samplers are permanently bound
	// into the set layout; later binding a sampler into an immutable
	// sampler slot in a descriptor set is not allowed. 
	// 
	// If pImmutableSamplers is not NULL, then it is considered to be
	// a pointer to an array of sampler handles that will be consumed 
	// by the set layout and used for the corresponding binding.
    // If pImmutableSamplers is NULL, then the sampler slots are dynamic 
    // and sampler handles must be bound into descriptor sets using this
	// layout. If descriptorType is not one of these descriptor types, 
    // then pImmutableSamplers is ignored.
    descriptor_binding.pImmutableSamplers = NULL;


    VkDescriptorSetLayoutCreateInfo desc;
    desc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    desc.pNext = NULL;
	// flags must be a valid combination of VkDescriptorSetLayoutCreateFlagBits values
    desc.flags = 0;
    // Resource are bound to binding points in the descriptor set.
    //
    // The number of the binding points that the set will contain
    desc.bindingCount = 1;
    // a pointer to an array containing their descriptions respectively.
    desc.pBindings = &descriptor_binding;

    // To create descriptor set layout objects
    VK_CHECK( qvkCreateDescriptorSetLayout(vk.device, &desc, NULL, pSetLayout) );
	
	ri.Printf(PRINT_ALL, " Descriptor set layout created . \n");
}


void vk_allocOneDescptrSet(VkDescriptorSet * const pSetRet)
{
    VkDescriptorSetAllocateInfo descSetAllocInfo;
    descSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descSetAllocInfo.pNext = NULL;
    descSetAllocInfo.descriptorPool = vk.descriptor_pool;
    descSetAllocInfo.descriptorSetCount = 1;
    // Note that we have to provide the descriptor set layout that 
    // This layout describes how the descriptor set is to be allocated.
    descSetAllocInfo.pSetLayouts = &vk.set_layout;

    VK_CHECK( qvkAllocateDescriptorSets(vk.device, &descSetAllocInfo, pSetRet) );
}
