//
// Created by hj6231 on 2024/1/5.
//

#include "vulkan_sampler.h"

VulkanSampler::VulkanSampler(VkDevice device, VkSampler sampler) :
        device_(device), sampler_(sampler) {

}

VulkanSampler::~VulkanSampler() {
    vkDestroySampler(device_, sampler_, nullptr);
}

VkSampler VulkanSampler::sampler() const {
    return sampler_;
}
