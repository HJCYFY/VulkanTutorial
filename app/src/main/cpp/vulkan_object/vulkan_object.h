//
// Created by hj6231 on 2023/12/22.
//

#pragma once
#include <string>
#include <vulkan/vulkan.h>
#include "shaderc.hpp"
#include "vulkan_logic_device.h"
#include "vulkan_shader_module.h"
#include "vulkan_render_pass.h"
#include "vulkan_pipeline.h"
#include "vulkan_pipeline_layout.h"
#include "vulkan_descriptor_pool.h"

class VulkanObject {
public:
    VulkanObject(VulkanLogicDevice* device);
    ~VulkanObject() = default;

    virtual int CreatePipeline(const VulkanRenderPass* render_pass) = 0;
    virtual void DestroyPipeline() = 0;

    virtual void Draw(const VulkanCommandBuffer* command_buffer,
                      const VulkanRenderPass* render_pass,
                      const VulkanFrameBuffer* frame_buffer,
                      uint32_t frame_buffer_width,
                      uint32_t frame_buffer_height) const = 0;

    virtual VkViewport GetVkViewport(uint32_t win_width, uint32_t win_height) const;
    virtual VkRect2D GetScissor(uint32_t win_width, uint32_t win_height) const;

    VulkanPipeline* pipeline();
protected:
    static std::vector<uint32_t> CompileFile(const std::string& source_name,
                                             shaderc_shader_kind kind,
                                             const std::string& source,
                                             bool optimize = false);

    VulkanShaderModule* CreateShaderModule(const std::string& name,
                                           shaderc_shader_kind kind,
                                           const std::string& source) const;

    virtual VulkanPipeline* CreateGraphicsPipeline(const VulkanShaderModule* vert_shader_module,
                                                   const VulkanShaderModule* frag_shader_module,
                                                   const VulkanPipelineLayout* layout,
                                                   const VulkanRenderPass* render_pass) const = 0;


    VulkanLogicDevice* device_;
    VulkanPipelineLayout* pipeline_layout_;
    VulkanPipeline* pipeline_;
    std::string vertex_str_;
    std::string fragment_str_;
};
