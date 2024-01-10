//
// Created by hj6231 on 2023/12/26.
//

#include "vulkan_frame_buffer.h"


VulkanFrameBuffer::VulkanFrameBuffer(VkDevice device, VkFramebuffer frame_buffer) :
    device_(device), frame_buffer_(frame_buffer) {

}

VulkanFrameBuffer::~VulkanFrameBuffer() {
    vkDestroyFramebuffer(device_, frame_buffer_, nullptr);
}

VkFramebuffer VulkanFrameBuffer::frame_buffer() const {
    return frame_buffer_;
}