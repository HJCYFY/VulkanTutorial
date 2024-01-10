//
// Created by hj6231 on 2023/12/29.
//

#include "vulkan_memory.h"


VulkanMemory::VulkanMemory(VkDevice device, VkDeviceMemory device_memory) :
        device_(device), device_memory_(device_memory) {
}

VulkanMemory::~VulkanMemory() {
    vkFreeMemory(device_, device_memory_, nullptr);
}

VkDeviceMemory VulkanMemory::device_memory() const {
    return device_memory_;
}

VkResult VulkanMemory::BindBufferMemory(VkBuffer buffer, VkDeviceSize memory_offset) const {
    return vkBindBufferMemory(device_, buffer, device_memory_, memory_offset);
}

VkResult VulkanMemory::BindImageMemory(VkImage image, VkDeviceSize memory_offset) const {
    return vkBindImageMemory(device_, image, device_memory_, memory_offset);
}

VkResult VulkanMemory::MapMemory(VkDeviceSize offset, VkDeviceSize size, void** data) {
    return vkMapMemory(device_, device_memory_, offset, size, 0, data);
}

void VulkanMemory::UnmapMemory() {
    vkUnmapMemory(device_, device_memory_);
}