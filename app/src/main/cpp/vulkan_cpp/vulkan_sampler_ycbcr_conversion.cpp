//
// Created by hj6231 on 2024/1/9.
//

#include "vulkan_sampler_ycbcr_conversion.h"


VulkanSamplerYcbcrConversion::VulkanSamplerYcbcrConversion(VkDevice device,
                                                           VkSamplerYcbcrConversion ycbcr_conversion):
                                                           device_(device), ycbcr_conversion_(ycbcr_conversion){

}

VulkanSamplerYcbcrConversion::~VulkanSamplerYcbcrConversion() {
    auto func = (PFN_vkDestroySamplerYcbcrConversion) vkGetDeviceProcAddr(device_,
                                                                         "vkDestroySamplerYcbcrConversion");
    if (func != nullptr) {
        func(device_, ycbcr_conversion_, nullptr);
    }
}

VkSamplerYcbcrConversion VulkanSamplerYcbcrConversion::ycbcr_conversion() {
    return ycbcr_conversion_;
}