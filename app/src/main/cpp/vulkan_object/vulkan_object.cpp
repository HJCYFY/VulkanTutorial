//
// Created by hj6231 on 2023/12/22.
//

#include "vulkan_object.h"

#include <cassert>
#include "log.h"

VulkanObject::VulkanObject(VulkanLogicDevice* device,
                           VkFormat swap_chain_image_format,
                           VkExtent2D frame_buffer_size) :
        device_(device),
        swap_chain_image_format_(swap_chain_image_format),
        frame_buffer_size_(frame_buffer_size),
        render_pass_(nullptr),
        pipeline_(nullptr),
        color_attachment_image_(nullptr),
        color_attachment_image_memory_(nullptr),
        color_attachment_image_view_(nullptr),
        depth_attachment_image_(nullptr),
        depth_attachment_image_memory_(nullptr),
        depth_attachment_image_view_(nullptr) {
}

VkViewport VulkanObject::GetVkViewport() const {
    /*typedef struct VkViewport {
        float    x;
        float    y;
        float    width;
        float    height;
        float    minDepth;
        float    maxDepth;
    } VkViewport;*/
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) frame_buffer_size_.width;
    viewport.height = (float) frame_buffer_size_.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    return viewport;
}

VkRect2D VulkanObject::GetScissor() const {
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent.width = frame_buffer_size_.width;
    scissor.extent.height = frame_buffer_size_.height;
    return scissor;
}

VulkanRenderPass* VulkanObject::render_pass() {
    return render_pass_;
}

VulkanImageView* VulkanObject::color_attachment_image_view() {
    return color_attachment_image_view_;
}

VulkanImageView* VulkanObject::depth_attachment_image_view() {
    return depth_attachment_image_view_;
}

VulkanPipeline* VulkanObject::pipeline() {
    return pipeline_;
}

std::vector<uint32_t> VulkanObject::CompileFile(const std::string& source_name,
                                                shaderc_shader_kind kind,
                                                const std::string& source,
                                                bool optimize) {
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

//    // Like -DMY_DEFINE=1
//    options.AddMacroDefinition("MY_DEFINE", "1");
    if (optimize) options.SetOptimizationLevel(shaderc_optimization_level_size);

    shaderc::SpvCompilationResult module =
            compiler.CompileGlslToSpv(source, kind, source_name.c_str(), options);

    if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
        LOG_E("CompileFile", "module.GetCompilationStatus() %d", module.GetCompilationStatus());
        return {};
    }

    return {module.cbegin(), module.cend()};
}

VulkanShaderModule* VulkanObject::CreateShaderModule(const std::string& name,
                                                     shaderc_shader_kind kind,
                                                     const std::string& source) const {
    std::vector<uint32_t> code =
            CompileFile(name, kind, source);
    if (code.empty()) {
        return nullptr;
    }
    /*typedef struct VkShaderModuleCreateInfo {
        VkStructureType              sType;
        const void*                  pNext;
        VkShaderModuleCreateFlags    flags;
        size_t                       codeSize;
        const uint32_t*              pCode;
    } VkShaderModuleCreateInfo;*/
    VkShaderModuleCreateInfo shader_module_create_info{};
    shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_create_info.codeSize = code.size() * sizeof (uint32_t);
    shader_module_create_info.pCode = code.data();
    return device_->CreateShaderModule(&shader_module_create_info);
}

void VulkanObject::DestroyRenderPass() {
    VulkanLogicDevice::DestroyRenderPass(&render_pass_);
}