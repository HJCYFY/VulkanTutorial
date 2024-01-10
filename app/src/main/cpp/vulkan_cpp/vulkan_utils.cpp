//
// Created by hj6231 on 2023/12/22.
//

#include "vulkan_utils.h"


bool CheckValidationLayerSupport(const std::vector<VkLayerProperties>& layer_properties) {
    bool support = false;
    for (VkLayerProperties layer_property : layer_properties) {
        if (strcmp("VK_LAYER_KHRONOS_validation", layer_property.layerName) == 0) {
            support = true;
            break;
        }
    }
    return support;
}

bool CheckPhysicalDeviceGraphicSupport(const VulkanPhysicalDevice& device) {
    std::vector<VkQueueFamilyProperties> family_properties = device.GetQueueFamilyProperties();
    bool graphic =
            std::any_of(family_properties.begin(), family_properties.end(),
                [](VkQueueFamilyProperties it) {
                          return (it.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;
                      });
    return graphic;
}

bool CheckPhysicalDeviceSwapChainSupport(const VulkanPhysicalDevice& device) {
    std::vector<VkExtensionProperties> extensions_properties = device.EnumerateExtensionProperties();
    for (auto extensions_property : extensions_properties) {
        if (strcmp(extensions_property.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
            return true;
        }
    }
    return false;
}

bool CheckPhysicalDeviceSamplerAnisotropySupport(const VulkanPhysicalDevice& device) {
    VkPhysicalDeviceFeatures device_features{};
    device.GetFeatures(&device_features);
    return device_features.samplerAnisotropy;
}

void GetGraphicQueueFamilyIndexes(const VulkanPhysicalDevice& device, uint32_t* indexes, uint32_t max_len, uint32_t* len) {
    std::vector<VkQueueFamilyProperties> family_properties = device.GetQueueFamilyProperties();
    int num = 0;
    for (uint32_t i = 0; i < family_properties.size() && i < max_len; ++i) {
        if (family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indexes[num] = i;
            ++ num;
        }
    }
    *len = num;
}

void GetPresentQueueFamilyIndexes(const VulkanPhysicalDevice& device, VkSurfaceKHR surface, uint32_t* indexes, uint32_t max_len, uint32_t* len) {
    std::vector<VkQueueFamilyProperties> family_properties = device.GetQueueFamilyProperties();
    int num = 0;
    for (uint32_t i = 0; i < family_properties.size() && i < max_len; ++i) {
        if (device.GetSurfaceSupport(i, surface)) {
            indexes[num] = i;
            ++ num;
        }
    }
    *len = num;
}
