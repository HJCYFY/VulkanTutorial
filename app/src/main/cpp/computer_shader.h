//
// Created by hj6231 on 2024/1/23.
//

#pragma once
#include "tutorial_base.h"
#include "vulkan_instance.h"
#include "vulkan_physical_device.h"
#include "particle.h"
#include "particle_graphic.h"

class ComputerShader : public TutorialBase {
public:
    ComputerShader(AAssetManager * asset_manager);
    ~ComputerShader() = default;
    void Run() override;

    void CreateInstance() override;
    void DestroyInstance() override;
    bool PickPhysicalDevice() override;

private:
    void CreateSurface(ANativeWindow* window);
    void DestroySurface();

    void CreateLogicalDevice();
    void DestroyLogicalDevice();

    void CreateCommandPool();
    void DestroyCommandPool();
    void CreateCommandBuffer();
    void DestroyCommandBuffer();
    void CreateSyncObjects();
    void DestroySyncObjects();

    void CreateComputerPipeline();
    void DestroyComputerPipeline();
    void CreateGraphicPipeline();
    void DestroyGraphicPipeline();

    void CreateSwapChain(ANativeWindow* window);
    void DestroySwapChain();
    void CreateImageViews();
    void DestroyImageViews();
    void CreateFrameBuffers(VkRenderPass renderPass);
    void DestroyFrameBuffers();

    static void QueryPhysicalDeviceInfo(const VulkanPhysicalDevice& device);
    VulkanInstance* instance_;
    VkDebugReportCallbackEXT callback_;
    VulkanPhysicalDevice* physical_device_;

    VulkanSurface* surface_;
    uint32_t queue_family_index_;
    VulkanLogicDevice* logic_device_;
    VulkanQueue* queue_;
    VulkanSwapChain* swap_chain_;
    std::vector<VulkanImageView*> swap_chain_image_views_;
    std::vector<VulkanFrameBuffer*> frame_buffers_;
    VkSurfaceFormatKHR surface_format_;
    VkExtent2D swap_chain_extent_;

    VulkanCommandPool* graphic_command_pool_;
    VulkanCommandBuffer* graphic_command_buffer_;

    std::vector<VulkanFence*> cpu_wait_fence_;
    std::vector<VulkanSemaphore*> swap_chain_image_available_semaphore_;
    std::vector<VulkanSemaphore*> render_finished_semaphore_;

    Particle* particle_;
    ParticleGraphic* particle_graphic_;

    const static int FRAME_IN_FLIGHT = 1;
};


