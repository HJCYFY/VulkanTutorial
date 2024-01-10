//
// Created by hj6231 on 2023/12/19.
//

#include "tutorial.h"

#include <cassert>
#include <vector>
#include <unistd.h>
#include "log.h"
#include "vulkan_utils.h"
#include "rectangle.h"
#include "rotate_rectangle.h"
#include "rgba_image_texture.h"
#include "nv12_image_texture.h"

void* thread_run(void* param) {
    auto* tutorial = (Tutorial*)param;
    tutorial->Run();
    return nullptr;
}

Tutorial::Tutorial(AAssetManager* asset_manager) :
        asset_manager_(asset_manager),
        thread_(0),
        thread_state_(0),
        window_(nullptr),
        instance_(nullptr),
        physical_device_(nullptr),
        surface_(nullptr),
        logic_device_(nullptr),
        graphic_queue_(nullptr),
        present_queue_(nullptr),
        swap_chain_(nullptr),
        render_pass_(nullptr),
        graphic_command_pool_(nullptr),
        graphic_command_buffer_(nullptr),
        frame_available_semaphore_(nullptr),
        render_finished_semaphore_(nullptr),
        cpu_wait_(nullptr),
        support_validation_(false),
        physical_device_vulkan_11_features_{},
        physical_device_features_{},
        physical_device_properties_{},
        graphic_queue_family_index_(0),
        present_queue_family_index_(0),
        surface_format_{},
        swap_chain_extent_{},
        callback_{},
        obj_(nullptr) {
    physical_device_vulkan_11_features_.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    physical_device_features_.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    physical_device_features_.pNext = &physical_device_vulkan_11_features_;
}

void Tutorial::StartThread(ANativeWindow* window) {
    window_  = window;
    std::unique_lock<std::mutex> lock(thread_state_mutex_);
    if (thread_state_ == 0) {
        thread_state_ = 1;
        pthread_create(&thread_, nullptr, thread_run, this);
    }
}

void Tutorial::StopThread() {
    std::unique_lock<std::mutex> lock(thread_state_mutex_);
    thread_state_ = 0;
    lock.unlock();
    void* ret = nullptr;
    pthread_join(thread_, &ret);
}

void Tutorial::Run() {
    CreateSurface(window_);
    CreateLogicalDevice();
    CreateSwapChain(window_);
    CreateImageViews();
    CreateRenderPass();
    CreateFrameBuffers();

    CreateCommandPool();
    CreateCommandBuffer();
    CreateSyncObjects();

    CreateGraphicPipeline();
    int thread_state;
    std::unique_lock<std::mutex> lock(thread_state_mutex_);
    thread_state = thread_state_;
    lock.unlock();
    while (thread_state == 1) {
        DrawFrame();
        usleep(33000);
        lock.lock();
        thread_state = thread_state_;
        lock.unlock();
    }
    logic_device_->DeviceWaitIdle();
    DestroyGraphicPipeline();

    DestroySyncObjects();
    DestroyCommandBuffer();
    DestroyCommandPool();
    DestroyFrameBuffers();
    DestroyRenderPass();
    DestroyImageViews();
    DestroySwapChain();
    DestroyLogicalDevice();
    DestroySurface();
}

void Tutorial::CreateInstance() {
    std::vector<VkLayerProperties> layer_properties =
            VulkanInstance::EnumerateInstanceLayerProperties();
    support_validation_ = CheckValidationLayerSupport(layer_properties);
    VkInstanceCreateInfo instance_create_info = create_info_factory_.GetInstanceCreateInfo(support_validation_);
    instance_ = VulkanInstance::CreateInstance(&instance_create_info);
    if (support_validation_) {
        VkDebugReportCallbackCreateInfoEXT debug_report_callback_create_info = create_info_factory_.GetDebugReportCallbackCreateInfo();
        instance_->CreateDebugReportCallbackEXT(&debug_report_callback_create_info, &callback_);
    }
}

void Tutorial::DestroyInstance() {
    if (instance_) {
        if (support_validation_) {
            instance_->DestroyDebugReportCallbackEXT(callback_);
        }
        VulkanInstance::DestroyInstance(&instance_);
    }
}

