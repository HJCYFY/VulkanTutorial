//
// Created by hj6231 on 2023/12/20.
//

#include "vulkan_physical_device.h"
#include "log.h"

VulkanPhysicalDevice::VulkanPhysicalDevice(VkInstance instance, VkPhysicalDevice device) :
        instance_(instance), device_(device) {
    LOG_D("VulkanPhysicalDevice1", "device_ %p\n", device_);
}

VulkanPhysicalDevice::VulkanPhysicalDevice(const VulkanPhysicalDevice& device) {
    LOG_D("VulkanPhysicalDevice2", "device_ %p\n", device_);
    instance_ = device.instance_;
    device_ = device.device_;
}

void VulkanPhysicalDevice::GetProperties(VkPhysicalDeviceProperties* device_properties) const {
    vkGetPhysicalDeviceProperties(device_, device_properties);
}

void VulkanPhysicalDevice::GetFeatures(VkPhysicalDeviceFeatures * device_features) const {
    vkGetPhysicalDeviceFeatures(device_, device_features);
}

void VulkanPhysicalDevice::GetFeatures2(VkPhysicalDeviceFeatures2 * device_features) const {
    auto func = (PFN_vkGetPhysicalDeviceFeatures2) vkGetInstanceProcAddr(instance_,
                                                                           "vkGetPhysicalDeviceFeatures2");
    if (func != nullptr) {
        func(device_, device_features);
        vkGetPhysicalDeviceFeatures(device_, &device_features->features);
    } else {
        LOG_W("", "No vkGetPhysicalDeviceFeatures2 Fun\n");
    }
}

void VulkanPhysicalDevice::GetFormatProperties(VkFormat format, VkFormatProperties* props) const {
    vkGetPhysicalDeviceFormatProperties(device_, format, props);
}

std::vector<VkExtensionProperties> VulkanPhysicalDevice::EnumerateExtensionProperties() const {
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(device_, nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> extensions_properties(extension_count);
    vkEnumerateDeviceExtensionProperties(device_, nullptr, &extension_count, extensions_properties.data());
    return extensions_properties;
}

std::vector<VkQueueFamilyProperties> VulkanPhysicalDevice::GetQueueFamilyProperties() const{
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device_,
                                             &queue_family_count,
                                             nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device_,
                                             &queue_family_count,
                                             queue_families.data());
    return queue_families;
}

bool VulkanPhysicalDevice::GetSurfaceSupport(uint32_t queue_family_index, VkSurfaceKHR surface) const {
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device_, queue_family_index, surface, &presentSupport);
    return presentSupport;
}

VkResult VulkanPhysicalDevice::GetSurfaceCapabilities(VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* capabilities) const{
    return vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device_, surface, capabilities);
}

std::vector<VkSurfaceFormatKHR> VulkanPhysicalDevice::GetSurfaceFormats(VkSurfaceKHR surface) const {
    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device_, surface, &format_count, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device_, surface, &format_count, formats.data());
    return formats;
}

std::vector<VkPresentModeKHR> VulkanPhysicalDevice::GetSurfacePresentModes(VkSurfaceKHR surface) const {
    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device_, surface, &present_mode_count, nullptr);
    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device_, surface, &present_mode_count, present_modes.data());
    return present_modes;
}

VulkanLogicDevice* VulkanPhysicalDevice::CreateDevice(const VkDeviceCreateInfo* info) const {
    VkDevice device;
    VkResult ret = vkCreateDevice(device_, info, nullptr, &device);
    if (ret == VK_SUCCESS) {
        auto logic_device = new VulkanLogicDevice(device_, device);
        return logic_device;
    }
    return nullptr;
}

void VulkanPhysicalDevice::DestroyDevice(VulkanLogicDevice** device) {
    delete (*device);
    *device = nullptr;
}

