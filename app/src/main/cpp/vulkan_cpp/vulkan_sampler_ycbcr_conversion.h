//
// Created by hj6231 on 2024/1/9.
//

#pragma once
#include <vulkan/vulkan.h>

class VulkanSamplerYcbcrConversion {
public:
    VulkanSamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcr_conversion);
    VulkanSamplerYcbcrConversion(const VulkanSamplerYcbcrConversion&) = delete;
    ~VulkanSamplerYcbcrConversion();

    VkSamplerYcbcrConversion ycbcr_conversion();

    VulkanSamplerYcbcrConversion& operator = (const VulkanSamplerYcbcrConversion&) = delete;
private:
    VkDevice device_;
    VkSamplerYcbcrConversion ycbcr_conversion_;
};

