//
// Created by hj6231 on 2023/12/27.
//

#include "vulkan_command_buffer.h"

VulkanCommandBuffer::VulkanCommandBuffer(VkDevice device,
                                         VkCommandPool command_pool,
                                         VkCommandBuffer command_buffer) :
        device_(device), command_pool_(command_pool), command_buffer_(command_buffer){

}

VulkanCommandBuffer::~VulkanCommandBuffer() {
    vkFreeCommandBuffers(device_, command_pool_, 1, &command_buffer_);
}

VkCommandBuffer VulkanCommandBuffer::command_buffer() {
    return command_buffer_;
}

VkResult VulkanCommandBuffer::ResetCommandBuffer(VkCommandBufferResetFlags flags) {
    return vkResetCommandBuffer(command_buffer_, flags);
}

VkResult VulkanCommandBuffer::BeginCommandBuffer(const VkCommandBufferBeginInfo* info) const {
    return vkBeginCommandBuffer(command_buffer_, info);
}

VkResult VulkanCommandBuffer::EndCommandBuffer() const {
    return vkEndCommandBuffer(command_buffer_);
}

void VulkanCommandBuffer::CmdBeginRenderPass(const VkRenderPassBeginInfo* info, VkSubpassContents contents) const {
    vkCmdBeginRenderPass(command_buffer_, info, contents);
}

void VulkanCommandBuffer::CmdEndRenderPass() const {
    vkCmdEndRenderPass(command_buffer_);
}

void VulkanCommandBuffer::CmdBindPipeline(VkPipelineBindPoint bind_point, VkPipeline pipeline) const {
    vkCmdBindPipeline(command_buffer_, bind_point, pipeline);
}

void VulkanCommandBuffer::CmdBindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType index_type) const {
    vkCmdBindIndexBuffer(command_buffer_, buffer, offset, index_type);
}

void VulkanCommandBuffer::CmdBindVertexBuffers(uint32_t first_binding, uint32_t binding_count,
                                               const VkBuffer* buffers,  const VkDeviceSize* offsets) const {
    vkCmdBindVertexBuffers(command_buffer_, first_binding, binding_count, buffers, offsets);
}

void VulkanCommandBuffer::CmdBindDescriptorSets(VkPipelineBindPoint pipeline_bind_point,
                                                VkPipelineLayout layout,
                                                uint32_t first_set,
                                                uint32_t descriptor_set_count,
                                                const VkDescriptorSet* descriptor_sets,
                                                uint32_t dynamic_offset_count,
                                                const uint32_t* dynamic_offsets) const {
    vkCmdBindDescriptorSets(command_buffer_,
                            pipeline_bind_point,
                            layout,
                            first_set,
                            descriptor_set_count, descriptor_sets,
                            dynamic_offset_count, dynamic_offsets);
}


void VulkanCommandBuffer::CmdPipelineBarrier(VkPipelineStageFlags src_stage_mask,
                                             VkPipelineStageFlags dst_stage_mask,
                                             VkDependencyFlags dependency_flags,
                                             uint32_t memory_barrier_count,
                                             const VkMemoryBarrier* memory_barriers,
                                             uint32_t buffer_memory_barrier_count,
                                             const VkBufferMemoryBarrier* buffer_memory_barriers,
                                             uint32_t image_memory_barrier_count,
                                             const VkImageMemoryBarrier* image_memory_barriers) const {
    vkCmdPipelineBarrier(command_buffer_, src_stage_mask, dst_stage_mask, dependency_flags,
                         memory_barrier_count, memory_barriers,
                         buffer_memory_barrier_count, buffer_memory_barriers,
                         image_memory_barrier_count, image_memory_barriers);
}

void VulkanCommandBuffer::CmdCopyBufferToImage(VkBuffer src_buffer, VkImage dst_image,
                                               VkImageLayout dst_image_layout,
                                               uint32_t region_count,
                                               const VkBufferImageCopy* regions) {
    vkCmdCopyBufferToImage(command_buffer_, src_buffer, dst_image,
                           dst_image_layout, region_count, regions);
}

void VulkanCommandBuffer::CmdSetViewport(uint32_t viewport_count, const VkViewport* viewports) const {
    vkCmdSetViewport(command_buffer_, 0, viewport_count, viewports);
}

void VulkanCommandBuffer::CmdSetScissor(uint32_t scissor_count, const VkRect2D* scissors) const {
    vkCmdSetScissor(command_buffer_, 0, scissor_count, scissors);
}

void VulkanCommandBuffer::CmdDraw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) const {
    vkCmdDraw(command_buffer_, vertex_count, instance_count, first_vertex, first_instance);
}

void VulkanCommandBuffer::CmdDrawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index,
                                         int32_t vertex_offset, uint32_t first_instance) const {
    vkCmdDrawIndexed(command_buffer_, index_count, instance_count, first_index, vertex_offset, first_instance);
}