//
// Created by hj6231 on 2023/12/27.
//

#include "vulkan_fence.h"

VulkanFence::VulkanFence(VkDevice device, VkFence fence) :
        device_(device), fence_(fence) {

}

VulkanFence::~VulkanFence() {
    vkDestroyFence(device_, fence_, nullptr);
}

VkFence VulkanFence::fence() const {
    return fence_;
}