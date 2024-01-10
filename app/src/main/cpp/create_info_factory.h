//
// Created by hj6231 on 2023/12/22.
//

#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "vulkan_render_pass.h"

class CreateInfoFactory {
public:
    CreateInfoFactory();
    ~CreateInfoFactory() = default;

    VkDebugReportCallbackCreateInfoEXT GetDebugReportCallbackCreateInfo() const;
    VkInstanceCreateInfo GetInstanceCreateInfo(bool enable_validation_layer) const;
    VkAndroidSurfaceCreateInfoKHR GetAndroidSurfaceCreateInfo(ANativeWindow* window) const;

    std::vector<VkDeviceQueueCreateInfo> GetDeviceQueueCreateInfos(uint32_t graphic_queue_family_index,
                                                                   uint32_t present_queue_family_index) const;
    VkDeviceCreateInfo GetDeviceCreateInfo(bool enable_validation_layer,
                                           const std::vector<VkDeviceQueueCreateInfo>& device_queue_create_infos) const;

    VkSwapchainCreateInfoKHR GetSwapChainCreateInfo(VkSurfaceKHR surface, uint32_t min_image_count,
                                                    VkSurfaceFormatKHR surface_format, VkExtent2D swap_chain_extent,
                                                    std::vector<uint32_t> queue_family_indices, VkPresentModeKHR present_mode,
                                                    VkSwapchainKHR old_swap_chain);
    VkImageViewCreateInfo GetImageViewCreateInfo(VkImage image, VkFormat format);
    static VkAttachmentDescription GetAttachmentDescription(VkFormat format);
    static VkAttachmentReference GetAttachmentReference(uint32_t attachment_index);
    static VkSubpassDescription GetSubpassDescription(const VkAttachmentReference& color_attachment_ref);
    static VkSubpassDependency GetSubpassDependency();
    static VkRenderPassCreateInfo GetRenderPassCreateInfo(const std::vector<VkAttachmentDescription>& attachments,
                                                          const std::vector<VkSubpassDescription>& subpasses,
                                                          const std::vector<VkSubpassDependency>& dependencies);
    static VkFramebufferCreateInfo GetFramebufferCreateInfo(const VulkanRenderPass* render_pass,
                                                            const std::vector<VkImageView>& attachments,
                                                            uint32_t width, uint32_t height);

    static VkCommandPoolCreateInfo GetCommandPoolCreateInfo(uint32_t queue_family_index);
private:
    void SetApplicationInfo();
    void SetDebugReportCallbackCreateInfo();

    VkDebugReportCallbackCreateInfoEXT debug_report_callback_create_info_;
    VkApplicationInfo app_info_;
    VkPhysicalDeviceFeatures features_;

    const char* application_name_ = "Vulkan Tutorial";
    const uint32_t application_version_ = VK_MAKE_VERSION(1, 0, 0);
    const uint32_t vulkan_api_version_ = VK_API_VERSION_1_3;
    const std::vector<const char*> required_instance_layers_ = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char*> required_instance_extensions_ = {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_ANDROID_SURFACE_EXTENSION_NAME};
    const std::vector<const char*> required_instance_extensions_with_validation_ = {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_ANDROID_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_EXTENSION_NAME};
    const float queue_priority_ = 1.0f;
    const std::vector<const char*> required_device_extensions_ = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
};

