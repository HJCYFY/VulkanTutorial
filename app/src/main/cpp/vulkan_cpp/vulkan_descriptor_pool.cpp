//
// Created by hj6231 on 2024/1/2.
//

#include "vulkan_descriptor_pool.h"
#include "log.h"


VulkanDescriptorPool::VulkanDescriptorPool(VkDevice device, VkDescriptorPool descriptor_pool) :
        device_(device), descriptor_pool_(descriptor_pool) {

}

VulkanDescriptorPool::~VulkanDescriptorPool() {
    vkDestroyDescriptorPool(device_, descriptor_pool_, nullptr);
}

VkDescriptorPool VulkanDescriptorPool::descriptor_pool() const {
    return descriptor_pool_;
}

VulkanDescriptorSet* VulkanDescriptorPool::AllocateDescriptorSet(const VkDescriptorSetLayout* set_layout) {
    /* typedef struct VkDescriptorSetAllocateInfo {
        VkStructureType                 sType;
        const void*                     pNext;
        VkDescriptorPool                descriptorPool;
        uint32_t                        descriptorSetCount;
        const VkDescriptorSetLayout*    pSetLayouts;
    } VkDescriptorSetAllocateInfo; */
    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = descriptor_pool_;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = set_layout;
    VkDescriptorSet descriptor_set;
    VkResult ret = vkAllocateDescriptorSets(device_, &alloc_info, &descriptor_set);
    if (ret == VK_SUCCESS) {
        VulkanDescriptorSet* vulkan_descriptor_set = new VulkanDescriptorSet(device_, descriptor_pool_, descriptor_set);
        return vulkan_descriptor_set;
    }
    LOG_D("", " Ret = %d\n", ret);
    return nullptr;
}

void VulkanDescriptorPool::FreeDescriptorSet(VulkanDescriptorSet** descriptor_set) {
    if (*descriptor_set) {
        delete *descriptor_set;
        *descriptor_set = nullptr;
    }
}