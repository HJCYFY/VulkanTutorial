//
// Created by hj6231 on 2023/12/20.
//

#include "vulkan_queue.h"

VulkanQueue::VulkanQueue(VkQueue queue) :
        queue_(queue) {

}

VulkanQueue::~VulkanQueue() {

}

VkResult VulkanQueue::QueueSubmit(uint32_t submitCount, const VkSubmitInfo* submits, VkFence fence) const {
    return vkQueueSubmit(queue_, submitCount, submits, fence);
}

VkResult VulkanQueue::QueuePresentKHR(const VkPresentInfoKHR* present_info) const {
    return vkQueuePresentKHR(queue_, present_info);
}

VkResult VulkanQueue::QueueWaitIdle() const {
    return vkQueueWaitIdle(queue_);
}