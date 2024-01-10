//
// Created by hj6231 on 2023/12/26.
//

#include "vulkan_render_pass.h"


VulkanRenderPass::VulkanRenderPass(VkDevice device, VkRenderPass render_pass) :
        device_(device), render_pass_(render_pass) {

}

VulkanRenderPass::~VulkanRenderPass() {
    vkDestroyRenderPass(device_, render_pass_, nullptr);
}

VkRenderPass VulkanRenderPass::render_pass() const {
    return render_pass_;
}