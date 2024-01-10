//
// Created by hj6231 on 2023/12/26.
//

#pragma once
#include <vulkan/vulkan.h>

class VulkanFrameBuffer {
public:
    VulkanFrameBuffer(VkDevice device, VkFramebuffer frame_buffer);
    VulkanFrameBuffer(const VulkanFrameBuffer&) = delete;
    ~VulkanFrameBuffer();
    VkFramebuffer frame_buffer() const;
    VulkanFrameBuffer& operator = (const VulkanFrameBuffer&) = delete;
private:
    VkDevice device_;
    VkFramebuffer frame_buffer_;
};

