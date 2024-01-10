//
// Created by hj6231 on 2024/1/5.
//

#pragma once
#include <vulkan/vulkan.h>

class VulkanSampler {
public:
    VulkanSampler(VkDevice device, VkSampler sampler);
    VulkanSampler(const VulkanSampler&) = delete;
    ~VulkanSampler();

    VkSampler sampler() const;

    VulkanSampler& operator = (const VulkanSampler&) = delete;
private:
    VkDevice device_;
    VkSampler sampler_;
};

