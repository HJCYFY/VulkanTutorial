//
// Created by hj6231 on 2023/12/26.
//

#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "vulkan_command_buffer.h"

class VulkanCommandPool {
public:
    VulkanCommandPool(VkDevice device, VkCommandPool command_pool);
    VulkanCommandPool(const VulkanCommandPool&) = delete;
    ~VulkanCommandPool();
    VkCommandPool command_pool() const;

    VulkanCommandBuffer* AllocateCommandBuffer(VkCommandBufferLevel level);
    static void FreeCommandBuffer(VulkanCommandBuffer** buffer);
    VulkanCommandPool& operator = (const VulkanCommandPool&) = delete;
private:
    VkDevice device_;
    VkCommandPool command_pool_;
};

