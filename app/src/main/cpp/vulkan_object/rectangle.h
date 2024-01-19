//
// Created by hj6231 on 2023/12/29.
//

#pragma once
#include "vulkan_object.h"
#include <glm/glm.hpp>

class Rectangle : public VulkanObject {
public:
    Rectangle(VulkanLogicDevice* device,
              VkFormat swap_chain_image_format,
              VkExtent2D frame_buffer_size);
    ~Rectangle() = default;

    typedef struct  {
        glm::vec2 pos;
        glm::vec3 color;
    } vertex_t;

    int CreatePipeline() override;
    void DestroyPipeline() override;

    void Draw(const VulkanCommandBuffer* command_buffer,
              const VulkanFrameBuffer* frame_buffer) const override;
protected:
    void LoadResource() override {}
    void CreateRenderPass() override;

    void CreateVertexBuffer() override;
    void DestroyVertexBuffer() override;

    void CreateIndexBuffer() override {}
    void DestroyIndexBuffer() override {}

    void CreateDescriptorSets() override {}
private:
    VulkanPipelineLayout* CreatePipelineLayout() const;
    VulkanPipeline* CreateGraphicsPipeline(const VulkanShaderModule* vert_shader_module,
                                           const VulkanShaderModule* frag_shader_module,
                                           const VulkanPipelineLayout* layout,
                                           const VulkanRenderPass* render_pass) const;
    // GraphicsPipelineCreateInfo
    static std::vector<VkPipelineShaderStageCreateInfo> GetPipelineShaderStageCreateInfos(
            const VulkanShaderModule* vert_shader_module, const VulkanShaderModule* frag_shader_module);
    VkPipelineVertexInputStateCreateInfo GetPipelineVertexInputStateCreateInfo() const;
    static VkPipelineInputAssemblyStateCreateInfo  GetPipelineInputAssemblyStateCreateInfo();
    static VkPipelineViewportStateCreateInfo GetPipelineViewportStateCreateInfo();
    static VkPipelineRasterizationStateCreateInfo GetPipelineRasterizationStateCreateInfo();
    static VkPipelineMultisampleStateCreateInfo GetPipelineMultisampleStateCreateInfo();
    static VkPipelineColorBlendStateCreateInfo GetPipelineColorBlendStateCreateInfo(
            const VkPipelineColorBlendAttachmentState* color_blend_attachment);
    static VkPipelineDynamicStateCreateInfo  GetPipelineDynamicStateCreateInfo(const std::vector<VkDynamicState>& dynamic_states);
    static VkPipelineColorBlendAttachmentState GetPipelineColorBlendAttachmentState();

    VulkanBuffer* vertex_buffer_;
    VulkanMemory* vertex_memory_;
    VulkanPipelineLayout* pipeline_layout_;

    static const char kVertShaderSource[];
    static const char kFragShaderSource[];

    const std::vector<VkDynamicState> dynamic_states_ = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    const std::vector<vertex_t> vertices_ = {
            {{0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{-0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    };
    const std::vector<VkVertexInputBindingDescription> vertex_binding_descriptions_ = {{0, sizeof (vertex_t), VK_VERTEX_INPUT_RATE_VERTEX}};
    const std::vector<VkVertexInputAttributeDescription> vertex_attribute_descriptions_ = {
            {0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex_t, pos)},
            {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex_t, color)}};
};
