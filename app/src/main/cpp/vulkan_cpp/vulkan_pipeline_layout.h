//
// Created by hj6231 on 2023/12/26.
//

#pragma once
#include <vulkan/vulkan.h>


class VulkanPipelineLayout {
public:
    VulkanPipelineLayout(VkDevice device, VkPipelineLayout layout);
    VulkanPipelineLayout(const VulkanPipelineLayout&) = delete;
    ~VulkanPipelineLayout();

    VkPipelineLayout layout() const;
    VulkanPipelineLayout& operator = (const VulkanPipelineLayout&) = delete;
private:
    VkDevice device_;
    VkPipelineLayout layout_;
};