bool Tutorial::pickPhysicalDevice() {
    std::vector<VulkanPhysicalDevice> physical_devices = instance_->EnumeratePhysicalDevices();

    bool find_physical_device = false;
    for (auto& physical_device : physical_devices) {
        if (CheckPhysicalDeviceGraphicSupport(physical_device) &&
                CheckPhysicalDeviceSwapChainSupport(physical_device)) {
            physical_device_ = new VulkanPhysicalDevice(physical_device);
            find_physical_device = true;
            break;
        }
    }
    if (find_physical_device) {
        physical_device_->GetFeatures2(&physical_device_features_);
        physical_device_->GetProperties(&physical_device_properties_);
        return true;
    }
    return false;
}

void Tutorial::CreateSurface(ANativeWindow* window) {
    VkAndroidSurfaceCreateInfoKHR info = create_info_factory_.GetAndroidSurfaceCreateInfo(window);
    surface_ = instance_->CreateAndroidSurface(&info);
}

void Tutorial::DestroySurface() {
    VulkanInstance::DestroyAndroidSurface(&surface_);
}

void Tutorial::CreateLogicalDevice() {
    uint32_t family_index_num = 0;
    uint32_t indexes[10];
    GetGraphicQueueFamilyIndexes(*physical_device_, indexes, 10, &family_index_num);
    assert(family_index_num > 0);
    graphic_queue_family_index_ = indexes[0];
    bool is_graphic_queue_support_present =
        physical_device_->GetSurfaceSupport(graphic_queue_family_index_,
                                           surface_->surface());
    present_queue_family_index_ = graphic_queue_family_index_;
    if (!is_graphic_queue_support_present) {
        GetPresentQueueFamilyIndexes(*physical_device_, surface_->surface(), indexes, 10, &family_index_num);
        assert(family_index_num > 0);
        present_queue_family_index_ = indexes[0];
    }
    std::vector<VkDeviceQueueCreateInfo> device_queue_create_infos = create_info_factory_.GetDeviceQueueCreateInfos(graphic_queue_family_index_, present_queue_family_index_);
    VkDeviceCreateInfo device_create_info = create_info_factory_.GetDeviceCreateInfo(support_validation_, device_queue_create_infos);
    device_create_info.pNext = &physical_device_vulkan_11_features_;

    logic_device_ = physical_device_->CreateDevice(&device_create_info);
    graphic_queue_ = logic_device_->GetDeviceQueue(graphic_queue_family_index_, 0);
    present_queue_ = logic_device_->GetDeviceQueue(present_queue_family_index_, 0);
}

void Tutorial::DestroyLogicalDevice() {
    VulkanPhysicalDevice::DestroyDevice(&logic_device_);
}

void Tutorial::CreateSwapChain(ANativeWindow* window) {
    VkSurfaceCapabilitiesKHR capabilities{};
    physical_device_->GetSurfaceCapabilities(surface_->surface(), &capabilities);
    uint32_t image_count = capabilities.minImageCount + 1;

    surface_format_ = ChooseSwapSurfaceFormat();
    VkPresentModeKHR present_mode = ChooseSwapPresentMode();
    swap_chain_extent_ = ChooseSwapExtent(capabilities, window);

    std::vector<uint32_t> queue_family_indices;
    queue_family_indices.push_back(graphic_queue_family_index_);
    if (present_queue_family_index_ != graphic_queue_family_index_) {
        queue_family_indices.push_back(present_queue_family_index_);
    }

    VkSwapchainCreateInfoKHR create_info = create_info_factory_.GetSwapChainCreateInfo(
            surface_->surface(), image_count, surface_format_, swap_chain_extent_,
            queue_family_indices, present_mode, VK_NULL_HANDLE);
    swap_chain_ = logic_device_->CreateSwapChain(&create_info);
    swap_chain_images_ = swap_chain_->GetImages();
}

void Tutorial::DestroySwapChain() {
    VulkanLogicDevice::DestroySwapChain(&swap_chain_);
}

