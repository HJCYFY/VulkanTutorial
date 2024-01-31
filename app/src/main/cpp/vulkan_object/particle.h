//
// Created by hj6231 on 2024/1/25.
//

#pragma once
#include "vulkan_object.h"
#include "glm/glm.hpp"

typedef struct {
    glm::vec2 pos;
    glm::vec2 velocity;
    glm::vec4 color;
} particle_t;

typedef struct{
    float t;
} delta_time_t;

class Particle {
public:
    Particle(VulkanLogicDevice* device,
             VkFormat swap_chain_image_format,
             VkExtent2D frame_buffer_size);
    ~Particle() = default;

    int CreatePipeline();
    void DestroyPipeline();

    void Draw(const VulkanCommandBuffer* command_buffer) const;

    VkBuffer GetVertexBuffer() const;

    const static int PARTICLE_COUNT = 8192;
private:
    void LoadResource();

    void CreateUinformBuffer();
    void DestroyUinformBuffer();
    void CopyDataToUinformBuffer() const;

    void CreateStorageBuffer();
    void DestroyStorageBuffer();
    void CopyDataToyStorageBuffer(void* data, size_t size);

    void CreateDescriptorPool();
    void CreateDescriptorSetLayout();
    void CreateDescriptorSet();
    void UpdateDescriptorSet();
    void CreatePipelineLayout();

    VulkanShaderModule* CreateShaderModule(const std::string& name,
                                           shaderc_shader_kind kind,
                                           const std::string& source) const;

    VulkanLogicDevice* device_;
    VkFormat swap_chain_image_format_;
    VkExtent2D frame_buffer_size_;

    VulkanPipeline* pipeline_;

    VulkanDescriptorPool* descriptor_pool_;
    VulkanDescriptorSetLayout* descriptor_set_layout_;
    VulkanDescriptorSet* descriptor_set_;
    VulkanPipelineLayout* pipeline_layout_;

    VulkanBuffer* uniform_buffer_;
    VulkanMemory* uniform_buffer_memory_;
    void* uniform_buffer_data_;

    VulkanBuffer* storage_buffer_;
    VulkanMemory* storage_buffer_memory_;
};


