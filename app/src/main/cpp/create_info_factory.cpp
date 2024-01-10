//
// Created by hj6231 on 2023/12/22.
//

#include "create_info_factory.h"

#include "log.h"

VkBool32 VKAPI_PTR debug_report_callback(
        VkDebugReportFlagsEXT                       flags,
        VkDebugReportObjectTypeEXT                  objectType,
        uint64_t                                    object,
        size_t                                      location,
        int32_t                                     messageCode,
        const char*                                 pLayerPrefix,
        const char*                                 pMessage,
        void*                                       pUserData) {
    (void) objectType;
    (void) object;
    (void) location;
    (void) messageCode;
    (void) pUserData;
    switch (flags) {
        case VK_DEBUG_REPORT_WARNING_BIT_EXT :
        case VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT :
            LOG_W("vkDebugReportCallbackEXT", "%s %s\n", pLayerPrefix, pMessage);
            break;
        case VK_DEBUG_REPORT_ERROR_BIT_EXT :
            LOG_E("vkDebugReportCallbackEXT", "%s %s\n", pLayerPrefix, pMessage);
            break;
        case VK_DEBUG_REPORT_INFORMATION_BIT_EXT:
        case VK_DEBUG_REPORT_DEBUG_BIT_EXT :
        default:
            LOG_D("vkDebugReportCallbackEXT", "%s %s\n", pLayerPrefix, pMessage);
    }
    return VK_FALSE; // 应用程序应该始终返回VK_FALSE
}

CreateInfoFactory::CreateInfoFactory() : features_{} {
    SetApplicationInfo();
    SetDebugReportCallbackCreateInfo();
    features_.samplerAnisotropy = VK_TRUE;
}

VkDebugReportCallbackCreateInfoEXT CreateInfoFactory::GetDebugReportCallbackCreateInfo() const {
    return debug_report_callback_create_info_;
}

VkInstanceCreateInfo CreateInfoFactory::GetInstanceCreateInfo(bool enable_validation_layer) const {
    /* typedef struct VkInstanceCreateInfo {
        VkStructureType             sType;
        const void*                 pNext; // VkDebugReportCallbackCreateInfoEXT、VkDebugUtilsMessengerCreateInfoEXT、VkDirectDriverLoadingListLUNARG、VkExportMetalObjectCreateInfoEXT、VkLayerSettingsCreateInfoEXT、VkValidationFeaturesEXT或VkValidationFlagsEXT
        VkInstanceCreateFlags       flags;
        const VkApplicationInfo*    pApplicationInfo;
        uint32_t                    enabledLayerCount;
        const char* const*          ppEnabledLayerNames;
        uint32_t                    enabledExtensionCount;
        const char* const*          ppEnabledExtensionNames;
    } VkInstanceCreateInfo; */
    VkInstanceCreateInfo instance_create_info{};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    if (enable_validation_layer) {
        instance_create_info.pNext =  &debug_report_callback_create_info_;
    }
    instance_create_info.pApplicationInfo = &app_info_;
    if (enable_validation_layer) {
        instance_create_info.enabledLayerCount = static_cast<uint32_t>(required_instance_layers_.size());;
        instance_create_info.ppEnabledLayerNames = required_instance_layers_.data();
        instance_create_info.enabledExtensionCount = static_cast<uint32_t>(required_instance_extensions_with_validation_.size());
        instance_create_info.ppEnabledExtensionNames = required_instance_extensions_with_validation_.data();
    } else {
        instance_create_info.enabledLayerCount = 0;
        instance_create_info.ppEnabledLayerNames = nullptr;
        instance_create_info.enabledExtensionCount = static_cast<uint32_t>(required_instance_extensions_.size());
        instance_create_info.ppEnabledExtensionNames = required_instance_extensions_.data();
    }
    return instance_create_info;
}

