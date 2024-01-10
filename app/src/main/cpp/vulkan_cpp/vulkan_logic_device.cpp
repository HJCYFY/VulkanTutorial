//
// Created by hj6231 on 2023/12/20.
//

#include "vulkan_logic_device.h"

VulkanLogicDevice::VulkanLogicDevice(VkPhysicalDevice physical_device, VkDevice device) :
        physical_device_(physical_device), device_(device) {

}

VulkanLogicDevice::~VulkanLogicDevice() {
    if (device_ != nullptr) {
        vkDestroyDevice(device_, nullptr);
        device_ = nullptr;
    }
}

VulkanQueue* VulkanLogicDevice::GetDeviceQueue(uint32_t queue_family, int queue_index) const {
    VkQueue queue;
    vkGetDeviceQueue(device_, queue_family, queue_index, &queue);
    return new VulkanQueue(queue);
}

VulkanSwapChain* VulkanLogicDevice::CreateSwapChain(const VkSwapchainCreateInfoKHR* info) const {
    VkSwapchainKHR swap_chain;
    VkResult ret = vkCreateSwapchainKHR(device_, info, nullptr, &swap_chain);
    if (ret == VK_SUCCESS) {
        return new VulkanSwapChain(device_, swap_chain);
    }
    return nullptr;
}

void VulkanLogicDevice::DestroySwapChain(VulkanSwapChain** swap_chain) {
    if (*swap_chain) {
        delete (*swap_chain);
        *swap_chain = nullptr;
    }
}

VkResult VulkanLogicDevice::AcquireNextImageKHR(VkSwapchainKHR swap_chain,
                                                  uint64_t timeout,
                                                  VkSemaphore semaphore,
                                                  VkFence fence,
                                                  uint32_t* image_index) const {
    return vkAcquireNextImageKHR(device_, swap_chain, timeout, semaphore, fence, image_index);
}

VulkanImageView* VulkanLogicDevice::CreateImageView(const VkImageViewCreateInfo* info) const {
    VkImageView image_view;
    VkResult ret = vkCreateImageView(device_, info, nullptr, &image_view);
    if (ret == VK_SUCCESS) {
        return new VulkanImageView(device_, image_view);
    }
    return nullptr;
}

void VulkanLogicDevice::DestroyImageView(VulkanImageView** view) {
    if(*view) {
        delete (*view);
        *view = nullptr;
    }
}

VulkanRenderPass* VulkanLogicDevice::CreateRenderPass(const VkRenderPassCreateInfo *info) {
    VkRenderPass render_pass;
    VkResult ret = vkCreateRenderPass(device_, info, nullptr, &render_pass);
    if (ret == VK_SUCCESS) {
        return new VulkanRenderPass(device_, render_pass);
    }
    return nullptr;
}

void VulkanLogicDevice::DestroyRenderPass(VulkanRenderPass** render_pass) {
    if (*render_pass) {
        delete *render_pass;
        *render_pass = nullptr;
    }
}

VulkanShaderModule* VulkanLogicDevice::CreateShaderModule(const VkShaderModuleCreateInfo* info) const {
    VkShaderModule shader_module;
    VkResult ret = vkCreateShaderModule(device_, info, nullptr, &shader_module);
    if (ret == VK_SUCCESS) {
        return new VulkanShaderModule(device_, shader_module);
    }
    return nullptr;
}

void VulkanLogicDevice::DestroyShaderModule(VulkanShaderModule** shader_module) {
    if (*shader_module) {
        delete *shader_module;
        *shader_module = nullptr;
    }
}

VulkanDescriptorSetLayout* VulkanLogicDevice::CreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo* info) {
    VkDescriptorSetLayout descriptor_set_layout;
    VkResult ret = vkCreateDescriptorSetLayout(device_, info, nullptr, &descriptor_set_layout);
    if (ret == VK_SUCCESS) {
        return new VulkanDescriptorSetLayout(device_, descriptor_set_layout);
    }
    return nullptr;
}

void VulkanLogicDevice::DestroyDescriptorSetLayout(VulkanDescriptorSetLayout** descriptor_set_layout) {
    if (*descriptor_set_layout) {
        delete *descriptor_set_layout;
        *descriptor_set_layout = nullptr;
    }
}

VulkanDescriptorPool* VulkanLogicDevice::CreateDescriptorPool(const VkDescriptorPoolCreateInfo* info) const {
    VkDescriptorPool descriptor_pool;
    VkResult ret = vkCreateDescriptorPool(device_, info, nullptr, &descriptor_pool);
    if (ret == VK_SUCCESS) {
        return new VulkanDescriptorPool(device_, descriptor_pool);
    }
    return nullptr;
}

void VulkanLogicDevice::DestroyDescriptorPool(VulkanDescriptorPool** descriptor_pool) {
    if (*descriptor_pool) {
        delete *descriptor_pool;
        *descriptor_pool = nullptr;
    }
}

