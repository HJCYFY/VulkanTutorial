//
// Created by hj6231 on 2023/12/20.
//

#pragma once
#include <vulkan/vulkan.h>

class VulkanQueue {
public:
    VulkanQueue(VkQueue queue);
    ~VulkanQueue();

    VkResult QueueSubmit(uint32_t submitCount, const VkSubmitInfo* submits, VkFence fence) const;
    VkResult QueuePresentKHR(const VkPresentInfoKHR* present_info) const;
    VkResult QueueWaitIdle() const;
private:
    VkQueue queue_;
};
