//
// Created by hj6231 on 2023/12/21.
//

#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include "vulkan_image.h"

class VulkanSwapChain {
public:
    VulkanSwapChain(VkDevice device, VkSwapchainKHR swap_chain);
    VulkanSwapChain(const VulkanSwapChain&) = delete;
    ~VulkanSwapChain();
    VkSwapchainKHR swap_chain();
    std::vector<VkImage> GetImages() const;
    VulkanSwapChain& operator = (const VulkanSwapChain&) = delete;
private:
    VkDevice device_;
    VkSwapchainKHR swap_chain_;
};
