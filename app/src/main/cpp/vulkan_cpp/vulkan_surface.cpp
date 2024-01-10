//
// Created by hj6231 on 2023/12/20.
//

#include "vulkan_surface.h"

VulkanSurface::VulkanSurface(VkInstance instance, VkSurfaceKHR surface) :
        instance_(instance),
        surface_(surface){

}
VulkanSurface::~VulkanSurface() {
    vkDestroySurfaceKHR(instance_, surface_, nullptr);
}

VkSurfaceKHR VulkanSurface::surface() const {
    return surface_;
}