//
// Created by hj6231 on 2024/1/3.
//
#pragma once
#include "vulkan_object.h"
#include <glm/glm.hpp>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

class Nv12ImageTexture : public VulkanObject {
public:
    Nv12ImageTexture(AAssetManager* asset_manager,
                     VulkanCommandPool* command_pool,
                     VulkanQueue* graphic_queue,
                     VulkanLogicDevice* device,
                     VkFormat swap_chain_image_format,
                     VkExtent2D frame_buffer_size);
    ~Nv12ImageTexture() = default;

    typedef struct  {
        glm::vec2 pos;
        glm::vec2 coordinate;
    } vertex_t;

    int CreatePipeline() override;
    void DestroyPipeline() override;

    void Draw(const VulkanCommandBuffer* command_buffer,
              const VulkanFrameBuffer* frame_buffer) const override;
protected:
    void LoadResource() override;
    void CreateRenderPass() override;

    void CreateVertexBuffer() override;
    void DestroyVertexBuffer() override;

    void CreateIndexBuffer() override;
    void DestroyIndexBuffer() override;

    void CreateDescriptorSets() override;
private:
    VulkanSamplerYcbcrConversion*  CreateSamplerYcbcrConversion();
    void CreateTextureImage();
    void CreateTextureImageView(const VkSamplerYcbcrConversionInfo* ycbcr_conversion_info);
    void CreateTextureSampler(const VkSamplerYcbcrConversionInfo* ycbcr_conversion_info);
    void CreateImage(uint32_t width, uint32_t height, VulkanImage*& image, VulkanMemory*& image_memory);
    void TransitionImageLayout(VkImage image, VkImageLayout oldLayout,
                               VkImageLayout newLayout);
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    VulkanDescriptorPool*  CreateDescriptorPool();
    VulkanDescriptorSetLayout* CreateDescriptorSetLayout() const;
    void BindDescriptorSetWithBuffer() const;

    VulkanPipelineLayout* CreatePipelineLayout(const VulkanDescriptorSetLayout* descriptor_set_layout) const;
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

    static const char kVertShaderSource[];
    static const char kFragShaderSource[];

    const std::vector<VkDynamicState> dynamic_states_ = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    const std::vector<vertex_t> vertices_ = {
            {{0.5f, -0.5f}, {1.0f, 1.0f}},
            {{-0.5f, -0.5f}, {0.0f, 1.0f}},
            {{0.5f, 0.5f}, {1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f}}
    };
    const std::vector<uint16_t> indices = { 0, 1, 2, 1, 2, 3};
    const std::vector<VkVertexInputBindingDescription> vertex_binding_descriptions_ = {{0, sizeof (vertex_t), VK_VERTEX_INPUT_RATE_VERTEX}};
    const std::vector<VkVertexInputAttributeDescription> vertex_attribute_descriptions_ = {
            {0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex_t, pos)},
            {1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex_t, coordinate)}};

    AAssetManager* asset_manager_;
    VulkanCommandPool* command_pool_;
    VulkanQueue* graphic_queue_;

    VulkanImage* texture_image_;
    VulkanMemory* texture_image_memory_;
    VulkanImageView* texture_image_view_;
    VulkanSampler* texture_image_sampler_;

    VulkanBuffer* vertex_buffer_;
    VulkanMemory* vertex_memory_;

    VulkanBuffer* indices_buffer_;
    VulkanMemory* indices_memory_;

    VulkanDescriptorPool* vulkan_descriptor_pool_;
    VulkanDescriptorSetLayout* descriptor_set_layout_;
    VulkanDescriptorSet* vulkan_descriptor_set_;

    VulkanPipelineLayout* pipeline_layout_;
};


