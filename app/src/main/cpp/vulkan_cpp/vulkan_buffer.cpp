//
// Created by hj6231 on 2023/12/29.
//

#include "vulkan_buffer.h"

VulkanBuffer::VulkanBuffer(VkDevice device, VkBuffer buffer) :
        device_(device), buffer_(buffer) {
}

VulkanBuffer::~VulkanBuffer() {
    vkDestroyBuffer(device_, buffer_, nullptr);
}

VkBuffer VulkanBuffer::buffer() const {
    return buffer_;
}

void VulkanBuffer::GetBufferMemoryRequirements(VkMemoryRequirements* mem_requirements) {
    vkGetBufferMemoryRequirements(device_, buffer_, mem_requirements);
}
