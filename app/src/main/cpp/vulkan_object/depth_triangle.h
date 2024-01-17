//
// Created by hj6231 on 2024/1/10.
//

#pragma once
#include "vulkan_object.h"
#include <glm/glm.hpp>

class DepthTriangle : public VulkanObject {
public:
    DepthTriangle(VulkanCommandPool* command_pool,
                  VulkanQueue* graphic_queue,
                  VulkanLogicDevice* device,
                  VkFormat swap_chain_image_format,
                  VkExtent2D frame_buffer_size);
    ~DepthTriangle() = default;
    int CreatePipeline() override;
    void DestroyPipeline() override;

    typedef struct  {
        glm::vec3 pos;
        glm::vec3 color;
    } vertex_t;

    void Draw(const VulkanCommandBuffer* command_buffer,
              const VulkanFrameBuffer* frame_buffer) const override;
protected:
    void LoadResource() override;
    void CreateRenderPass() override;

    void CreateVertexBuffer() override;
    void DestroyVertexBuffer() override;

    void CreateIndexBuffer() override;
    void DestroyIndexBuffer() override;

    void CreateDescriptorSets() override {}
private:
    void CreateDepthImage();
    void DestroyDepthImage();
    void CreateDepthImageView();
    void DestroyDepthImageView();
    void TransitionImageLayout();

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
    static VkPipelineDepthStencilStateCreateInfo GetPipelineDepthStencilStateCreateInfo();
    static VkPipelineColorBlendStateCreateInfo GetPipelineColorBlendStateCreateInfo(
            const VkPipelineColorBlendAttachmentState* color_blend_attachment);
    static VkPipelineDynamicStateCreateInfo  GetPipelineDynamicStateCreateInfo(const std::vector<VkDynamicState>& dynamic_states);
    static VkPipelineColorBlendAttachmentState GetPipelineColorBlendAttachmentState();

    static const char kVertShaderSource[];
    static const char kFragShaderSource[];

    VulkanCommandPool* command_pool_;
    VulkanQueue* graphic_queue_;

    VulkanPipelineLayout* pipeline_layout_;

    VulkanBuffer* vertex_buffer_;
    VulkanMemory* vertex_memory_;

    VulkanBuffer* indices_buffer_;
    VulkanMemory* indices_memory_;

    const std::vector<VkDynamicState> dynamic_states_ = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    const std::vector<vertex_t> vertices_ = {
            {{0.5f, -0.5f, 0.4f}, {1.0f, 0.0f, 0.0f}},
            {{-0.5f, -0.5f, 0.4f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f, 0.4f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f, 1.0f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, -0.5f, 0.3f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f, 0.3f}, {0.0f, 1.0f, 0.0f}},
    };
    const std::vector<uint16_t> indices = { 0, 1, 2, 3, 4, 5};
    const std::vector<VkVertexInputBindingDescription> vertex_binding_descriptions_ = {{0, sizeof (vertex_t), VK_VERTEX_INPUT_RATE_VERTEX}};
    const std::vector<VkVertexInputAttributeDescription> vertex_attribute_descriptions_ = {
            {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex_t, pos)},
            {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex_t, color)}};
};
