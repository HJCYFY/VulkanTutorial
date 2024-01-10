//
// Created by hj6231 on 2023/12/22.
//

#include "vulkan_object.h"

#include <cassert>
#include "log.h"

VulkanObject::VulkanObject(VulkanLogicDevice* device) :
        device_(device),
        pipeline_layout_(nullptr),
        pipeline_(nullptr) {
}

VkViewport VulkanObject::GetVkViewport(uint32_t win_width, uint32_t win_height) const {
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
    viewport.width = (float) win_width;
    viewport.height = (float) win_height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    return viewport;
}

VkRect2D VulkanObject::GetScissor(uint32_t win_width, uint32_t win_height) const {
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent.width = win_width;
    scissor.extent.height = win_height;
    return scissor;
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