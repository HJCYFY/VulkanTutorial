//
// Created by hj6231 on 2023/12/25.
//

#pragma once
#include <vector>

#include "vulkan_object.h"

class Triangle : public VulkanObject {
public:
    Triangle(VulkanLogicDevice* device);
    ~Triangle() = default;

    int CreatePipeline(const VulkanRenderPass* render_pass) override;
    void DestroyPipeline() override;

    void Draw(const VulkanCommandBuffer* command_buffer,
              const VulkanRenderPass* render_pass,
              const VulkanFrameBuffer* frame_buffer,
              uint32_t frame_buffer_width,
              uint32_t frame_buffer_height) const override;
protected:
private:
    VulkanPipelineLayout* CreatePipelineLayout() const;
    VulkanPipeline* CreateGraphicsPipeline(const VulkanShaderModule* vert_shader_module,
                                           const VulkanShaderModule* frag_shader_module,
                                           const VulkanPipelineLayout* layout,
                                           const VulkanRenderPass* render_pass) const override;
    // GraphicsPipelineCreateInfo
    static std::vector<VkPipelineShaderStageCreateInfo> GetPipelineShaderStageCreateInfos(
            const VulkanShaderModule* vert_shader_module, const VulkanShaderModule* frag_shader_module);
    static VkPipelineVertexInputStateCreateInfo GetPipelineVertexInputStateCreateInfo();
    static VkPipelineInputAssemblyStateCreateInfo  GetPipelineInputAssemblyStateCreateInfo();
    static VkPipelineViewportStateCreateInfo GetPipelineViewportStateCreateInfo();
    static VkPipelineRasterizationStateCreateInfo GetPipelineRasterizationStateCreateInfo();
    static VkPipelineMultisampleStateCreateInfo GetPipelineMultisampleStateCreateInfo();
    static VkPipelineColorBlendStateCreateInfo GetPipelineColorBlendStateCreateInfo(
            const VkPipelineColorBlendAttachmentState* color_blend_attachment);
    static VkPipelineDynamicStateCreateInfo  GetPipelineDynamicStateCreateInfo(const std::vector<VkDynamicState>& dynamic_states);
    static VkPipelineColorBlendAttachmentState GetPipelineColorBlendAttachmentState();

    static const char kVertShaderSource[];
    static const char kFragShaderSource[];

    const std::vector<VkDynamicState> dynamic_states_ = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
};


