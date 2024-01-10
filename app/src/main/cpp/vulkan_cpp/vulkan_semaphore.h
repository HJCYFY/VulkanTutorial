//
// Created by hj6231 on 2023/12/27.
//

#pragma once
#include <vulkan/vulkan.h>

class VulkanSemaphore {
public:
    VulkanSemaphore(VkDevice device, VkSemaphore semaphore);
    VulkanSemaphore(const VulkanSemaphore&) = delete;
    ~VulkanSemaphore();

    VkSemaphore semaphore() const;
    VulkanSemaphore& operator = (const VulkanSemaphore&) = delete;
private:
    VkDevice device_;
    VkSemaphore semaphore_;
};


