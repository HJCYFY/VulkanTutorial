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
#include "vulkan_render_pass.h"

class VulkanObject {
public:
    VulkanObject(VulkanLogicDevice* device,
                 VkFormat swap_chain_image_format,
                 VkExtent2D frame_buffer_size);
    virtual ~VulkanObject() = default;

    virtual int CreatePipeline() = 0;
    virtual void DestroyPipeline() = 0;

    virtual void Draw(const VulkanCommandBuffer* command_buffer,
                      const VulkanFrameBuffer* frame_buffer) const = 0;

    virtual VkViewport GetVkViewport() const;
    virtual VkRect2D GetScissor() const;

    VulkanRenderPass* render_pass();
    VulkanImageView* depth_attachment_image_view();
    VulkanPipeline* pipeline();
protected:
    virtual void LoadResource() = 0;
    virtual void CreateRenderPass() = 0;
    void DestroyRenderPass();

    virtual void CreateVertexBuffer() = 0;
    virtual void DestroyVertexBuffer() = 0;

    virtual void CreateIndexBuffer() = 0;
    virtual void DestroyIndexBuffer() = 0;

    virtual void CreateDescriptorSets() = 0;

    static std::vector<uint32_t> CompileFile(const std::string& source_name,
                                             shaderc_shader_kind kind,
                                             const std::string& source,
                                             bool optimize = false);

    VulkanShaderModule* CreateShaderModule(const std::string& name,
                                           shaderc_shader_kind kind,
                                           const std::string& source) const;

    VulkanLogicDevice* device_;
    VkFormat swap_chain_image_format_;
    VkExtent2D frame_buffer_size_;

    VulkanRenderPass* render_pass_;

    VulkanPipeline* pipeline_;

    VulkanImage* depth_attachment_image_;
    VulkanImageView* depth_attachment_image_view_;

    std::string vertex_str_;
    std::string fragment_str_;
};
