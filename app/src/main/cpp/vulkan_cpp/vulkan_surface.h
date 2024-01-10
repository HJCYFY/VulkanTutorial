//
// Created by hj6231 on 2023/12/20.
//

#pragma once
#include <vulkan/vulkan.h>

class VulkanSurface {
public:
    VulkanSurface(VkInstance instance, VkSurfaceKHR surface);
    VulkanSurface(const VulkanSurface& surface) = delete;
    ~VulkanSurface();

    VkSurfaceKHR surface() const;
    VulkanSurface& operator = (const VulkanSurface& surface) = delete;
private:
    VkInstance instance_;
    VkSurfaceKHR surface_;
};

