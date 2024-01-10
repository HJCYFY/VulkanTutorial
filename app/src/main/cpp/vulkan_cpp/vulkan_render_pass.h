//
// Created by hj6231 on 2023/12/26.
//

#pragma once
#include <vulkan/vulkan.h>

class VulkanRenderPass {
public:
    VulkanRenderPass(VkDevice device, VkRenderPass render_pass);
    VulkanRenderPass(const VulkanRenderPass&) = delete;
    ~VulkanRenderPass();
    VkRenderPass render_pass() const;

    VulkanRenderPass& operator = (const VulkanRenderPass&) = delete;
private:
    VkDevice device_;
    VkRenderPass render_pass_;
};
