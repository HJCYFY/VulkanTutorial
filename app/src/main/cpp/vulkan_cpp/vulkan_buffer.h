//
// Created by hj6231 on 2023/12/29.
//

#pragma once
#include <vulkan/vulkan.h>

class VulkanBuffer {
public:
    VulkanBuffer(VkDevice device, VkBuffer buffer);
    VulkanBuffer(const VulkanBuffer&) = delete;
    ~VulkanBuffer();

    VkBuffer buffer() const;

    void GetBufferMemoryRequirements(VkMemoryRequirements* mem_requirements);

    VulkanBuffer& operator = (const VulkanBuffer&) = delete;
private:
    VkDevice device_;
    VkBuffer buffer_;
};


