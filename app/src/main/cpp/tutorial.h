//
// Created by hj6231 on 2023/12/19.
//

#pragma once
#include <pthread.h>
#include <mutex>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/native_window_jni.h>
#include "create_info_factory.h"
#include "vulkan_instance.h"
#include "vulkan_physical_device.h"
#include "vulkan_surface.h"
#include "vulkan_swap_chain.h"

#include "triangle.h"

class Tutorial {
public:
    Tutorial(AAssetManager * asset_manager);
    ~Tutorial() = default;
    void StartThread(ANativeWindow* window);
    void StopThread();
    void Run();

    void CreateInstance();
    void DestroyInstance();
    bool pickPhysicalDevice();

    // TEST
//    void TestAOTCompilation(AAssetManager* asset_manager);
private:
    void CreateSurface(ANativeWindow* window);
    void DestroySurface();
    void CreateLogicalDevice();
    void DestroyLogicalDevice();
    void CreateSwapChain(ANativeWindow* window);
    void DestroySwapChain();
    void CreateImageViews();
    void DestroyImageViews();
    void CreateRenderPass();
    void DestroyRenderPass();
    void CreateGraphicPipeline();
    void DestroyGraphicPipeline();
    void CreateFrameBuffers();
    void DestroyFrameBuffers();

    void CreateCommandPool();
    void DestroyCommandPool();
    void CreateCommandBuffer();
    void DestroyCommandBuffer();
    void CreateSyncObjects();
    void DestroySyncObjects();

    void DrawFrame();

    VkSurfaceFormatKHR ChooseSwapSurfaceFormat();
    VkPresentModeKHR ChooseSwapPresentMode();
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, ANativeWindow* window);

    AAssetManager * asset_manager_;
    pthread_t thread_;
    std::mutex thread_state_mutex_;
    int thread_state_;
    ANativeWindow* window_;
    CreateInfoFactory create_info_factory_;

    VulkanInstance* instance_;
    VulkanPhysicalDevice* physical_device_;
    VulkanSurface* surface_;
    VulkanLogicDevice* logic_device_;
    VulkanQueue* graphic_queue_;
    VulkanQueue* present_queue_;
    VulkanSwapChain* swap_chain_;
    VulkanRenderPass* render_pass_;
    std::vector<VulkanFrameBuffer*> frame_buffers_;
    std::vector<VkImage> swap_chain_images_;
    std::vector<VulkanImageView*> swap_chain_image_views_;
    VulkanCommandPool* graphic_command_pool_;
    VulkanCommandBuffer* graphic_command_buffer_;
    VulkanSemaphore* frame_available_semaphore_;
    VulkanSemaphore* render_finished_semaphore_;
    VulkanFence* cpu_wait_;

    bool support_validation_;
    VkPhysicalDeviceVulkan11Features physical_device_vulkan_11_features_;
    VkPhysicalDeviceFeatures2 physical_device_features_;
    VkPhysicalDeviceProperties physical_device_properties_;
    uint32_t graphic_queue_family_index_;
    uint32_t present_queue_family_index_;
    VkSurfaceFormatKHR surface_format_;
    VkExtent2D swap_chain_extent_;
    VkDebugReportCallbackEXT callback_;

    VulkanObject* obj_;
};

