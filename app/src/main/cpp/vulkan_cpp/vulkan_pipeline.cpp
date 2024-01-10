//
// Created by hj6231 on 2023/12/26.
//

#include "vulkan_pipeline.h"

VulkanPipeline::VulkanPipeline(VkDevice device, VkPipeline pipeline) :
        device_(device), pipeline_(pipeline) {

}

VulkanPipeline::~VulkanPipeline() {
    vkDestroyPipeline(device_, pipeline_, nullptr);
}

VkPipeline VulkanPipeline::pipeline() const {
    return pipeline_;
}
