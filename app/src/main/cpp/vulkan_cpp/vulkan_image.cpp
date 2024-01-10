//
// Created by hj6231 on 2023/12/21.
//

#include "vulkan_image.h"


VulkanImage::VulkanImage(VkDevice device, VkImage image) :
    device_(device), image_(image) {

}

VulkanImage::~VulkanImage() {
    vkDestroyImage(device_, image_, nullptr);
}

VkImage VulkanImage::image() const {
    return image_;
}

void VulkanImage::GetImageMemoryRequirements(VkMemoryRequirements* mem_requirements) {
    vkGetImageMemoryRequirements(device_, image_, mem_requirements);
}