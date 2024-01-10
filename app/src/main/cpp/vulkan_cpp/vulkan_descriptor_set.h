//
// Created by hj6231 on 2024/1/2.
//

#include <vulkan/vulkan.h>


class VulkanDescriptorSet {
public:
    VulkanDescriptorSet(VkDevice device, VkDescriptorPool descriptor_pool, VkDescriptorSet descriptor_set);
    VulkanDescriptorSet(const VulkanDescriptorSet& ) = delete;
    ~VulkanDescriptorSet();

    VkDescriptorSet descriptor_set() const;

    VulkanDescriptorSet& operator = (const VulkanDescriptorSet&) = delete;
private:
    VkDevice device_;
    VkDescriptorPool descriptor_pool_;
    VkDescriptorSet descriptor_set_;
};


