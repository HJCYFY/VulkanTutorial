//
// Created by hj6231 on 2023/12/19.
//

#include "vulkan_instance.h"

#include "log.h"

VulkanInstance::VulkanInstance() : vk_instance_(nullptr) {

}

VulkanInstance::~VulkanInstance()  {
    vkDestroyInstance(vk_instance_, nullptr);
    vk_instance_ = nullptr;
}

std::vector<VulkanPhysicalDevice> VulkanInstance::EnumeratePhysicalDevices() const {
    std::vector<VulkanPhysicalDevice> physical_devices;
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(vk_instance_, &device_count, nullptr);
    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(vk_instance_, &device_count, devices.data());
    for (auto device : devices) {
        VulkanPhysicalDevice d(vk_instance_, device);
        physical_devices.push_back(d);
    }
    return physical_devices;
}

std::vector<VkLayerProperties> VulkanInstance::EnumerateInstanceLayerProperties() {
    uint32_t layer_count;
    VkResult ret = vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    if (ret == VK_SUCCESS) {
        std::vector<VkLayerProperties> layer_properties(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, layer_properties.data());
        return layer_properties;
    } else {
        LOG_E("VulkanInstance", "vkEnumerateInstanceLayerProperties Failed, %d\n", ret);
        std::vector<VkLayerProperties> layer_properties;
        return layer_properties;
    }
}

std::vector<VkExtensionProperties> VulkanInstance::EnumerateExtensionProperties() {
    uint32_t extension_count = 0;
    VkResult ret = vkEnumerateInstanceExtensionProperties(nullptr,
                                                          &extension_count,
                                                          nullptr);
    if (ret == VK_SUCCESS) {
        std::vector<VkExtensionProperties> extensions(extension_count);
        vkEnumerateInstanceExtensionProperties(nullptr,
                                               &extension_count,
                                               extensions.data());
        return extensions;
    } else {
        std::vector<VkExtensionProperties> extensions;
        return extensions;
    }
}

VulkanInstance* VulkanInstance::CreateInstance(const VkInstanceCreateInfo* info) {
    auto* instance = new VulkanInstance();
    VkResult ret = vkCreateInstance(info, nullptr, &instance->vk_instance_);
    if (ret == VK_SUCCESS) {
        return instance;
    } else {
        LOG_E("VulkanInstance", "vkCreateInstance failed, ret = %d\n", ret);
        delete instance;
        return nullptr;
    }
}

void VulkanInstance::DestroyInstance(VulkanInstance** instance) {
    if ((*instance) != nullptr) {
        delete (*instance);
        *instance = nullptr;
    }
}

VkResult VulkanInstance::CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* info,
                                      VkDebugUtilsMessengerEXT* messenger) const {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(vk_instance_,
                                                                           "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(vk_instance_, info, nullptr, messenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void VulkanInstance::DestroyDebugUtilsMessengerEXT(VkDebugUtilsMessengerEXT messenger) const {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(vk_instance_,
                                                                            "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(vk_instance_, messenger, nullptr);
    }
}


VkResult VulkanInstance::CreateDebugReportCallbackEXT(const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                                      VkDebugReportCallbackEXT* callback) const {
    auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(vk_instance_,
                                                                           "vkCreateDebugReportCallbackEXT");
    if (func != nullptr) {
        return func(vk_instance_, pCreateInfo, nullptr, callback);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void VulkanInstance::DestroyDebugReportCallbackEXT(VkDebugReportCallbackEXT callback) const {
    auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(vk_instance_,
                                                                            "vkDestroyDebugReportCallbackEXT");
    if (func != nullptr) {
        func(vk_instance_, callback, nullptr);
    }
}

VulkanSurface* VulkanInstance::CreateAndroidSurface(const VkAndroidSurfaceCreateInfoKHR* info) const {
    VkSurfaceKHR surface;
    VkResult ret = vkCreateAndroidSurfaceKHR(vk_instance_, info, nullptr, &surface);
    if (ret == VK_SUCCESS) {
        return new VulkanSurface(vk_instance_, surface);
    }
    return nullptr;
}

void VulkanInstance::DestroyAndroidSurface(VulkanSurface** surface) {
    if (*surface) {
        delete *surface;
        *surface = nullptr;
    }
}