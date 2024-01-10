//
// Created by hj6231 on 2023/12/26.
//

#pragma once
#include <vulkan/vulkan.h>

class VulkanPipeline {
public:
    VulkanPipeline(VkDevice device, VkPipeline pipeline);
    VulkanPipeline(const VulkanPipeline&) = delete;
    ~VulkanPipeline();

    VkPipeline pipeline() const;

    VulkanPipeline& operator = (const VulkanPipeline&) = delete;
private:
    VkDevice device_;
    VkPipeline pipeline_;
};