VkAndroidSurfaceCreateInfoKHR CreateInfoFactory::GetAndroidSurfaceCreateInfo(ANativeWindow* window) const {
    /*typedef struct VkAndroidSurfaceCreateInfoKHR {
        VkStructureType                   sType;
        const void*                       pNext;
        VkAndroidSurfaceCreateFlagsKHR    flags;
        struct ANativeWindow*             window;
    } VkAndroidSurfaceCreateInfoKHR;*/
    VkAndroidSurfaceCreateInfoKHR info{};
    info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    info.window = window;
    return info;
}

std::vector<VkDeviceQueueCreateInfo> CreateInfoFactory::GetDeviceQueueCreateInfos(uint32_t graphic_queue_family_index,
                                                               uint32_t present_queue_family_index) const {
    /* typedef struct VkDeviceQueueCreateInfo {
        VkStructureType             sType;
        const void*                 pNext;
        VkDeviceQueueCreateFlags    flags;
        uint32_t                    queueFamilyIndex;
        uint32_t                    queueCount;
        const float*                pQueuePriorities;
    } VkDeviceQueueCreateInfo; */
    std::vector<VkDeviceQueueCreateInfo> device_queue_create_infos;
    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = graphic_queue_family_index;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority_;
    device_queue_create_infos.push_back(queue_create_info);
    if (graphic_queue_family_index != present_queue_family_index) {
        queue_create_info.queueFamilyIndex = present_queue_family_index;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority_;
        device_queue_create_infos.push_back(queue_create_info);
    }
    return device_queue_create_infos;
}

VkDeviceCreateInfo CreateInfoFactory::GetDeviceCreateInfo(bool enable_validation_layer, const std::vector<VkDeviceQueueCreateInfo>& device_queue_create_infos) const {
    /* typedef struct VkDeviceCreateInfo {
        VkStructureType                    sType;
        const void*                        pNext;
        VkDeviceCreateFlags                flags;
        uint32_t                           queueCreateInfoCount;
        const VkDeviceQueueCreateInfo*     pQueueCreateInfos;
        uint32_t                           enabledLayerCount;
        const char* const*                 ppEnabledLayerNames;
        uint32_t                           enabledExtensionCount;
        const char* const*                 ppEnabledExtensionNames;
        const VkPhysicalDeviceFeatures*    pEnabledFeatures;
    } VkDeviceCreateInfo; */
    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.queueCreateInfoCount = device_queue_create_infos.size();
    device_create_info.pQueueCreateInfos = device_queue_create_infos.data();
    if (enable_validation_layer) {
        device_create_info.enabledLayerCount = 1;
        device_create_info.ppEnabledLayerNames = required_instance_layers_.data();
    }
    device_create_info.enabledExtensionCount = static_cast<uint32_t>(required_device_extensions_.size());
    device_create_info.ppEnabledExtensionNames = required_device_extensions_.data();
    device_create_info.pEnabledFeatures = &features_;
    return device_create_info;
}

VkSwapchainCreateInfoKHR CreateInfoFactory::GetSwapChainCreateInfo(
        VkSurfaceKHR surface, uint32_t min_image_count,
        VkSurfaceFormatKHR surface_format, VkExtent2D swap_chain_extent,
        std::vector<uint32_t> queue_family_indices,
        VkPresentModeKHR present_mode,
        VkSwapchainKHR old_swap_chain
        ) {
    /*typedef struct VkSwapchainCreateInfoKHR {
        VkStructureType                  sType;
        const void*                      pNext;
        VkSwapchainCreateFlagsKHR        flags;
        VkSurfaceKHR                     surface;
        uint32_t                         minImageCount;
        VkFormat                         imageFormat;
        VkColorSpaceKHR                  imageColorSpace;
        VkExtent2D                       imageExtent;
        uint32_t                         imageArrayLayers;
        VkImageUsageFlags                imageUsage;
        VkSharingMode                    imageSharingMode;
        uint32_t                         queueFamilyIndexCount;
        const uint32_t*                  pQueueFamilyIndices;
        VkSurfaceTransformFlagBitsKHR    preTransform;
        VkCompositeAlphaFlagBitsKHR      compositeAlpha;
        VkPresentModeKHR                 presentMode;
        VkBool32                         clipped;
        VkSwapchainKHR                   oldSwapchain;
    } VkSwapchainCreateInfoKHR; */
    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface;
    create_info.minImageCount = min_image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = swap_chain_extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (queue_family_indices.size() == 1) {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0; // Optional
        create_info.pQueueFamilyIndices = nullptr; // Optional
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = static_cast<uint32_t>(queue_family_indices.size()); // Optional
        create_info.pQueueFamilyIndices = queue_family_indices.data(); // Optional
    }
    create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR ;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = old_swap_chain;
    return create_info;
}

