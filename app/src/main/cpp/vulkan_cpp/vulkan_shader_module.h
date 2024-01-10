//
// Created by hj6231 on 2023/12/26.
//

#pragma once
#include <vulkan/vulkan.h>

class VulkanShaderModule {
public:
    VulkanShaderModule(VkDevice device, VkShaderModule shader_module);
    VulkanShaderModule(const VulkanShaderModule&) = delete;
    ~VulkanShaderModule();

    VkShaderModule shader_module() const;

    VulkanShaderModule& operator = (const VulkanShaderModule&) = delete;
private:
    VkDevice device_;
    VkShaderModule shader_module_;
};


