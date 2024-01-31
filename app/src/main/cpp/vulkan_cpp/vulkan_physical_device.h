//
// Created by hj6231 on 2023/12/20.
//

#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include "vulkan_logic_device.h"

class VulkanPhysicalDevice {
public:
    VulkanPhysicalDevice(VkInstance instance, VkPhysicalDevice device);
    VulkanPhysicalDevice(const VulkanPhysicalDevice& device);
    ~VulkanPhysicalDevice() = default;

    void GetProperties(VkPhysicalDeviceProperties* device_properties) const;
    void GetProperties2(VkPhysicalDeviceProperties2* device_properties) const;
    void GetFeatures(VkPhysicalDeviceFeatures * device_features) const;
    void GetFeatures2(VkPhysicalDeviceFeatures2 * device_features) const;
    void GetFormatProperties(VkFormat format, VkFormatProperties* props) const;
    void GetMemoryProperties(VkPhysicalDeviceMemoryProperties* memory_properties) const;

    std::vector<VkExtensionProperties> EnumerateExtensionProperties() const;
    std::vector<VkLayerProperties> EnumerateLayerProperties() const;

    std::vector<VkQueueFamilyProperties> GetQueueFamilyProperties() const;

    bool GetSurfaceSupport(uint32_t queue_family_index, VkSurfaceKHR surface) const;
    VkResult GetSurfaceCapabilities(VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* capabilities) const;
    std::vector<VkSurfaceFormatKHR> GetSurfaceFormats(VkSurfaceKHR surface) const;
    std::vector<VkPresentModeKHR> GetSurfacePresentModes(VkSurfaceKHR surface) const;

    VulkanLogicDevice* CreateDevice(const VkDeviceCreateInfo* info) const;
    static void DestroyDevice(VulkanLogicDevice** device);
private:
    VkInstance instance_;
    VkPhysicalDevice device_;
};
