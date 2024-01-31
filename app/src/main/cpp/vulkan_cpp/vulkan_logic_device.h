//
// Created by hj6231 on 2023/12/20.
//

#pragma once
#undef VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#include "vulkan_queue.h"
#include "vulkan_swap_chain.h"
#include "vulkan_image.h"
#include "vulkan_image_view.h"
#include "vulkan_frame_buffer.h"
#include "vulkan_command_pool.h"
#include "vulkan_shader_module.h"
#include "vulkan_pipeline_layout.h"
#include "vulkan_render_pass.h"
#include "vulkan_pipeline.h"
#include "vulkan_semaphore.h"
#include "vulkan_fence.h"
#include "vulkan_buffer.h"
#include "vulkan_memory.h"
#include "vulkan_descriptor_set_layout.h"
#include "vulkan_descriptor_pool.h"
#include "vulkan_sampler.h"
#include "vulkan_sampler_ycbcr_conversion.h"

class VulkanLogicDevice {
public:
    VulkanLogicDevice(VkPhysicalDevice physical_device, VkDevice device);
    VulkanLogicDevice(const VulkanLogicDevice& device) = delete;
    ~VulkanLogicDevice();

    VulkanQueue* GetDeviceQueue(uint32_t queue_family, int queue_index) const;

    VulkanSwapChain* CreateSwapChain(const VkSwapchainCreateInfoKHR* info) const;
    static void DestroySwapChain(VulkanSwapChain** swap_chain);
    VkResult AcquireNextImageKHR(VkSwapchainKHR swap_chain,
                                 uint64_t timeout,
                                 VkSemaphore semaphore,
                                 VkFence fence,
                                 uint32_t* image_index) const;

    VulkanImageView* CreateImageView(const VkImageViewCreateInfo* info) const;
    static void DestroyImageView(VulkanImageView** view);

    VulkanRenderPass* CreateRenderPass(const VkRenderPassCreateInfo *info);
    static void DestroyRenderPass(VulkanRenderPass** render_pass);

    /* graphic pipeline*/
    VulkanShaderModule* CreateShaderModule(const VkShaderModuleCreateInfo* info) const;
    static void DestroyShaderModule(VulkanShaderModule** shader_module);
    VulkanDescriptorSetLayout* CreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo* info);
    static void DestroyDescriptorSetLayout(VulkanDescriptorSetLayout** descriptor_set_layout);
    VulkanDescriptorPool* CreateDescriptorPool(const VkDescriptorPoolCreateInfo* info) const;
    static void DestroyDescriptorPool(VulkanDescriptorPool** descriptor_pool);
    std::vector<VulkanDescriptorSet*> AllocateDescriptorSets(const VkDescriptorSetAllocateInfo * info) const;
    static void FreeDescriptorSets(std::vector<VulkanDescriptorSet*>& descriptor_sets);
    void UpdateDescriptorSets(uint32_t descriptor_write_count, const VkWriteDescriptorSet* descriptor_writes) const;
    void UpdateDescriptorSets(uint32_t descriptor_copy_count, const VkCopyDescriptorSet* descriptor_copies) const;

    VulkanPipelineLayout* CreatePipelineLayout(const VkPipelineLayoutCreateInfo *info) const;
    static void DestroyPipelineLayout(VulkanPipelineLayout** pipeline_layout);
    VulkanPipeline* CreateGraphicPipeline(const VkGraphicsPipelineCreateInfo *info) const;
    VulkanPipeline* CreateComputePipeline(const VkComputePipelineCreateInfo *info) const;
    static void DestroyPipelines(VulkanPipeline** pipeline);

    VulkanFrameBuffer* CreateFrameBuffer(const VkFramebufferCreateInfo *info) const;
    static void DestroyFrameBuffer(VulkanFrameBuffer** framebuffer);
    VulkanCommandPool* CreateCommandPool(const VkCommandPoolCreateInfo *info) const;
    static void DestroyCommandPool(VulkanCommandPool** command_pool);

    VulkanSemaphore* CreateSemaphore(const VkSemaphoreCreateInfo* info) const;
    static void DestroySemaphore(VulkanSemaphore** semaphore);
    VulkanFence* CreateFence(const VkFenceCreateInfo* info) const;
    static void DestroyFence(VulkanFence** fence);
    VkResult WaitForFences(uint32_t fence_count, const VkFence* fences, VkBool32 wait_all, uint64_t timeout_ns) const;
    VkResult ResetFences(uint32_t fence_count, const VkFence* fences) const;

    VulkanBuffer* CreateBuffer(const VkBufferCreateInfo *info) const;
    static void DestroyBuffer(VulkanBuffer** buffer);
    VulkanMemory* AllocateMemory(const VkMemoryAllocateInfo *info) const;
    static void FreeMemory(VulkanMemory** memory);

    VkResult DeviceWaitIdle() const;

    void GetPhysicalDeviceMemoryProperties(VkPhysicalDeviceMemoryProperties* mem_properties) const;
    static VkResult GetMemoryType(const VkPhysicalDeviceMemoryProperties* mem_properties, uint32_t type_filter, VkMemoryPropertyFlags property_flags, uint32_t* type_index);

    VulkanImage* CreateImage(const VkImageCreateInfo* info);
    static void DestroyImage(VulkanImage** image);

    VulkanSampler* CreateSampler(const VkSamplerCreateInfo* info);
    static void DestroySampler(VulkanSampler** sampler);
    VulkanSamplerYcbcrConversion* CreateSamplerYcbcrConversion(const VkSamplerYcbcrConversionCreateInfo* info);
    static void DestroySamplerYcbcrConversion(VulkanSamplerYcbcrConversion** ycbcr_conversion);

    VulkanLogicDevice& operator = (const VulkanLogicDevice& device) = delete;
private:
    VkPhysicalDevice physical_device_;
    VkDevice device_;
};
