//
// Created by hj6231 on 2024/1/17.
//

#pragma once

#include "vulkan_object.h"
#include <glm/glm.hpp>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

class VikingRoomMipmap : public VulkanObject {
public:
    VikingRoomMipmap(AAssetManager* asset_manager,
    VulkanCommandPool* command_pool,
            VulkanQueue* graphic_queue,
    VulkanLogicDevice* device,
            VkFormat swap_chain_image_format,
    VkExtent2D frame_buffer_size);

    ~VikingRoomMipmap() = default;
    int CreatePipeline() override;
    void DestroyPipeline() override;

    struct vertex_t {
        glm::vec3 pos;
        glm::vec2 coordinate;
    };

    typedef struct  {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 project;
    } mvp_t;

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
    void ReadVerticesIndexes();

    void CreateMvpBuffer();
    void DestroyMvpBuffer();

    void CreateTextureImage();
    void DestroyTextureImage();
    void CreateTextureImageViewAndSampler();
    void DestroyTextureImageViewAndSampler();

    void CreateDepthImage();
    void DestroyDepthImage();
    void CreateDepthImageView();
    void DestroyDepthImageView();

    VulkanPipelineLayout* CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& set_layouts) const;
    VulkanPipeline* CreateGraphicsPipeline(const VulkanShaderModule* vert_shader_module,
                                           const VulkanShaderModule* frag_shader_module,
                                           const VulkanPipelineLayout* layout,
                                           const VulkanRenderPass* render_pass) const;
    void CreateImage(uint32_t width, uint32_t height, VkFormat format,
                     VkImageTiling tiling, VkImageUsageFlags usage,
                     VulkanImage*& image, VulkanMemory*& image_memory, uint32_t mip_levels = 1) const;
    void TransitionImageLayout(VkImage image, VkImageLayout oldLayout,
                               VkImageLayout newLayout, uint32_t mip_levels) const;
    void GenerateMipmaps(VkImage image, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) const;
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const;

    VulkanDescriptorPool*  CreateDescriptorPool() const;
    VulkanDescriptorSetLayout* CreateDescriptorSetLayout() const;
    void BindTextureDescriptorSetWithImage() const;
    void BindMvpDescriptorSetWithBuffer() const;
    void CopyDataToUniformBuffer() const;

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

    AAssetManager* asset_manager_;
    VulkanCommandPool* command_pool_;
    VulkanQueue* graphic_queue_;

    VulkanPipelineLayout* pipeline_layout_;

    VulkanBuffer* vertex_buffer_;
    VulkanMemory* vertex_memory_;

    VulkanBuffer* indices_buffer_;
    VulkanMemory* indices_memory_;

    VulkanBuffer* mvp_buffer_;
    VulkanMemory* mvp_memory_;
    void* uniform_buffer_mapped_;

    uint32_t mip_levels_;
    VulkanImage* texture_image_;
    VulkanMemory* texture_image_memory_;
    VulkanImageView* texture_image_view_;
    VulkanSampler* texture_image_sampler_;

    VulkanDescriptorPool* descriptor_pool_;
    VulkanDescriptorSetLayout* descriptor_set_layout_;
    VulkanDescriptorSet* descriptor_set_;

    std::vector<vertex_t> vertices_;
    const std::vector<uint16_t> indices_ = {
            0,1,2,3,4,5
    };

    const std::vector<VkDynamicState> dynamic_states_ = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    const std::vector<VkVertexInputBindingDescription> vertex_binding_descriptions_ = {{0, sizeof (vertex_t), VK_VERTEX_INPUT_RATE_VERTEX}};
    const std::vector<VkVertexInputAttributeDescription> vertex_attribute_descriptions_ = {
            {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex_t, pos)},
            {1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex_t, coordinate)}};
};

