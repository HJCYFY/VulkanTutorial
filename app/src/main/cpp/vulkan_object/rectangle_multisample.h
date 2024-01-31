//
// Created by hj6231 on 2024/1/19.
//

#pragma once
#include "vulkan_object.h"
#include <glm/glm.hpp>

class RectangleMultisample : public VulkanObject {
public:
    RectangleMultisample(VulkanLogicDevice* device,
    VkFormat swap_chain_image_format,
            VkExtent2D frame_buffer_size);
    ~RectangleMultisample() = default;
    typedef struct  {
        glm::vec2 pos;
        glm::vec3 color;
    } vertex_t;

    typedef struct  {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 project;
    } mvp_t;

    int CreatePipeline() override;
    void DestroyPipeline() override;

    void Draw(const VulkanCommandBuffer* command_buffer,
              const VulkanFrameBuffer* frame_buffer) const override;
protected:
    void LoadResource() override;
    void CreateRenderPass() override;
    void CreateVertexBuffer() override;
    void DestroyVertexBuffer() override;

    void CreateIndexBuffer() override {}
    void DestroyIndexBuffer() override {}

    void CreateDescriptorSets() override;

    void CreateColorAttachment();
    void DestroyColorAttachment();

    void CreateUniformBufferAndMap();
    void CopyDataToUniformBuffer() const;
    void UnmapAndDestroyUniformBuffer();

    VulkanDescriptorPool*  CreateDescriptorPool();
    VulkanDescriptorSetLayout* CreateDescriptorSetLayout() const;
    VulkanDescriptorSet* CreateDescriptorSet(VulkanDescriptorPool* pool, VulkanDescriptorSetLayout* set_layout);
    void BindDescriptorSetWithBuffer();

    VulkanPipelineLayout* CreatePipelineLayout(const VulkanDescriptorSetLayout* descriptor_set_layout) const;
    VulkanPipeline* CreateGraphicsPipeline(const VulkanShaderModule* vert_shader_module,
                                           const VulkanShaderModule* frag_shader_module,
                                           const VulkanPipelineLayout* layout,
                                           const VulkanRenderPass* render_pass) const;

    VulkanDescriptorSetLayout* descriptor_set_layout_;
    VulkanBuffer* vertex_buffer_;
    VulkanBuffer* uniform_buffer_;
    VulkanMemory* vertex_memory_;
    VulkanMemory* uniform_memory_;
    void* uniform_buffer_mapped_;
    VulkanPipelineLayout* pipeline_layout_;
    VulkanDescriptorPool* vulkan_descriptor_pool_;
    VulkanDescriptorSet* vulkan_descriptor_set_;

private:// GraphicsPipelineCreateInfo
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

