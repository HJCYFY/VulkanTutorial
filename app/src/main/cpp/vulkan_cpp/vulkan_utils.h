//
// Created by hj6231 on 2023/12/22.
//

#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "vulkan_physical_device.h"

bool CheckValidationLayerSupport(const std::vector<VkLayerProperties>& layer_properties);

bool CheckPhysicalDeviceGraphicSupport(const VulkanPhysicalDevice& device);

bool CheckPhysicalDeviceSwapChainSupport(const VulkanPhysicalDevice& device);

bool CheckPhysicalDeviceSamplerAnisotropySupport(const VulkanPhysicalDevice& device);

void GetGraphicQueueFamilyIndexes(const VulkanPhysicalDevice& device, uint32_t* indexes, uint32_t max_len, uint32_t* len);

void GetPresentQueueFamilyIndexes(const VulkanPhysicalDevice& device, VkSurfaceKHR surface, uint32_t* indexes, uint32_t max_len, uint32_t* len);