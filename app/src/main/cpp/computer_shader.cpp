//
// Created by hj6231 on 2024/1/23.
//

#include "computer_shader.h"

#include <cassert>
#include <unistd.h>
#include "log.h"

static VkBool32 VKAPI_PTR debug_report_callback(
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

ComputerShader::ComputerShader(AAssetManager * asset_manager) :
        TutorialBase(asset_manager),
        instance_(nullptr),
        callback_{},
        physical_device_(nullptr),
        surface_(nullptr),
        queue_family_index_(0),
        logic_device_(nullptr),
        queue_(nullptr),
        swap_chain_(nullptr),
        surface_format_{},
        swap_chain_extent_{},
        graphic_command_pool_(nullptr),
        graphic_command_buffer_(nullptr),
        particle_(nullptr) {

}

void ComputerShader::Run() {
    CreateSurface(window_);
    CreateLogicalDevice();
    CreateCommandPool();
    CreateCommandBuffer();
    CreateSyncObjects();

    CreateSwapChain(window_);

    CreateComputerPipeline();
    CreateGraphicPipeline();

    CreateImageViews();
    CreateFrameBuffers(particle_graphic_->render_pass()->render_pass());


    int thread_state;
    std::unique_lock<std::mutex> lock(thread_state_mutex_);
    thread_state = thread_state_;
    lock.unlock();
    uint32_t image_index;
    uint32_t flight_index = 0;
    while (thread_state == 1) {
        VkSemaphore frame_available_semaphore = swap_chain_image_available_semaphore_[flight_index]->semaphore();
        VkSemaphore render_finished_semaphore = render_finished_semaphore_[flight_index]->semaphore();
        VkFence cpu_wait_fence = cpu_wait_fence_[flight_index]->fence();
        logic_device_->AcquireNextImageKHR(swap_chain_->swap_chain(),
                                           UINT64_MAX,
                                           frame_available_semaphore,
                                           VK_NULL_HANDLE,
                                           &image_index);
        logic_device_->WaitForFences( 1, &cpu_wait_fence, VK_TRUE, UINT64_MAX);


        graphic_command_buffer_->ResetCommandBuffer(0);

        VkPipelineStageFlags waitDstStage[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkCommandBuffer commandBuffers[] = { graphic_command_buffer_->command_buffer() };
        VkSubmitInfo submitInfo{
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .pNext = nullptr,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = &frame_available_semaphore,
                .pWaitDstStageMask = waitDstStage,
                .commandBufferCount = 1,
                .pCommandBuffers = commandBuffers,
                .signalSemaphoreCount = 1,
                .pSignalSemaphores = &render_finished_semaphore
        };
        LOG_D("HJ", "RUN 1 time");
        logic_device_->ResetFences(1, &cpu_wait_fence);
        particle_->Draw(graphic_command_buffer_);
        particle_graphic_->Draw(graphic_command_buffer_, frame_buffers_[image_index]);
        VkResult ret = queue_->QueueSubmit(1, &submitInfo, cpu_wait_fence);
        assert(ret == VK_SUCCESS);

        VkSwapchainKHR swapchainKhrs[] = { swap_chain_->swap_chain() };
        VkPresentInfoKHR presentInfoKhr{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &render_finished_semaphore,
            .swapchainCount = 1,
            .pSwapchains = swapchainKhrs,
            .pImageIndices = &image_index,
            .pResults = nullptr
        };
        queue_->QueuePresentKHR(&presentInfoKhr);

        LOG_D("HJ", "RUN 1 time end");
        lock.lock();
        thread_state = thread_state_;
        lock.unlock();
        flight_index = (flight_index + 1) % FRAME_IN_FLIGHT;
    }

    logic_device_->DeviceWaitIdle();

    DestroyGraphicPipeline();
    DestroyComputerPipeline();

    DestroyFrameBuffers();
    DestroyImageViews();
    DestroySwapChain();

    DestroySyncObjects();
    DestroyCommandBuffer();
    DestroyCommandPool();

    DestroyLogicalDevice();
    DestroySurface();
}

void ComputerShader::CreateInstance() {
    std::vector<VkLayerProperties> layer_properties =
            VulkanInstance::EnumerateInstanceLayerProperties();

    LOG_D("HJ", "instance layer properties\n");
    for (auto it : layer_properties) {
        LOG_D("HJ", "\t %s\n", it.layerName);
    }

    std::vector<VkExtensionProperties> extensionProperties = VulkanInstance::EnumerateExtensionProperties();
    LOG_D("HJ", "instance extension properties\n");
    for (auto it : extensionProperties) {
        LOG_D("HJ", "\t %s\n", it.extensionName);
    }

    VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfoExt {
        .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
        .pNext = nullptr,
        .flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT|
                 VK_DEBUG_REPORT_WARNING_BIT_EXT|
                 VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT|
                 VK_DEBUG_REPORT_ERROR_BIT_EXT|
                 VK_DEBUG_REPORT_DEBUG_BIT_EXT,
        .pfnCallback = debug_report_callback,
        .pUserData = nullptr
    };

    VkApplicationInfo applicationInfo {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName =  "Vulkan Tutorial",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = nullptr,
            .engineVersion = VK_MAKE_VERSION(1, 3, 0),
            .apiVersion = VK_API_VERSION_1_3
    };

    const std::vector<const char*> required_instance_layers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char*> required_instance_extensions =
            {VK_KHR_SURFACE_EXTENSION_NAME,
             VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
             VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
             VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME, // optional
             VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME, // optional
             VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, // optional
             };
    VkInstanceCreateInfo instanceCreateInfo {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = &debugReportCallbackCreateInfoExt,
        .flags = 0,
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = static_cast<uint32_t>(required_instance_layers.size()),
        .ppEnabledLayerNames = required_instance_layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(required_instance_extensions.size()),
        .ppEnabledExtensionNames = required_instance_extensions.data()
    };
    instance_ = VulkanInstance::CreateInstance(&instanceCreateInfo);
    assert(instance_);
    instance_->CreateDebugReportCallbackEXT(&debugReportCallbackCreateInfoExt, &callback_);
}

void ComputerShader::DestroyInstance() {
    if (instance_) {
        instance_->DestroyDebugReportCallbackEXT(callback_);
        VulkanInstance::DestroyInstance(&instance_);
    }
}

bool ComputerShader::PickPhysicalDevice() {
    std::vector<VulkanPhysicalDevice> physical_devices = instance_->EnumeratePhysicalDevices();
    for (auto& physical_device : physical_devices) {
        QueryPhysicalDeviceInfo(physical_device);
    }
    if (!physical_devices.empty()) {
        physical_device_ = new VulkanPhysicalDevice(physical_devices[0]);
        return true;
    }
    return false;
}

void ComputerShader::CreateSurface(ANativeWindow* window) {
    VkAndroidSurfaceCreateInfoKHR info{
        .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .window = window
    };
    surface_ = instance_->CreateAndroidSurface(&info);
    assert(surface_);
}

void ComputerShader::DestroySurface() {
    VulkanInstance::DestroyAndroidSurface(&surface_);
}

void ComputerShader::CreateLogicalDevice() {
    std::vector<VkQueueFamilyProperties> family_properties = physical_device_->GetQueueFamilyProperties();
    for (uint32_t i = 0; i < family_properties.size(); ++i) {
        if (((family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) &&
                ((family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0)) {
            queue_family_index_ = i;
            break;
        }
    }
    assert(physical_device_->GetSurfaceSupport(queue_family_index_,
                                               surface_->surface()));

    float queuePriorities = 1.0;

    VkDeviceQueueCreateInfo deviceQueueCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueFamilyIndex = queue_family_index_,
        .queueCount = 1,
        .pQueuePriorities = &queuePriorities
    };

    const char* VK_LAYER_KHRONOS_validation = "VK_LAYER_KHRONOS_validation";
    const char* SWAPCHAIN_EXTENSION = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

    VkPhysicalDeviceFeatures features {
        .samplerAnisotropy = VK_TRUE
    };

    VkDeviceCreateInfo deviceCreateInfo {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &deviceQueueCreateInfo,
        .enabledLayerCount = 1,
        .ppEnabledLayerNames = &VK_LAYER_KHRONOS_validation,
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = &SWAPCHAIN_EXTENSION,
        .pEnabledFeatures = &features
    };

    logic_device_ = physical_device_->CreateDevice(&deviceCreateInfo);
    assert(logic_device_);
    queue_ = logic_device_->GetDeviceQueue(queue_family_index_, 0);
    assert(queue_);
}

void ComputerShader::DestroyLogicalDevice() {
    VulkanPhysicalDevice::DestroyDevice(&logic_device_);
}

void ComputerShader::CreateCommandPool() {
    VkCommandPoolCreateInfo commandPoolCreateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = queue_family_index_
    };
    graphic_command_pool_ = logic_device_->CreateCommandPool(&commandPoolCreateInfo);
    assert(graphic_command_pool_);
}

void ComputerShader::DestroyCommandPool() {
    VulkanLogicDevice::DestroyCommandPool(&graphic_command_pool_);
}

void ComputerShader::CreateCommandBuffer() {
    // 这里也应该分配 FRAME_IN_FLIGHT 个 CommandBuffer
    graphic_command_buffer_ = graphic_command_pool_->AllocateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
}

void ComputerShader::DestroyCommandBuffer() {
    VulkanCommandPool::FreeCommandBuffer(&graphic_command_buffer_);
}

void ComputerShader::CreateSyncObjects() {
    cpu_wait_fence_.resize(FRAME_IN_FLIGHT);
    swap_chain_image_available_semaphore_.resize(FRAME_IN_FLIGHT);
    render_finished_semaphore_.resize(FRAME_IN_FLIGHT);
    for (int i=0; i<FRAME_IN_FLIGHT; ++i) {
        VkFenceCreateInfo fenceCreateInfo {
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .pNext = nullptr,
                .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };
        cpu_wait_fence_[i] = logic_device_->CreateFence(&fenceCreateInfo);

        VkSemaphoreCreateInfo semaphoreCreateInfo{
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0 // flags is reserved for future use.
        };
        swap_chain_image_available_semaphore_[i] = logic_device_->CreateSemaphore(&semaphoreCreateInfo);
        render_finished_semaphore_[i] = logic_device_->CreateSemaphore(&semaphoreCreateInfo);
    }
}

void ComputerShader::DestroySyncObjects() {
    for (int i=0; i<FRAME_IN_FLIGHT; ++i) {
        VulkanLogicDevice::DestroySemaphore(&render_finished_semaphore_[i]);
        VulkanLogicDevice::DestroySemaphore(&swap_chain_image_available_semaphore_[i]);
        VulkanLogicDevice::DestroyFence(&cpu_wait_fence_[i]);
    }
    render_finished_semaphore_.clear();
    swap_chain_image_available_semaphore_.clear();
    cpu_wait_fence_.clear();
}

void ComputerShader::CreateComputerPipeline() {
    particle_ = new Particle(logic_device_, VK_FORMAT_R8G8B8A8_SRGB, swap_chain_extent_);
    particle_->CreatePipeline();
}

void ComputerShader::DestroyComputerPipeline() {
    particle_->DestroyPipeline();
    delete particle_;
    particle_ = nullptr;
}

void ComputerShader::CreateGraphicPipeline() {
    particle_graphic_ = new ParticleGraphic(logic_device_, VK_FORMAT_R8G8B8A8_SRGB, swap_chain_extent_, particle_);
    particle_graphic_->CreatePipeline();
}

void ComputerShader::DestroyGraphicPipeline() {
    particle_graphic_->DestroyPipeline();
    delete particle_graphic_;
    particle_graphic_ = nullptr;
}

void ComputerShader::CreateSwapChain(ANativeWindow* window) {
    VkSurfaceCapabilitiesKHR capabilities{};
    physical_device_->GetSurfaceCapabilities(surface_->surface(), &capabilities);

    LOG_D("HJ", "Surface Capabilities\n");
    LOG_D("HJ", "\t minImageCount %u\n", capabilities.minImageCount);
    LOG_D("HJ", "\t maxImageCount %u\n", capabilities.maxImageCount);
    LOG_D("HJ", "\t currentExtent width %u height %u\n", capabilities.currentExtent.width, capabilities.currentExtent.height);
    LOG_D("HJ", "\t minImageExtent width %u height %u\n", capabilities.minImageExtent.width, capabilities.minImageExtent.height);
    LOG_D("HJ", "\t maxImageExtent width %u height %u\n", capabilities.maxImageExtent.width, capabilities.maxImageExtent.height);
    LOG_D("HJ", "\t maxImageArrayLayers %u\n", capabilities.maxImageArrayLayers);
    LOG_D("HJ", "\t supportedTransforms 0x%x\n", capabilities.supportedTransforms);
    LOG_D("HJ", "\t currentTransform 0x%x\n", capabilities.currentTransform);
    LOG_D("HJ", "\t supportedCompositeAlpha 0x%x\n", capabilities.supportedCompositeAlpha);
    LOG_D("HJ", "\t supportedUsageFlags 0x%x\n", capabilities.supportedUsageFlags);

    uint32_t image_count = capabilities.minImageCount + 1;

    std::vector<VkSurfaceFormatKHR> formats =
            physical_device_->GetSurfaceFormats(surface_->surface());
    LOG_D("HJ", "surface format\n");
    surface_format_ = formats[0];
    for (const auto& format : formats) {
        LOG_D("HJ", "\t format %d color space %d\n", format.format, format.colorSpace);
        if ((format.format == VK_FORMAT_R8G8B8A8_SRGB) && (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)) {
            surface_format_ = format;
        }
    }
    LOG_D("HJ", "surface format %d\n", surface_format_.format);

    std::vector<VkPresentModeKHR> present_modes =
            physical_device_->GetSurfacePresentModes(surface_->surface());
    VkPresentModeKHR present_mode = present_modes[0];
    LOG_D("HJ", "present modes\n");
    for (const auto& mode : present_modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            present_mode = mode;
        }
        LOG_D("HJ", "\t present mode %d\n", mode);
    }
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        swap_chain_extent_ = capabilities.currentExtent;
    } else {
        swap_chain_extent_.width = ANativeWindow_getWidth(window);
        swap_chain_extent_.height = ANativeWindow_getHeight(window);
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfoKhr{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = surface_->surface(),
        .minImageCount = image_count,
        .imageFormat = surface_format_.format,
        .imageColorSpace = surface_format_.colorSpace,
        .imageExtent = swap_chain_extent_,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &queue_family_index_,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
        .presentMode = present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
    };
    swap_chain_ = logic_device_->CreateSwapChain(&swapchainCreateInfoKhr);
    assert(swap_chain_);
}

void ComputerShader::DestroySwapChain() {
    VulkanLogicDevice::DestroySwapChain(&swap_chain_);
}

void ComputerShader::CreateImageViews() {
    std::vector<VkImage> swap_chain_images = swap_chain_->GetImages();
    for (const auto& image : swap_chain_images) {
        VkImageViewCreateInfo imageViewCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = surface_format_.format,
            .components = {.r = VK_COMPONENT_SWIZZLE_IDENTITY, .g = VK_COMPONENT_SWIZZLE_IDENTITY, .b = VK_COMPONENT_SWIZZLE_IDENTITY, .a = VK_COMPONENT_SWIZZLE_IDENTITY},
            .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1}
        };
        LOG_D("HJ", "fMT %d\n", imageViewCreateInfo.format);
        VulkanImageView* image_view = logic_device_->CreateImageView(&imageViewCreateInfo);
        assert(image_view);
        swap_chain_image_views_.push_back(image_view);
    }
}