void Tutorial::CreateImageViews() {
    for (const auto& image : swap_chain_images_) {
        VkImageViewCreateInfo create_info = create_info_factory_.GetImageViewCreateInfo(image, surface_format_.format);
        VulkanImageView* image_view = logic_device_->CreateImageView(&create_info);
        swap_chain_image_views_.push_back(image_view);
    }
}

void Tutorial::DestroyImageViews() {
    for (auto& imageview : swap_chain_image_views_) {
        VulkanLogicDevice::DestroyImageView(&imageview);
    }
    swap_chain_image_views_.clear();
}

void Tutorial::CreateRenderPass() {
    std::vector<VkAttachmentDescription> attachments(1);
    attachments[0] = CreateInfoFactory::GetAttachmentDescription(surface_format_.format);
    VkAttachmentReference color_attachment_ref = CreateInfoFactory::GetAttachmentReference(0);
    std::vector<VkSubpassDescription> subpasses(1);
    subpasses[0] = CreateInfoFactory::GetSubpassDescription(color_attachment_ref);
    std::vector<VkSubpassDependency> dependencies(1);
    dependencies[0] = CreateInfoFactory::GetSubpassDependency();
    VkRenderPassCreateInfo render_pass_info = CreateInfoFactory::GetRenderPassCreateInfo(attachments, subpasses, dependencies);
    render_pass_ = logic_device_->CreateRenderPass(&render_pass_info);
}

void Tutorial::DestroyRenderPass() {
    VulkanLogicDevice::DestroyRenderPass(&render_pass_);
}

void Tutorial::CreateGraphicPipeline() {
    obj_ = new Nv12ImageTexture(asset_manager_, graphic_command_pool_, graphic_queue_, logic_device_);
    obj_->CreatePipeline(render_pass_);
}

void Tutorial::DestroyGraphicPipeline() {
    obj_->DestroyPipeline();
    delete obj_;
    obj_ = nullptr;
}

void Tutorial::CreateFrameBuffers() {
    frame_buffers_.resize(swap_chain_image_views_.size());
    for (size_t i = 0; i < swap_chain_image_views_.size(); i++) {
        std::vector<VkImageView> color_attachment;
        color_attachment.push_back(swap_chain_image_views_[i]->image_view());
        VkFramebufferCreateInfo frame_buffer_create_info = CreateInfoFactory::GetFramebufferCreateInfo(render_pass_, color_attachment, swap_chain_extent_.width, swap_chain_extent_.height);
        VulkanFrameBuffer* frame_buffer = logic_device_->CreateFrameBuffer(&frame_buffer_create_info);
        if (frame_buffer != nullptr) {
            frame_buffers_[i] = frame_buffer;
        } else {
            return;
        }
    }
}

void Tutorial::DestroyFrameBuffers() {
    for (auto& it : frame_buffers_) {
        VulkanLogicDevice::DestroyFrameBuffer(&it);
    }
}

void Tutorial::CreateCommandPool() {
    VkCommandPoolCreateInfo pool_create_info = CreateInfoFactory::GetCommandPoolCreateInfo(graphic_queue_family_index_);
    graphic_command_pool_ = logic_device_->CreateCommandPool(&pool_create_info);
}

void Tutorial::DestroyCommandPool() {
    VulkanLogicDevice::DestroyCommandPool(&graphic_command_pool_);
}

void Tutorial::CreateCommandBuffer() {
    graphic_command_buffer_ = graphic_command_pool_->AllocateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
}

void Tutorial::DestroyCommandBuffer() {
    VulkanCommandPool::FreeCommandBuffer(&graphic_command_buffer_);
}

void Tutorial::CreateSyncObjects() {
    /* typedef struct VkSemaphoreCreateInfo {
        VkStructureType           sType;
        const void*               pNext;
        VkSemaphoreCreateFlags    flags;
    } VkSemaphoreCreateInfo; */
    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    /* typedef struct VkFenceCreateInfo {
        VkStructureType       sType;
        const void*           pNext;
        VkFenceCreateFlags    flags;
    } VkFenceCreateInfo; */
    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    frame_available_semaphore_ = logic_device_->CreateSemaphore(&semaphore_info);
    assert(frame_available_semaphore_);
    render_finished_semaphore_ = logic_device_->CreateSemaphore(&semaphore_info);
    assert(render_finished_semaphore_);
    cpu_wait_ = logic_device_->CreateFence(&fence_info);
    assert(cpu_wait_);
}