std::vector<VulkanDescriptorSet*> VulkanLogicDevice::AllocateDescriptorSets(const VkDescriptorSetAllocateInfo* info) const {
    /*  typedef struct VkDescriptorSetAllocateInfo {
        VkStructureType                 sType;
        const void*                     pNext;
        VkDescriptorPool                descriptorPool;
        uint32_t                        descriptorSetCount;
        const VkDescriptorSetLayout*    pSetLayouts;
    } VkDescriptorSetAllocateInfo; */
    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

    std::vector<VkDescriptorSet> descriptor_sets(info->descriptorSetCount);
    VkResult ret = vkAllocateDescriptorSets(device_, info, descriptor_sets.data());
    std::vector<VulkanDescriptorSet*> sets;
    if (ret == VK_SUCCESS) {
        for(auto it : descriptor_sets) {
            sets.push_back(new VulkanDescriptorSet(device_, info->descriptorPool, it));
        }
    }
    return sets;
}

void VulkanLogicDevice::FreeDescriptorSets(std::vector<VulkanDescriptorSet*>& descriptor_sets) {
    for (auto& it : descriptor_sets) {
        delete it;
        it = nullptr;
    }
    descriptor_sets.clear();
}

void VulkanLogicDevice::UpdateDescriptorSets(uint32_t descriptor_write_count, const VkWriteDescriptorSet* descriptor_writes) const {
    vkUpdateDescriptorSets(device_, descriptor_write_count, descriptor_writes, 0, nullptr);
}

void VulkanLogicDevice::UpdateDescriptorSets(uint32_t descriptor_copy_count, const VkCopyDescriptorSet* descriptor_copies) const {
    vkUpdateDescriptorSets(device_, 0, nullptr, descriptor_copy_count, descriptor_copies);
}

VulkanPipelineLayout* VulkanLogicDevice::CreatePipelineLayout(const VkPipelineLayoutCreateInfo *info) const {
    VkPipelineLayout pipeline_layout;
    VkResult ret = vkCreatePipelineLayout(device_, info, nullptr, &pipeline_layout);
    if (ret == VK_SUCCESS) {
        return new VulkanPipelineLayout(device_, pipeline_layout);
    }
    return nullptr;
}

void VulkanLogicDevice::DestroyPipelineLayout(VulkanPipelineLayout** pipeline_layout) {
    if (*pipeline_layout) {
        delete *pipeline_layout;
        *pipeline_layout = nullptr;
    }
}

VulkanPipeline* VulkanLogicDevice::CreateGraphicPipeline(const VkGraphicsPipelineCreateInfo *info) const {
    VkPipeline pipeline;
    VkResult ret = vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1,
                                     info, nullptr, &pipeline);
    if (ret == VK_SUCCESS) {
        return new VulkanPipeline(device_, pipeline);
    }
    return nullptr;
}

void VulkanLogicDevice::DestroyPipelines(VulkanPipeline** pipeline) {
    if (*pipeline) {
        delete *pipeline;
        *pipeline = nullptr;
    }
}

VulkanFrameBuffer*  VulkanLogicDevice::CreateFrameBuffer(const VkFramebufferCreateInfo *info) const {
    VkFramebuffer framebuffer;
    VkResult ret = vkCreateFramebuffer(device_, info, nullptr, &framebuffer);
    if (ret == VK_SUCCESS) {
        return new VulkanFrameBuffer(device_, framebuffer);
    }
    return nullptr;
}

void VulkanLogicDevice::DestroyFrameBuffer(VulkanFrameBuffer** framebuffer) {
    if (*framebuffer) {
        delete *framebuffer;
        *framebuffer = nullptr;
    }
}

VulkanCommandPool* VulkanLogicDevice::CreateCommandPool(const VkCommandPoolCreateInfo *info) const {
    VkCommandPool command_pool;
    VkResult ret = vkCreateCommandPool(device_, info, nullptr, &command_pool);
    if (ret == VK_SUCCESS) {
        return new VulkanCommandPool(device_, command_pool);
    }
    return nullptr;
}

void VulkanLogicDevice::DestroyCommandPool(VulkanCommandPool** command_pool) {
    if (*command_pool) {
        delete *command_pool;
        *command_pool = nullptr;
    }
}

VulkanSemaphore* VulkanLogicDevice::CreateSemaphore(const VkSemaphoreCreateInfo* info) const {
    VkSemaphore semaphore;
    VkResult ret = vkCreateSemaphore(device_, info, nullptr, &semaphore);
    if (ret == VK_SUCCESS) {
        return new VulkanSemaphore(device_, semaphore);
    }
    return nullptr;
}

void VulkanLogicDevice::DestroySemaphore(VulkanSemaphore** semaphore) {
    if (*semaphore) {
        delete *semaphore;
        *semaphore = nullptr;
    }
}

