//
// Created by hj6231 on 2023/12/26.
//

#include "vulkan_shader_module.h"


VulkanShaderModule::VulkanShaderModule(VkDevice device, VkShaderModule shader_module) :
        device_(device), shader_module_(shader_module){

}

VulkanShaderModule::~VulkanShaderModule() {
    vkDestroyShaderModule(device_, shader_module_, nullptr);
}

VkShaderModule VulkanShaderModule::shader_module() const {
    return shader_module_;
}
