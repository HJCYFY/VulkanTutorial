//
// Created by hj6231 on 2023/12/19.
//

#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include "vulkan_physical_device.h"
#include "vulkan_surface.h"

class VulkanInstance {
public:
    static std::vector<VkLayerProperties> EnumerateInstanceLayerProperties();
    static std::vector<VkExtensionProperties> EnumerateExtensionProperties();
    static VulkanInstance* CreateInstance(const VkInstanceCreateInfo* info);
    static void DestroyInstance(VulkanInstance** instance);

    ~VulkanInstance();

    std::vector<VulkanPhysicalDevice> EnumeratePhysicalDevices() const;


    VkResult CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* info,
                                          VkDebugUtilsMessengerEXT* messenger) const;
    void DestroyDebugUtilsMessengerEXT(VkDebugUtilsMessengerEXT messenger) const;

    VkResult CreateDebugReportCallbackEXT(const VkDebugReportCallbackCreateInfoEXT* create_info,
                                           VkDebugReportCallbackEXT* callback) const;
    void DestroyDebugReportCallbackEXT(VkDebugReportCallbackEXT callback) const;

    VulkanSurface* CreateAndroidSurface(const VkAndroidSurfaceCreateInfoKHR* info) const;
    static void DestroyAndroidSurface(VulkanSurface** surface);
private:
    VulkanInstance();
    VulkanInstance(const VulkanInstance&) = delete;
    VkInstance vk_instance_;
};
