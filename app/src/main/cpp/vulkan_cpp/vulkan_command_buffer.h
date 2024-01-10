//
// Created by hj6231 on 2023/12/27.
//

#pragma once
#include <vulkan/vulkan.h>

class VulkanCommandBuffer {
public:
    VulkanCommandBuffer(VkDevice device, VkCommandPool command_pool, VkCommandBuffer command_buffer);
    VulkanCommandBuffer(const VulkanCommandBuffer&) = delete;
    ~VulkanCommandBuffer();

    VkCommandBuffer command_buffer();

    VkResult ResetCommandBuffer(VkCommandBufferResetFlags flags);

    VkResult BeginCommandBuffer(const VkCommandBufferBeginInfo* info) const;
    VkResult EndCommandBuffer() const;
    void CmdBeginRenderPass(const VkRenderPassBeginInfo* info, VkSubpassContents contents) const;
    void CmdEndRenderPass() const;
    void CmdBindPipeline(VkPipelineBindPoint bind_point, VkPipeline pipeline) const;
    void CmdSetViewport(uint32_t viewport_count, const VkViewport* viewports) const;
    void CmdSetScissor(uint32_t scissor_count, const VkRect2D* scissors) const;
    void CmdDraw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) const;
    void CmdDrawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index,
                        int32_t vertex_offset, uint32_t first_instance) const;

    void CmdBindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType index_type) const;
    void CmdBindVertexBuffers(uint32_t first_binding, uint32_t binding_count,
                              const VkBuffer* buffers,  const VkDeviceSize* offsets) const;
    void CmdBindDescriptorSets(VkPipelineBindPoint pipeline_bind_point,
                               VkPipelineLayout layout,
                               uint32_t first_set,
                               uint32_t descriptor_set_count,
                               const VkDescriptorSet* descriptor_sets,
                               uint32_t dynamic_offset_count,
                               const uint32_t* dynamic_offsets) const;

    void CmdPipelineBarrier(VkPipelineStageFlags src_stage_mask,
                            VkPipelineStageFlags dst_stage_mask,
                            VkDependencyFlags dependency_flags,
                            uint32_t memory_barrier_count,
                            const VkMemoryBarrier* memory_barriers,
                            uint32_t buffer_memory_barrier_count,
                            const VkBufferMemoryBarrier* buffer_memory_barriers,
                            uint32_t image_memory_barrier_count,
                            const VkImageMemoryBarrier* image_memory_barriers) const;

    void CmdCopyBufferToImage(VkBuffer src_buffer, VkImage dst_image,
                              VkImageLayout dst_image_layout,
                              uint32_t region_count,
                              const VkBufferImageCopy* regions);

    VulkanCommandBuffer& operator = (const VulkanCommandBuffer&) = delete;
private:
    VkDevice device_;
    VkCommandPool command_pool_;
    VkCommandBuffer command_buffer_;
};


