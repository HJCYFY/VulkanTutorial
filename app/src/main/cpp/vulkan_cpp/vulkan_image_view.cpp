//
// Created by hj6231 on 2023/12/21.
//

#include "vulkan_image_view.h"

VulkanImageView::VulkanImageView(VkDevice device, VkImageView image_view) :
        device_(device),
        image_view_(image_view){

}

VulkanImageView::~VulkanImageView() {
    vkDestroyImageView(device_, image_view_, nullptr);
}

VkImageView VulkanImageView::image_view() const {
    return image_view_;
}