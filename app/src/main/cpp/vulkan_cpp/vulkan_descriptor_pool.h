//
// Created by hj6231 on 2024/1/2.
//

#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "vulkan_descriptor_set.h"

class VulkanDescriptorPool {
public:
    VulkanDescriptorPool(VkDevice device, VkDescriptorPool descriptor_pool);
    VulkanDescriptorPool(const VulkanDescriptorPool&) = delete;
    ~VulkanDescriptorPool();

    VkDescriptorPool descriptor_pool() const;
    VulkanDescriptorPool& operator = (const VulkanDescriptorPool&) = delete;

    VulkanDescriptorSet* AllocateDescriptorSet(const VkDescriptorSetLayout* set_layout);
    static void FreeDescriptorSet(VulkanDescriptorSet** descriptor_set);
private:
    VkDevice device_;
    VkDescriptorPool descriptor_pool_;
};


