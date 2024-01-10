//
// Created by hj6231 on 2024/1/2.
//

#include "vulkan_descriptor_set.h"
#include "log.h"
VulkanDescriptorSet::VulkanDescriptorSet(VkDevice device, VkDescriptorPool descriptor_pool, VkDescriptorSet descriptor_set) :
        device_(device),
        descriptor_pool_(descriptor_pool),
        descriptor_set_(descriptor_set) {
}

VulkanDescriptorSet::~VulkanDescriptorSet() {
    vkFreeDescriptorSets(device_, descriptor_pool_, 1, &descriptor_set_);
}

VkDescriptorSet VulkanDescriptorSet::descriptor_set() const {
    return descriptor_set_;
}