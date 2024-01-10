//
// Created by hj6231 on 2023/12/27.
//

#pragma once
#include <vulkan/vulkan.h>

class VulkanFence {
public:
    VulkanFence(VkDevice device, VkFence fence);
    VulkanFence(const VulkanFence&) = delete;
    ~VulkanFence();

    VkFence fence() const;

    VulkanFence& operator = (const VulkanFence&) = delete;
private:
    VkDevice device_;
    VkFence fence_;
};

