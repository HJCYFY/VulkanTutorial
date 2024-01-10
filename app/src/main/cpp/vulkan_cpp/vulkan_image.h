//
// Created by hj6231 on 2023/12/21.
//

#pragma once
#include <vulkan/vulkan.h>

class VulkanImage {
public:
    VulkanImage(VkDevice device, VkImage image);
    VulkanImage(const VulkanImage&) = delete;
    ~VulkanImage();

    VkImage image() const;
    VulkanImage operator = (const VulkanImage&) = delete;

    void GetImageMemoryRequirements(VkMemoryRequirements* mem_requirements);
private:
    VkDevice device_;
    VkImage image_;
};