VkImageViewCreateInfo CreateInfoFactory::GetImageViewCreateInfo(
            VkImage image,
            VkFormat format) {
    /*typedef struct VkImageViewCreateInfo {
        VkStructureType            sType;
        const void*                pNext;
        VkImageViewCreateFlags     flags;
        VkImage                    image;
        VkImageViewType            viewType;
        VkFormat                   format;
        VkComponentMapping         components;
        VkImageSubresourceRange    subresourceRange;
    } VkImageViewCreateInfo;*/
    VkImageViewCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = image;
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = format;
    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    /*typedef struct VkImageSubresourceRange {
        VkImageAspectFlags    aspectMask;
        uint32_t              baseMipLevel;
        uint32_t              levelCount;
        uint32_t              baseArrayLayer;
        uint32_t              layerCount;
    } VkImageSubresourceRange;*/
    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = 1;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;
    return create_info;
}

VkFramebufferCreateInfo CreateInfoFactory::GetFramebufferCreateInfo(const VulkanRenderPass* render_pass,
                                                                    const std::vector<VkImageView>& attachments,
                                                                    uint32_t width, uint32_t height) {
    /* typedef struct VkFramebufferCreateInfo {
        VkStructureType             sType;
        const void*                 pNext;
        VkFramebufferCreateFlags    flags;
        VkRenderPass                renderPass;
        uint32_t                    attachmentCount;
        const VkImageView*          pAttachments;
        uint32_t                    width;
        uint32_t                    height;
        uint32_t                    layers;
    } VkFramebufferCreateInfo; */
    VkFramebufferCreateInfo framebuffer_info{};
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = render_pass->render_pass();
    framebuffer_info.attachmentCount = static_cast<uint32_t>(attachments.size()) ;
    framebuffer_info.pAttachments = attachments.data();
    framebuffer_info.width = width;
    framebuffer_info.height = height;
    framebuffer_info.layers = 1;
    return framebuffer_info;
}

VkCommandPoolCreateInfo CreateInfoFactory::GetCommandPoolCreateInfo(uint32_t queue_family_index) {
    /* typedef struct VkCommandPoolCreateInfo {
        VkStructureType             sType;
        const void*                 pNext;
        VkCommandPoolCreateFlags    flags;
        uint32_t                    queueFamilyIndex;
    } VkCommandPoolCreateInfo; */
    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = queue_family_index;
    return pool_info;
}

VkAttachmentDescription CreateInfoFactory::GetAttachmentDescription(VkFormat format) {
    /*typedef struct VkAttachmentDescription {
        VkAttachmentDescriptionFlags    flags;
        VkFormat                        format;
        VkSampleCountFlagBits           samples;
        VkAttachmentLoadOp              loadOp;
        VkAttachmentStoreOp             storeOp;
        VkAttachmentLoadOp              stencilLoadOp;
        VkAttachmentStoreOp             stencilStoreOp;
        VkImageLayout                   initialLayout;
        VkImageLayout                   finalLayout;
    } VkAttachmentDescription;*/
    VkAttachmentDescription color_attachment{};
    color_attachment.format = format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    return color_attachment;
}

VkAttachmentReference CreateInfoFactory::GetAttachmentReference(uint32_t attachment_index) {
    /*typedef struct VkAttachmentReference {
        uint32_t         attachment;
        VkImageLayout    layout;
    } VkAttachmentReference;*/
    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = attachment_index;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    return color_attachment_ref;
}