VulkanFence* VulkanLogicDevice::CreateFence(const VkFenceCreateInfo* info) const{
    VkFence fence;
    VkResult ret = vkCreateFence(device_, info, nullptr, &fence);
    if (ret == VK_SUCCESS) {
        return new VulkanFence(device_, fence);
    }
    return nullptr;
}

void VulkanLogicDevice::DestroyFence(VulkanFence** fence) {
    if (*fence) {
        delete *fence;
        *fence = nullptr;
    }
}

VkResult VulkanLogicDevice::WaitForFences(uint32_t fence_count, const VkFence* fences, VkBool32 wait_all, uint64_t timeout_ns) const {
    return vkWaitForFences(device_, fence_count, fences, wait_all, timeout_ns);
}

VkResult VulkanLogicDevice::ResetFences(uint32_t fence_count, const VkFence* fences) const {
    return vkResetFences(device_, fence_count, fences);
}

VulkanBuffer* VulkanLogicDevice::CreateBuffer(const VkBufferCreateInfo *info) const {
    VkBuffer vertex_buffer;
    VkResult ret = vkCreateBuffer(device_, info, nullptr, &vertex_buffer);
    if (ret == VK_SUCCESS) {
        return new VulkanBuffer(device_, vertex_buffer);
    }
    return nullptr;
}

void VulkanLogicDevice::DestroyBuffer(VulkanBuffer** buffer) {
    if (*buffer) {
        delete *buffer;
        *buffer = nullptr;
    }
}

VulkanMemory* VulkanLogicDevice::AllocateMemory(const VkMemoryAllocateInfo *info) const {
    VkDeviceMemory memory;
    VkResult ret = vkAllocateMemory(device_, info, nullptr, &memory);
    if (ret == VK_SUCCESS) {
        return new VulkanMemory(device_, memory);
    }
    return nullptr;
}

void VulkanLogicDevice::FreeMemory(VulkanMemory** memory) {
    if (*memory) {
        delete *memory;
        *memory = nullptr;
    }
}

VkResult VulkanLogicDevice::DeviceWaitIdle() const {
    return vkDeviceWaitIdle(device_);
}

void VulkanLogicDevice::GetPhysicalDeviceMemoryProperties(VkPhysicalDeviceMemoryProperties* mem_properties) const {
    vkGetPhysicalDeviceMemoryProperties(physical_device_, mem_properties);
}

VkResult VulkanLogicDevice::GetMemoryType(const VkPhysicalDeviceMemoryProperties* mem_properties, uint32_t type_filter, VkMemoryPropertyFlags property_flags, uint32_t* type_index) {
    for (uint32_t i = 0; i < mem_properties->memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) && (mem_properties->memoryTypes[i].propertyFlags & property_flags) == property_flags) {
            *type_index = i;
            return VK_SUCCESS;
        }
    }
    return VK_ERROR_UNKNOWN;
}

VulkanImage* VulkanLogicDevice::CreateImage(const VkImageCreateInfo* info) {
    VkImage image;
    VkResult ret = vkCreateImage(device_, info, nullptr, &image);
    if (ret == VK_SUCCESS) {
        return new VulkanImage(device_, image);
    }
    return nullptr;
}

void VulkanLogicDevice::DestroyImage(VulkanImage** image) {
    if (*image) {
        delete *image;
        *image = nullptr;
    }
}

VulkanSampler* VulkanLogicDevice::CreateSampler(const VkSamplerCreateInfo* info) {
    VkSampler sampler;
    VkResult ret = vkCreateSampler(device_, info, nullptr, &sampler);
    if (ret == VK_SUCCESS) {
        return new VulkanSampler(device_, sampler);
    }
    return nullptr;
}

void VulkanLogicDevice::DestroySampler(VulkanSampler** sampler) {
    if (*sampler) {
        delete *sampler;
        *sampler = nullptr;
    }
}

VulkanSamplerYcbcrConversion* VulkanLogicDevice::CreateSamplerYcbcrConversion(const VkSamplerYcbcrConversionCreateInfo* info) {
    auto func = (PFN_vkCreateSamplerYcbcrConversion) vkGetDeviceProcAddr(device_,
                                                                           "vkCreateSamplerYcbcrConversion");
    VkSamplerYcbcrConversion ycbcr_conversion;
    if (func != nullptr) {
        VkResult ret = func(device_, info, nullptr, &ycbcr_conversion);
        if (ret == VK_SUCCESS) {
            return new VulkanSamplerYcbcrConversion(device_, ycbcr_conversion);
        }
    }
    return nullptr;
}

void VulkanLogicDevice::DestroySamplerYcbcrConversion(VulkanSamplerYcbcrConversion** ycbcr_conversion) {
    if (*ycbcr_conversion) {
        delete *ycbcr_conversion;
        *ycbcr_conversion = nullptr;
    }
}