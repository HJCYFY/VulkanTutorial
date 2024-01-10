//
// Created by hj6231 on 2023/12/26.
//

#include "vulkan_command_pool.h"

VulkanCommandPool::VulkanCommandPool(VkDevice device, VkCommandPool command_pool) :
    device_(device), command_pool_(command_pool) {

}

VulkanCommandPool::~VulkanCommandPool() {
    vkDestroyCommandPool(device_, command_pool_, nullptr);
}

VkCommandPool VulkanCommandPool::command_pool() const {
    return command_pool_;
}

VulkanCommandBuffer* VulkanCommandPool::AllocateCommandBuffer(
        VkCommandBufferLevel level) {
    /* typedef struct VkCommandBufferAllocateInfo {
        VkStructureType         sType;
        const void*             pNext; // pNext must be NULL
        VkCommandPool           commandPool;
        VkCommandBufferLevel    level;
        uint32_t                commandBufferCount;
    } VkCommandBufferAllocateInfo; */
    VkCommandBuffer command_buffer;
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = command_pool_;
    alloc_info.level = level;
    alloc_info.commandBufferCount = 1;
    VkResult ret = vkAllocateCommandBuffers(device_, &alloc_info, &command_buffer);
    if (ret == VK_SUCCESS) {
        return new VulkanCommandBuffer(device_, command_pool_, command_buffer);
    }
    return nullptr;
}

void VulkanCommandPool::FreeCommandBuffer(VulkanCommandBuffer** buffer) {
    if (*buffer) {
        delete *buffer;
        *buffer = nullptr;
    }
}