void ComputerShader::DestroyImageViews() {
    for (auto& imageview : swap_chain_image_views_) {
        VulkanLogicDevice::DestroyImageView(&imageview);
    }
    swap_chain_image_views_.clear();
}

void ComputerShader::CreateFrameBuffers(VkRenderPass renderPass) {
    frame_buffers_.resize(swap_chain_image_views_.size());
    for (size_t i = 0; i < swap_chain_image_views_.size(); i++) {
        std::vector<VkImageView> attachments;
        attachments.push_back(swap_chain_image_views_[i]->image_view());
        VkFramebufferCreateInfo framebufferCreateInfo {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .renderPass = renderPass,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .width = swap_chain_extent_.width,
            .height = swap_chain_extent_.height,
            .layers = 1
        };
        VulkanFrameBuffer* frame_buffer = logic_device_->CreateFrameBuffer(&framebufferCreateInfo);
        assert(frame_buffer);
        frame_buffers_[i] = frame_buffer;
    }
}

void ComputerShader::DestroyFrameBuffers() {
    for (auto& it : frame_buffers_) {
        VulkanLogicDevice::DestroyFrameBuffer(&it);
    }
    frame_buffers_.clear();
}

void ComputerShader::QueryPhysicalDeviceInfo(const VulkanPhysicalDevice& device) {
    VkPhysicalDeviceProperties deviceProperties{};
    device.GetProperties(&deviceProperties);
    LOG_D("HJ", "physical device properties\n");
    LOG_D("HJ", "\t apiVersion %u.%u.%u\n", VK_API_VERSION_MAJOR(deviceProperties.apiVersion), VK_API_VERSION_MINOR(deviceProperties.apiVersion), VK_API_VERSION_PATCH(deviceProperties.apiVersion));
    LOG_D("HJ", "\t driverVersion %u\n", deviceProperties.driverVersion);
    LOG_D("HJ", "\t vendorID %u\n", deviceProperties.vendorID);
    LOG_D("HJ", "\t deviceID %u\n", deviceProperties.deviceID);
    LOG_D("HJ", "\t deviceType %d\n", deviceProperties.deviceType);
    LOG_D("HJ", "\t deviceName %s\n", deviceProperties.deviceName);

    VkPhysicalDeviceVulkan11Features physicalDeviceVulkan11Features {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
        .pNext = nullptr,
    };

    VkPhysicalDeviceVulkan12Features physicalDeviceVulkan12Features {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = &physicalDeviceVulkan11Features,
    };

    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &physicalDeviceVulkan12Features,
        .features = {}
    };
    device.GetFeatures2(&physicalDeviceFeatures2);
    LOG_D("HJ", "physical device features\n");
    LOG_D("HJ", "\trobustBufferAccess %d", physicalDeviceFeatures2.features.robustBufferAccess);
    LOG_D("HJ", "\tfullDrawIndexUint32 %d",physicalDeviceFeatures2.features.fullDrawIndexUint32); 
    LOG_D("HJ", "\timageCubeArray %d", physicalDeviceFeatures2.features.imageCubeArray);
    LOG_D("HJ", "\tindependentBlend %d", physicalDeviceFeatures2.features.independentBlend);
    LOG_D("HJ", "\tgeometryShader %d", physicalDeviceFeatures2.features.geometryShader);
    LOG_D("HJ", "\ttessellationShader %d", physicalDeviceFeatures2.features.tessellationShader);
    LOG_D("HJ", "\tsampleRateShading %d", physicalDeviceFeatures2.features.sampleRateShading);
    LOG_D("HJ", "\tdualSrcBlend %d", physicalDeviceFeatures2.features.dualSrcBlend);
    LOG_D("HJ", "\tlogicOp %d", physicalDeviceFeatures2.features.logicOp);
    LOG_D("HJ", "\tmultiDrawIndirect %d", physicalDeviceFeatures2.features.multiDrawIndirect);
    LOG_D("HJ", "\tdrawIndirectFirstInstance %d", physicalDeviceFeatures2.features.drawIndirectFirstInstance);
    LOG_D("HJ", "\tdepthClamp %d", physicalDeviceFeatures2.features.depthClamp);
    LOG_D("HJ", "\tdepthBiasClamp %d", physicalDeviceFeatures2.features.depthBiasClamp);
    LOG_D("HJ", "\tfillModeNonSolid %d", physicalDeviceFeatures2.features.fillModeNonSolid);
    LOG_D("HJ", "\tdepthBounds %d", physicalDeviceFeatures2.features.depthBounds);
    LOG_D("HJ", "\twideLines %d", physicalDeviceFeatures2.features.wideLines);
    LOG_D("HJ", "\tlargePoints %d", physicalDeviceFeatures2.features.largePoints);
    LOG_D("HJ", "\talphaToOne %d", physicalDeviceFeatures2.features.alphaToOne);
    LOG_D("HJ", "\tmultiViewport %d", physicalDeviceFeatures2.features.multiViewport);
    LOG_D("HJ", "\tsamplerAnisotropy %d", physicalDeviceFeatures2.features.samplerAnisotropy);
    LOG_D("HJ", "\ttextureCompressionETC2 %d", physicalDeviceFeatures2.features.textureCompressionETC2);
    LOG_D("HJ", "\ttextureCompressionASTC_LDR %d", physicalDeviceFeatures2.features.textureCompressionASTC_LDR);
    LOG_D("HJ", "\ttextureCompressionBC %d", physicalDeviceFeatures2.features.textureCompressionBC);
    LOG_D("HJ", "\tocclusionQueryPrecise %d", physicalDeviceFeatures2.features.occlusionQueryPrecise);
    LOG_D("HJ", "\tpipelineStatisticsQuery %d", physicalDeviceFeatures2.features.pipelineStatisticsQuery);
    LOG_D("HJ", "\tvertexPipelineStoresAndAtomics %d", physicalDeviceFeatures2.features.vertexPipelineStoresAndAtomics);
    LOG_D("HJ", "\tfragmentStoresAndAtomics %d", physicalDeviceFeatures2.features.fragmentStoresAndAtomics);
    LOG_D("HJ", "\tshaderTessellationAndGeometryPointSize %d", physicalDeviceFeatures2.features.shaderTessellationAndGeometryPointSize);
    LOG_D("HJ", "\tshaderImageGatherExtended %d", physicalDeviceFeatures2.features.shaderImageGatherExtended);
    LOG_D("HJ", "\tshaderStorageImageExtendedFormats %d", physicalDeviceFeatures2.features.shaderStorageImageExtendedFormats);
    LOG_D("HJ", "\tshaderStorageImageMultisample %d", physicalDeviceFeatures2.features.shaderStorageImageMultisample);
    LOG_D("HJ", "\tshaderStorageImageReadWithoutFormat %d", physicalDeviceFeatures2.features.shaderStorageImageReadWithoutFormat);
    LOG_D("HJ", "\tshaderStorageImageWriteWithoutFormat %d", physicalDeviceFeatures2.features.shaderStorageImageWriteWithoutFormat);
    LOG_D("HJ", "\tshaderUniformBufferArrayDynamicIndexing %d", physicalDeviceFeatures2.features.shaderUniformBufferArrayDynamicIndexing);
    LOG_D("HJ", "\tshaderSampledImageArrayDynamicIndexing %d", physicalDeviceFeatures2.features.shaderSampledImageArrayDynamicIndexing);
    LOG_D("HJ", "\tshaderStorageBufferArrayDynamicIndexing %d", physicalDeviceFeatures2.features.shaderStorageBufferArrayDynamicIndexing);
    LOG_D("HJ", "\tshaderStorageImageArrayDynamicIndexing %d", physicalDeviceFeatures2.features.shaderStorageImageArrayDynamicIndexing);
    LOG_D("HJ", "\tshaderClipDistance %d", physicalDeviceFeatures2.features.shaderClipDistance);
    LOG_D("HJ", "\tshaderCullDistance %d", physicalDeviceFeatures2.features.shaderCullDistance);
    LOG_D("HJ", "\tshaderFloat64 %d", physicalDeviceFeatures2.features.shaderFloat64);
    LOG_D("HJ", "\tshaderInt64 %d", physicalDeviceFeatures2.features.shaderInt64);
    LOG_D("HJ", "\tshaderInt16 %d", physicalDeviceFeatures2.features.shaderInt16);
    LOG_D("HJ", "\tshaderResourceResidency %d", physicalDeviceFeatures2.features.shaderResourceResidency);
    LOG_D("HJ", "\tshaderResourceMinLod %d", physicalDeviceFeatures2.features.shaderResourceMinLod);
    LOG_D("HJ", "\tsparseBinding %d", physicalDeviceFeatures2.features.sparseBinding);
    LOG_D("HJ", "\tsparseResidencyBuffer %d", physicalDeviceFeatures2.features.sparseResidencyBuffer);
    LOG_D("HJ", "\tsparseResidencyImage2D %d", physicalDeviceFeatures2.features.sparseResidencyImage2D);
    LOG_D("HJ", "\tsparseResidencyImage3D %d", physicalDeviceFeatures2.features.sparseResidencyImage3D);
    LOG_D("HJ", "\tsparseResidency2Samples %d", physicalDeviceFeatures2.features.sparseResidency2Samples);
    LOG_D("HJ", "\tsparseResidency4Samples %d", physicalDeviceFeatures2.features.sparseResidency4Samples);
    LOG_D("HJ", "\tsparseResidency8Samples %d", physicalDeviceFeatures2.features.sparseResidency8Samples);
    LOG_D("HJ", "\tsparseResidency16Samples %d", physicalDeviceFeatures2.features.sparseResidency16Samples);
    LOG_D("HJ", "\tsparseResidencyAliased %d", physicalDeviceFeatures2.features.sparseResidencyAliased);
    LOG_D("HJ", "\tvariableMultisampleRate %d", physicalDeviceFeatures2.features.variableMultisampleRate);
    LOG_D("HJ", "\tinheritedQueries %d", physicalDeviceFeatures2.features.inheritedQueries);
    LOG_D("HJ", "physical device 11 features\n");
    LOG_D("HJ", "\tstorageBuffer16BitAccess %d", physicalDeviceVulkan11Features.storageBuffer16BitAccess);
    LOG_D("HJ", "\tuniformAndStorageBuffer16BitAccess %d", physicalDeviceVulkan11Features.uniformAndStorageBuffer16BitAccess);
    LOG_D("HJ", "\tstoragePushConstant16 %d", physicalDeviceVulkan11Features.storagePushConstant16);
    LOG_D("HJ", "\tstorageInputOutput16 %d", physicalDeviceVulkan11Features.storageInputOutput16);
    LOG_D("HJ", "\tmultiview %d", physicalDeviceVulkan11Features.multiview);
    LOG_D("HJ", "\tmultiviewGeometryShader %d", physicalDeviceVulkan11Features.multiviewGeometryShader);
    LOG_D("HJ", "\tmultiviewTessellationShader %d", physicalDeviceVulkan11Features.multiviewTessellationShader);
    LOG_D("HJ", "\tvariablePointersStorageBuffer %d", physicalDeviceVulkan11Features.variablePointersStorageBuffer);
    LOG_D("HJ", "\tvariablePointers %d", physicalDeviceVulkan11Features.variablePointers);
    LOG_D("HJ", "\tprotectedMemory %d", physicalDeviceVulkan11Features.protectedMemory);
    LOG_D("HJ", "\tsamplerYcbcrConversion %d", physicalDeviceVulkan11Features.samplerYcbcrConversion);
    LOG_D("HJ", "\tshaderDrawParameters %d", physicalDeviceVulkan11Features.shaderDrawParameters);
    LOG_D("HJ", "physical device 12 features\n");
    LOG_D("HJ", "\t samplerMirrorClampToEdge %d", physicalDeviceVulkan12Features.samplerMirrorClampToEdge);
    LOG_D("HJ", "\t drawIndirectCount %d", physicalDeviceVulkan12Features.drawIndirectCount);
    LOG_D("HJ", "\t storageBuffer8BitAccess %d", physicalDeviceVulkan12Features.storageBuffer8BitAccess);
    LOG_D("HJ", "\t uniformAndStorageBuffer8BitAccess %d", physicalDeviceVulkan12Features.uniformAndStorageBuffer8BitAccess);
    LOG_D("HJ", "\t storagePushConstant8 %d", physicalDeviceVulkan12Features.storagePushConstant8);
    LOG_D("HJ", "\t shaderBufferInt64Atomics %d", physicalDeviceVulkan12Features.shaderBufferInt64Atomics);
    LOG_D("HJ", "\t shaderSharedInt64Atomics %d", physicalDeviceVulkan12Features.shaderSharedInt64Atomics);
    LOG_D("HJ", "\t shaderFloat16 %d", physicalDeviceVulkan12Features.shaderFloat16);
    LOG_D("HJ", "\t shaderInt8 %d", physicalDeviceVulkan12Features.shaderInt8);
    LOG_D("HJ", "\t descriptorIndexing %d", physicalDeviceVulkan12Features.descriptorIndexing);
    LOG_D("HJ", "\t shaderInputAttachmentArrayDynamicIndexing %d", physicalDeviceVulkan12Features.shaderInputAttachmentArrayDynamicIndexing);
    LOG_D("HJ", "\t shaderUniformTexelBufferArrayDynamicIndexing %d", physicalDeviceVulkan12Features.shaderUniformTexelBufferArrayDynamicIndexing);
    LOG_D("HJ", "\t shaderStorageTexelBufferArrayDynamicIndexing %d", physicalDeviceVulkan12Features.shaderStorageTexelBufferArrayDynamicIndexing);
    LOG_D("HJ", "\t shaderUniformBufferArrayNonUniformIndexing %d", physicalDeviceVulkan12Features.shaderUniformBufferArrayNonUniformIndexing);
    LOG_D("HJ", "\t shaderSampledImageArrayNonUniformIndexing %d", physicalDeviceVulkan12Features.shaderSampledImageArrayNonUniformIndexing);
    LOG_D("HJ", "\t shaderStorageBufferArrayNonUniformIndexing %d", physicalDeviceVulkan12Features.shaderStorageBufferArrayNonUniformIndexing);
    LOG_D("HJ", "\t shaderStorageImageArrayNonUniformIndexing %d", physicalDeviceVulkan12Features.shaderStorageImageArrayNonUniformIndexing);
    LOG_D("HJ", "\t shaderInputAttachmentArrayNonUniformIndexing %d", physicalDeviceVulkan12Features.shaderInputAttachmentArrayNonUniformIndexing);
    LOG_D("HJ", "\t shaderUniformTexelBufferArrayNonUniformIndexing %d", physicalDeviceVulkan12Features.shaderUniformTexelBufferArrayNonUniformIndexing);
    LOG_D("HJ", "\t shaderStorageTexelBufferArrayNonUniformIndexing %d", physicalDeviceVulkan12Features.shaderStorageTexelBufferArrayNonUniformIndexing);
    LOG_D("HJ", "\t descriptorBindingUniformBufferUpdateAfterBind %d", physicalDeviceVulkan12Features.descriptorBindingUniformBufferUpdateAfterBind);
    LOG_D("HJ", "\t descriptorBindingSampledImageUpdateAfterBind %d", physicalDeviceVulkan12Features.descriptorBindingSampledImageUpdateAfterBind);
    LOG_D("HJ", "\t descriptorBindingStorageImageUpdateAfterBind %d", physicalDeviceVulkan12Features.descriptorBindingStorageImageUpdateAfterBind);
    LOG_D("HJ", "\t descriptorBindingStorageBufferUpdateAfterBind %d", physicalDeviceVulkan12Features.descriptorBindingStorageBufferUpdateAfterBind);
    LOG_D("HJ", "\t descriptorBindingUniformTexelBufferUpdateAfterBind %d", physicalDeviceVulkan12Features.descriptorBindingUniformTexelBufferUpdateAfterBind);
    LOG_D("HJ", "\t descriptorBindingStorageTexelBufferUpdateAfterBind %d", physicalDeviceVulkan12Features.descriptorBindingStorageTexelBufferUpdateAfterBind);
    LOG_D("HJ", "\t descriptorBindingUpdateUnusedWhilePending %d", physicalDeviceVulkan12Features.descriptorBindingUpdateUnusedWhilePending);
    LOG_D("HJ", "\t descriptorBindingPartiallyBound %d", physicalDeviceVulkan12Features.descriptorBindingPartiallyBound);
    LOG_D("HJ", "\t descriptorBindingVariableDescriptorCount %d", physicalDeviceVulkan12Features.descriptorBindingVariableDescriptorCount);
    LOG_D("HJ", "\t runtimeDescriptorArray %d", physicalDeviceVulkan12Features.runtimeDescriptorArray);
    LOG_D("HJ", "\t samplerFilterMinmax %d", physicalDeviceVulkan12Features.samplerFilterMinmax);
    LOG_D("HJ", "\t scalarBlockLayout %d", physicalDeviceVulkan12Features.scalarBlockLayout);
    LOG_D("HJ", "\t imagelessFramebuffer %d", physicalDeviceVulkan12Features.imagelessFramebuffer);
    LOG_D("HJ", "\t uniformBufferStandardLayout %d", physicalDeviceVulkan12Features.uniformBufferStandardLayout);
    LOG_D("HJ", "\t shaderSubgroupExtendedTypes %d", physicalDeviceVulkan12Features.shaderSubgroupExtendedTypes);
    LOG_D("HJ", "\t separateDepthStencilLayouts %d", physicalDeviceVulkan12Features.separateDepthStencilLayouts);
    LOG_D("HJ", "\t hostQueryReset %d", physicalDeviceVulkan12Features.hostQueryReset);
    LOG_D("HJ", "\t timelineSemaphore %d", physicalDeviceVulkan12Features.timelineSemaphore);
    LOG_D("HJ", "\t bufferDeviceAddress %d", physicalDeviceVulkan12Features.bufferDeviceAddress);
    LOG_D("HJ", "\t bufferDeviceAddressCaptureReplay %d", physicalDeviceVulkan12Features.bufferDeviceAddressCaptureReplay);
    LOG_D("HJ", "\t bufferDeviceAddressMultiDevice %d", physicalDeviceVulkan12Features.bufferDeviceAddressMultiDevice);
    LOG_D("HJ", "\t vulkanMemoryModel %d", physicalDeviceVulkan12Features.vulkanMemoryModel);
    LOG_D("HJ", "\t vulkanMemoryModelDeviceScope %d", physicalDeviceVulkan12Features.vulkanMemoryModelDeviceScope);
    LOG_D("HJ", "\t vulkanMemoryModelAvailabilityVisibilityChains %d", physicalDeviceVulkan12Features.vulkanMemoryModelAvailabilityVisibilityChains);
    LOG_D("HJ", "\t shaderOutputViewportIndex %d", physicalDeviceVulkan12Features.shaderOutputViewportIndex);
    LOG_D("HJ", "\t shaderOutputLayer %d", physicalDeviceVulkan12Features.shaderOutputLayer);
    LOG_D("HJ", "\t subgroupBroadcastDynamicId %d", physicalDeviceVulkan12Features.subgroupBroadcastDynamicId);

    VkPhysicalDeviceMemoryProperties deviceMemoryProperties{};
    device.GetMemoryProperties(&deviceMemoryProperties);
    LOG_D("HJ", "physical device memory properties\n");
    for (int i=0; i<deviceMemoryProperties.memoryTypeCount; ++i) {
        LOG_D("HJ", "\t memoryTypes propertyFlags 0x%x heapIndex %u\n", deviceMemoryProperties.memoryTypes[i].propertyFlags, deviceMemoryProperties.memoryTypes[i].heapIndex);
    }
    for (int i=0; i<deviceMemoryProperties.memoryHeapCount; ++i) {
        LOG_D("HJ", "\t memoryHeaps size %llu flags 0x%x\n", (long long unsigned int) deviceMemoryProperties.memoryHeaps[i].size,
              deviceMemoryProperties.memoryHeaps[i].flags);
    }

    LOG_D("HJ", "physical device queue family properties\n");
    std::vector<VkQueueFamilyProperties> queueFamilyProperties = device.GetQueueFamilyProperties();
    for (auto it : queueFamilyProperties) {
        LOG_D("HJ", "\t queueFlags 0x%x queueCount %u timestampValidBits %u\n", it.queueFlags, it.queueCount, it.timestampValidBits);
    }

    LOG_D("HJ", "physical device extension properties\n");
    std::vector<VkExtensionProperties> extensionProperties = device.EnumerateExtensionProperties();
    for (const auto& it : extensionProperties) {
        LOG_D("HJ", "\t extensionName %s\n", it.extensionName);
    }

    LOG_D("HJ", "physical device layer properties\n");
    std::vector<VkLayerProperties> layerProperties = device.EnumerateLayerProperties();
    for (const auto& it : layerProperties) {
        LOG_D("HJ", "\t layerName %s\n", it.layerName);
    }
}