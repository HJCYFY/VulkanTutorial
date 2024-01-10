////
//// Created by hj6231 on 2024/1/5.
////
//
//#pragma once
//#include "vulkan_object.h"
//#include <glm/glm.hpp>
//#include <android/asset_manager.h>
//#include <android/asset_manager_jni.h>
//
//class RgbaImageTexture : public VulkanObject {
//public:
//    RgbaImageTexture(AAssetManager* asset_manager,
//                     VulkanCommandPool* command_pool,
//                     VulkanQueue* graphic_queue,
//                     VulkanLogicDevice* device,
//                     VkFormat swap_chain_image_format,
//                     VkExtent2D frame_buffer_size);
//    ~RgbaImageTexture() = default;
//
//    typedef struct  {
//        glm::vec2 pos;
//        glm::vec2 coordinate;
//    } vertex_t;
//
//    int CreatePipeline(const VulkanRenderPass* render_pass) override;
//    void DestroyPipeline() override;
//
//    void Draw(const VulkanCommandBuffer* command_buffer,
//              const VulkanRenderPass* render_pass,
//              const VulkanFrameBuffer* frame_buffer,
//              uint32_t frame_buffer_width,
//              uint32_t frame_buffer_height) const override;
//protected:
//    void CreateVertexBuffer();
//    void CopyDataToVertexBuffer();
//    void DestroyVertexBuffer();
//
//    void CreateTextureImage();
//    void CreateTextureImageView();
//    void CreateTextureSampler();
//        void CreateImage(uint32_t width, uint32_t height, VkFormat format,
//                         VkImageTiling tiling, VkImageUsageFlags usage,
//                         VkMemoryPropertyFlags properties, VulkanImage*& image,
//                         VulkanMemory*& image_memory);
//        void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
//                                   VkImageLayout newLayout);
//        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
//
//    VulkanDescriptorPool*  CreateDescriptorPool();
//    VulkanDescriptorSetLayout* CreateDescriptorSetLayout() const;
//    VulkanDescriptorSet* CreateDescriptorSet(VulkanDescriptorPool* pool, VulkanDescriptorSetLayout* set_layout);
//    void BindDescriptorSetWithBuffer() const;
//
//    VulkanPipelineLayout* CreatePipelineLayout(const VulkanDescriptorSetLayout* descriptor_set_layout) const;
//    VulkanPipeline* CreateGraphicsPipeline(const VulkanShaderModule* vert_shader_module,
//                                           const VulkanShaderModule* frag_shader_module,
//                                           const VulkanPipelineLayout* layout,
//                                           const VulkanRenderPass* render_pass) const override;
//    AAssetManager* asset_manager_;
//    VulkanCommandPool* command_pool_;
//    VulkanQueue* graphic_queue_;
//    VulkanBuffer* vertex_buffer_;
//    VulkanMemory* vertex_memory_;
//    VulkanImage* texture_image_;
//    VulkanMemory* texture_image_memory_;
//    VulkanImageView* texture_image_view_;
//    VulkanSampler* texture_image_sampler_;
//    VulkanPipelineLayout* pipeline_layout_;
//    VulkanDescriptorPool* vulkan_descriptor_pool_;
//    VulkanDescriptorSet* vulkan_descriptor_set_;
//
//private:// GraphicsPipelineCreateInfo
//    static std::vector<VkPipelineShaderStageCreateInfo> GetPipelineShaderStageCreateInfos(
//            const VulkanShaderModule* vert_shader_module, const VulkanShaderModule* frag_shader_module);
//    VkPipelineVertexInputStateCreateInfo GetPipelineVertexInputStateCreateInfo() const;
//    static VkPipelineInputAssemblyStateCreateInfo  GetPipelineInputAssemblyStateCreateInfo();
//    static VkPipelineViewportStateCreateInfo GetPipelineViewportStateCreateInfo();
//    static VkPipelineRasterizationStateCreateInfo GetPipelineRasterizationStateCreateInfo();
//    static VkPipelineMultisampleStateCreateInfo GetPipelineMultisampleStateCreateInfo();
//    static VkPipelineColorBlendStateCreateInfo GetPipelineColorBlendStateCreateInfo(
//            const VkPipelineColorBlendAttachmentState* color_blend_attachment);
//    static VkPipelineDynamicStateCreateInfo  GetPipelineDynamicStateCreateInfo(const std::vector<VkDynamicState>& dynamic_states);
//    static VkPipelineColorBlendAttachmentState GetPipelineColorBlendAttachmentState();
//
//    static const char kVertShaderSource[];
//    static const char kFragShaderSource[];
//
//    const std::vector<VkDynamicState> dynamic_states_ = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
//    const std::vector<vertex_t> vertices_ = {
//            {{0.5f, -0.5f}, {1.0f, 1.0f}},
//            {{-0.5f, -0.5f}, {0.0f, 1.0f}},
//            {{0.5f, 0.5f}, {1.0f, 0.0f}},
//            {{-0.5f, 0.5f}, {0.0f, 0.0f}}
//    };
//    const std::vector<VkVertexInputBindingDescription> vertex_binding_descriptions_ = {{0, sizeof (vertex_t), VK_VERTEX_INPUT_RATE_VERTEX}};
//    const std::vector<VkVertexInputAttributeDescription> vertex_attribute_descriptions_ = {
//            {0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex_t, pos)},
//            {1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex_t, coordinate)}};
//};
//
