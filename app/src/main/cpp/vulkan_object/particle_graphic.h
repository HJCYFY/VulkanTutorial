//
// Created by hj6231 on 2024/1/26.
//

#pragma once
#include "particle.h"

class ParticleGraphic : public VulkanObject {
public:
    ParticleGraphic(VulkanLogicDevice* device,
                    VkFormat swap_chain_image_format,
                    VkExtent2D frame_buffer_size,
                    Particle* particle);
    ~ParticleGraphic() = default;

    int CreatePipeline() override;
    void DestroyPipeline() override;

    void Draw(const VulkanCommandBuffer* command_buffer,
              const VulkanFrameBuffer* frame_buffer) const override;
protected:
    void LoadResource() override {}
    void CreateRenderPass() override {};
    void CreateVertexBuffer() override{}
    void DestroyVertexBuffer() override{}
    void CreateIndexBuffer() override {}
    void DestroyIndexBuffer() override {}
    void CreateDescriptorSets() override {}

private:
    Particle* particle_;
    VulkanPipelineLayout* pipeline_layout_;
};