void Tutorial::DestroySyncObjects() {
    VulkanLogicDevice::DestroySemaphore(&frame_available_semaphore_);
    VulkanLogicDevice::DestroySemaphore(&render_finished_semaphore_);
    VulkanLogicDevice::DestroyFence(&cpu_wait_);
}

void Tutorial::DrawFrame() {
    uint32_t image_index;
    logic_device_->AcquireNextImageKHR(swap_chain_->swap_chain(),
                                       UINT64_MAX,
                                       frame_available_semaphore_->semaphore(), VK_NULL_HANDLE, &image_index);
    VkFence fence = cpu_wait_->fence();
    logic_device_->WaitForFences( 1, &fence, VK_TRUE, UINT64_MAX);

    graphic_command_buffer_->ResetCommandBuffer(0);
    obj_->Draw(graphic_command_buffer_, render_pass_, frame_buffers_[image_index], swap_chain_extent_.width, swap_chain_extent_.height);
    /* typedef struct VkSubmitInfo {
        VkStructureType                sType;
        const void*                    pNext;
        uint32_t                       waitSemaphoreCount;
        const VkSemaphore*             pWaitSemaphores;
        const VkPipelineStageFlags*    pWaitDstStageMask;
        uint32_t                       commandBufferCount;
        const VkCommandBuffer*         pCommandBuffers;
        uint32_t                       signalSemaphoreCount;
        const VkSemaphore*             pSignalSemaphores;
    } VkSubmitInfo; */
    VkSemaphore wait_semaphores[] = {frame_available_semaphore_->semaphore()};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signal_semaphores[] = {render_finished_semaphore_->semaphore()};
    VkCommandBuffer command_buffer = graphic_command_buffer_->command_buffer();
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    logic_device_->ResetFences(1, &fence);
    VkResult ret = graphic_queue_->QueueSubmit(1, &submit_info, fence);
    assert(ret == VK_SUCCESS);
    /* typedef struct VkPresentInfoKHR {
        VkStructureType          sType;
        const void*              pNext;
        uint32_t                 waitSemaphoreCount;
        const VkSemaphore*       pWaitSemaphores;
        uint32_t                 swapchainCount;
        const VkSwapchainKHR*    pSwapchains;
        const uint32_t*          pImageIndices;
        VkResult*                pResults;
    } VkPresentInfoKHR; */
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swap_chains[] = {swap_chain_->swap_chain()};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &image_index;
    present_queue_->QueuePresentKHR(&present_info);
}

//void Tutorial::TestAOTCompilation(AAssetManager* asset_manager) {
//    AAsset* file = AAssetManager_open(asset_manager,
//                                      "shaders/triangle.vert.spv", AASSET_MODE_BUFFER);
//    size_t fileLength = AAsset_getLength(file);
//    char* fileContent = new char[fileLength];
//    AAsset_read(file, fileContent, fileLength);
//    delete[] fileContent;
//    AAsset_close(file);
//}

VkSurfaceFormatKHR Tutorial::ChooseSwapSurfaceFormat() {
    std::vector<VkSurfaceFormatKHR> formats =
        physical_device_->GetSurfaceFormats(surface_->surface());
    for (const auto& format : formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }
    return formats[0];
}

VkPresentModeKHR Tutorial::ChooseSwapPresentMode() {
    std::vector<VkPresentModeKHR> present_modes =
            physical_device_->GetSurfacePresentModes(surface_->surface());
    for (const auto& present_mode : present_modes) {
        if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return present_mode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Tutorial::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, ANativeWindow* window) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width = ANativeWindow_getWidth(window);
        int height = ANativeWindow_getHeight(window);

        VkExtent2D actualExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
        };
        return actualExtent;
    }
}