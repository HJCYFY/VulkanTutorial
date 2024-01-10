//
// Created by hj6231 on 2023/12/29.
//

#pragma once
#include <vulkan/vulkan.h>


class VulkanMemory {
public:
    VulkanMemory(VkDevice device, VkDeviceMemory device_memory);
    VulkanMemory(const VulkanMemory&) = delete;
    ~VulkanMemory();

    VkDeviceMemory device_memory() const;
    VkResult BindBufferMemory(VkBuffer buffer, VkDeviceSize memory_offset) const;
    VkResult BindImageMemory(VkImage image, VkDeviceSize memory_offset) const;
    VkResult MapMemory(VkDeviceSize offset, VkDeviceSize size, void** data);
    void UnmapMemory();
    VulkanMemory& operator = (const VulkanMemory&) = delete;
private:
    VkDevice device_;
    VkDeviceMemory device_memory_;
};


