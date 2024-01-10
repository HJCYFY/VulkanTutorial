//
// Created by hj6231 on 2024/1/2.
//

#include "vulkan_descriptor_set_layout.h"

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VkDevice device,
                                                     VkDescriptorSetLayout descriptor_set_layout) :
                                                     device_(device), descriptor_set_layout_(descriptor_set_layout) {
}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout() {
    vkDestroyDescriptorSetLayout(device_, descriptor_set_layout_, nullptr);
}

VkDescriptorSetLayout VulkanDescriptorSetLayout::descriptor_set_layout() const {
    return descriptor_set_layout_;
}
