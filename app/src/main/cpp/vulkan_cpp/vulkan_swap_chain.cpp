//
// Created by hj6231 on 2023/12/21.
//

#include "vulkan_swap_chain.h"


VulkanSwapChain::VulkanSwapChain(VkDevice device, VkSwapchainKHR swap_chain) :
        device_(device), swap_chain_(swap_chain) {

}

VulkanSwapChain::~VulkanSwapChain() {
    vkDestroySwapchainKHR(device_, swap_chain_, nullptr);
}

VkSwapchainKHR VulkanSwapChain::swap_chain() {
    return swap_chain_;
}

std::vector<VkImage> VulkanSwapChain::GetImages() const {
    uint32_t image_count;
    vkGetSwapchainImagesKHR(device_, swap_chain_, &image_count, nullptr);
    std::vector<VkImage> swap_chain_images(image_count);
    vkGetSwapchainImagesKHR(device_, swap_chain_, &image_count, swap_chain_images.data());
    return swap_chain_images;
}