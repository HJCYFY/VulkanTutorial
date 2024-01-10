//
// Created by hj6231 on 2023/12/26.
//

#include "vulkan_pipeline_layout.h"

VulkanPipelineLayout::VulkanPipelineLayout(VkDevice device, VkPipelineLayout layout) :
        device_(device), layout_(layout) {

}

VulkanPipelineLayout::~VulkanPipelineLayout() {
    vkDestroyPipelineLayout(device_, layout_, nullptr);
}

VkPipelineLayout VulkanPipelineLayout::layout() const {
    return layout_;
}