//
// Created by hj6231 on 2024/1/2.
//

#pragma once
#include "vulkan_object.h"
#include <glm/glm.hpp>

class RotateRectangle : public VulkanObject {
public:
    RotateRectangle(VulkanLogicDevice* device);
    ~RotateRectangle() = default;

    typedef struct  {
        glm::vec2 pos;
        glm::vec3 color;
    } vertex_t;

    typedef struct  {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 project;
    } mvp_t;

    int CreatePipeline(const VulkanRenderPass* render_pass) override;
    void DestroyPipeline() override;

    void Draw(const VulkanCommandBuffer* command_buffer,
              const VulkanRenderPass* render_pass,
              const VulkanFrameBuffer* frame_buffer,
              uint32_t frame_buffer_width,
              uint32_t frame_buffer_height) const override;
protected:
    void CreateVertexBuffer();
    void CopyDataToVertexBuffer();
    void DestroyVertexBuffer();

    void CreateUniformBufferAndMap();
    void CopyDataToUniformBuffer(uint32_t frame_buffer_width, uint32_t frame_buffer_height) const;
    void UnmapAndDestroyUniformBuffer();

    VulkanDescriptorPool*  CreateDescriptorPool();
    VulkanDescriptorSetLayout* CreateDescriptorSetLayout() const;
    VulkanDescriptorSet* CreateDescriptorSet(VulkanDescriptorPool* pool, VulkanDescriptorSetLayout* set_layout);
    void BindDescriptorSetWithBuffer();

    VulkanPipelineLayout* CreatePipelineLayout(const VulkanDescriptorSetLayout* descriptor_set_layout) const;
    VulkanPipeline* CreateGraphicsPipeline(const VulkanShaderModule* vert_shader_module,
                                           const VulkanShaderModule* frag_shader_module,
                                           const VulkanPipelineLayout* layout,
                                           const VulkanRenderPass* render_pass) const override;
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


