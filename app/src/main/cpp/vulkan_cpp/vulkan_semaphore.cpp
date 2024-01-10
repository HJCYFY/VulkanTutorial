//
// Created by hj6231 on 2023/12/27.
//

#include "vulkan_semaphore.h"


VulkanSemaphore::VulkanSemaphore(VkDevice device, VkSemaphore semaphore) :
    device_(device), semaphore_(semaphore) {

}

VulkanSemaphore::~VulkanSemaphore() {
    vkDestroySemaphore(device_, semaphore_, nullptr);
}

VkSemaphore VulkanSemaphore::semaphore() const {
    return semaphore_;
}