VkSubpassDescription CreateInfoFactory::GetSubpassDescription(const VkAttachmentReference& color_attachment_ref) {
    /*typedef struct VkSubpassDescription {
        VkSubpassDescriptionFlags       flags;
        VkPipelineBindPoint             pipelineBindPoint;
        uint32_t                        inputAttachmentCount;
        const VkAttachmentReference*    pInputAttachments;
        uint32_t                        colorAttachmentCount;
        const VkAttachmentReference*    pColorAttachments;
        const VkAttachmentReference*    pResolveAttachments;
        const VkAttachmentReference*    pDepthStencilAttachment;
        uint32_t                        preserveAttachmentCount;
        const uint32_t*                 pPreserveAttachments;
    } VkSubpassDescription;*/
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    return subpass;
}

VkSubpassDependency CreateInfoFactory::GetSubpassDependency() {
    /* typedef struct VkSubpassDependency {
        uint32_t                srcSubpass;
        uint32_t                dstSubpass;
        VkPipelineStageFlags    srcStageMask;
        VkPipelineStageFlags    dstStageMask;
        VkAccessFlags           srcAccessMask;
        VkAccessFlags           dstAccessMask;
        VkDependencyFlags       dependencyFlags;
    } VkSubpassDependency; */
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // 特殊值VK_SUBPASS_EXTERNAL是指渲染通道之前或之后的隐式子通道
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    return dependency;
}

VkRenderPassCreateInfo CreateInfoFactory::GetRenderPassCreateInfo(const std::vector<VkAttachmentDescription>& attachments,
                                                      const std::vector<VkSubpassDescription>& subpasses,
                                                      const std::vector<VkSubpassDependency>& dependencies) {

    /* typedef struct VkRenderPassCreateInfo {
        VkStructureType                   sType;
        const void*                       pNext;
        VkRenderPassCreateFlags           flags;
        uint32_t                          attachmentCount;
        const VkAttachmentDescription*    pAttachments;
        uint32_t                          subpassCount;
        const VkSubpassDescription*       pSubpasses;
        uint32_t                          dependencyCount;
        const VkSubpassDependency*        pDependencies;
    } VkRenderPassCreateInfo; */
    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
    render_pass_info.pAttachments = attachments.data();
    render_pass_info.subpassCount = static_cast<uint32_t>(subpasses.size());
    render_pass_info.pSubpasses = subpasses.data();
    render_pass_info.dependencyCount = static_cast<uint32_t>(dependencies.size());
    render_pass_info.pDependencies = dependencies.data();
    return render_pass_info;
}

void CreateInfoFactory::SetApplicationInfo() {
    /* typedef struct VkApplicationInfo {
        VkStructureType    sType;
        const void*        pNext;
        const char*        pApplicationName;
        uint32_t           applicationVersion;
        const char*        pEngineName;
        uint32_t           engineVersion;
        uint32_t           apiVersion;
    } VkApplicationInfo; */
    app_info_.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info_.pNext = nullptr;
    app_info_.pApplicationName = application_name_;
    app_info_.applicationVersion = application_version_;
    app_info_.pEngineName = "No Engine";
    app_info_.engineVersion = VK_MAKE_VERSION(1, 3, 0);
    app_info_.apiVersion = vulkan_api_version_;
}

void CreateInfoFactory::SetDebugReportCallbackCreateInfo() {
    /*typedef struct VkDebugReportCallbackCreateInfoEXT {
        VkStructureType                 sType;
        const void*                     pNext;
        VkDebugReportFlagsEXT           flags;
        PFN_vkDebugReportCallbackEXT    pfnCallback;
        void*                           pUserData;
    } VkDebugReportCallbackCreateInfoEXT;*/
    debug_report_callback_create_info_.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    debug_report_callback_create_info_.pNext = nullptr;
    debug_report_callback_create_info_.flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT|
                                               VK_DEBUG_REPORT_WARNING_BIT_EXT|
                                               VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT|
                                               VK_DEBUG_REPORT_ERROR_BIT_EXT|
                                               VK_DEBUG_REPORT_DEBUG_BIT_EXT;
    debug_report_callback_create_info_.pfnCallback = debug_report_callback;
    debug_report_callback_create_info_.pUserData = nullptr;
}