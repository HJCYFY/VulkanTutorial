//
// Created by hj6231 on 2023/12/21.
//

#pragma once
#include <vulkan/vulkan.h>

class VulkanImageView {
public:
    VulkanImageView(VkDevice device, VkImageView image_view);
    VulkanImageView(const VulkanImageView& ) = delete;
    ~VulkanImageView();

    VkImageView image_view() const;

    VulkanImageView& operator = (const VulkanImageView&) = delete;
private:
    VkDevice device_;
    VkImageView image_view_;
};
