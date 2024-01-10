//
// Created by hj6231 on 2024/1/2.
//

#pragma once
#include <vulkan/vulkan.h>

class VulkanDescriptorSetLayout {
public:
    VulkanDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptor_set_layout);
    VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout&) = delete;
    ~VulkanDescriptorSetLayout();

    VkDescriptorSetLayout descriptor_set_layout() const;

    VulkanDescriptorSetLayout& operator = (const VulkanDescriptorSetLayout&) = default;
private:
    VkDevice device_;
    VkDescriptorSetLayout descriptor_set_layout_;
};


