//
// Created by hj6231 on 2023/12/25.
//

#include "triangle.h"
#include <cassert>
#include "log.h"

const char Triangle::kVertShaderSource[] =
        "#version 450\n"
        "layout(location = 0) out vec3 fragColor;\n"
        "vec2 positions[3] = vec2[](\n"
        "    vec2(0.0, -0.5),\n"
        "    vec2(0.5, 0.5),\n"
        "    vec2(-0.5, 0.5)\n"
        ");\n"
        "vec3 colors[3] = vec3[](\n"
        "    vec3(1.0, 0.0, 0.0),\n"
        "    vec3(0.0, 1.0, 0.0),\n"
        "    vec3(0.0, 0.0, 1.0)\n"
        ");"
        "void main() {\n"
        "    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);\n"
        "    fragColor = colors[gl_VertexIndex];\n"
        "}\n";

const char Triangle::kFragShaderSource[] =
        "#version 450\n"
        "layout(location = 0) in vec3 fragColor;\n"
        "layout(location = 0) out vec4 outColor;\n"
        "void main() {\n"
        "    outColor = vec4(fragColor, 1.0);\n"
        "}\n";


Triangle::Triangle(VulkanLogicDevice* device,
                   VkFormat swap_chain_image_format,
                   VkExtent2D frame_buffer_size) :
            VulkanObject(device, swap_chain_image_format, frame_buffer_size),
            pipeline_layout_(nullptr) {
    vertex_str_ = kVertShaderSource;
    fragment_str_ = kFragShaderSource;
}

int Triangle::CreatePipeline() {
    VulkanShaderModule* vert_shader_module;
    VulkanShaderModule* frag_shader_module;
    VulkanPipeline* pipeline;
    vert_shader_module = CreateShaderModule("VertShaderSrc",
                                            shaderc_glsl_vertex_shader,
                                            vertex_str_);
    assert(vert_shader_module);
    if (vert_shader_module == nullptr) {
        goto ERROR_EXIT;
    }
    frag_shader_module = CreateShaderModule("FragShaderSrc",
                                            shaderc_glsl_fragment_shader,
                                            fragment_str_);
    assert(frag_shader_module);
    if (frag_shader_module == nullptr) {
        goto ERROR_EXIT;
    }
    CreateRenderPass();

    pipeline_layout_ = CreatePipelineLayout();
    assert(pipeline_layout_);
    if (pipeline_layout_ == nullptr) {
        goto ERROR_EXIT;
    }

    pipeline = CreateGraphicsPipeline(vert_shader_module, frag_shader_module,
                                      pipeline_layout_, render_pass_);
    assert(pipeline);
    if (pipeline == nullptr) {
        goto ERROR_EXIT;
    }
    pipeline_ = pipeline;

    VulkanLogicDevice::DestroyShaderModule(&vert_shader_module);
    VulkanLogicDevice::DestroyShaderModule(&frag_shader_module);
    return 0;
    ERROR_EXIT:
    DestroyPipeline();
    return -1;
}

void Triangle::DestroyPipeline() {
    if (pipeline_ != nullptr) {
        VulkanLogicDevice::DestroyPipelines(&pipeline_);
    }
    if (pipeline_layout_) {
        VulkanLogicDevice::DestroyPipelineLayout(&pipeline_layout_);
    }
    DestroyRenderPass();
}

void Triangle::Draw(const VulkanCommandBuffer* command_buffer,
                    const VulkanFrameBuffer* frame_buffer) const {

    /* typedef struct VkCommandBufferBeginInfo {
        VkStructureType                          sType;
        const void*                              pNext;
        VkCommandBufferUsageFlags                flags;
        const VkCommandBufferInheritanceInfo*    pInheritanceInfo;
    } VkCommandBufferBeginInfo; */
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = 0; // Optional
    begin_info.pInheritanceInfo = nullptr; // Optional
    VkResult ret = command_buffer->BeginCommandBuffer(&begin_info);
    assert(ret == VK_SUCCESS);
    /* typedef struct VkRenderPassBeginInfo {
        VkStructureType        sType;
        const void*            pNext;
        VkRenderPass           renderPass;
        VkFramebuffer          framebuffer;
        VkRect2D               renderArea;
        uint32_t               clearValueCount;
        const VkClearValue*    pClearValues;
    } VkRenderPassBeginInfo; */
    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = render_pass_->render_pass();
    render_pass_info.framebuffer = frame_buffer->frame_buffer();
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = frame_buffer_size_;
    VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_color;
    command_buffer->CmdBeginRenderPass(&render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    command_buffer->CmdBindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_->pipeline());
    VkViewport viewport = GetVkViewport();
    command_buffer->CmdSetViewport(1, &viewport);
    VkRect2D scissor = GetScissor();
    command_buffer->CmdSetScissor(1, &scissor);
    command_buffer->CmdDraw(3, 1, 0, 0);
    command_buffer->CmdEndRenderPass();
    command_buffer->EndCommandBuffer();
}

void Triangle::CreateRenderPass() {
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
    color_attachment.format = swap_chain_image_format_;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    /*typedef struct VkAttachmentReference {
        uint32_t         attachment;
        VkImageLayout    layout;
    } VkAttachmentReference;*/
    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

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
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &color_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;
    render_pass_ = device_->CreateRenderPass(&render_pass_info);
}

VulkanPipelineLayout* Triangle::CreatePipelineLayout() const {
    /*typedef struct VkPipelineLayoutCreateInfo {
        VkStructureType                 sType;
        const void*                     pNext;
        VkPipelineLayoutCreateFlags     flags;
        uint32_t                        setLayoutCount;
        const VkDescriptorSetLayout*    pSetLayouts;
        uint32_t                        pushConstantRangeCount;
        const VkPushConstantRange*      pPushConstantRanges;
    } VkPipelineLayoutCreateInfo; */
    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 0; // Optional
    pipeline_layout_info.pSetLayouts = nullptr; // Optional
    pipeline_layout_info.pushConstantRangeCount = 0; // Optional
    pipeline_layout_info.pPushConstantRanges = nullptr; // Optional
    return device_->CreatePipelineLayout(&pipeline_layout_info);
}

VulkanPipeline* Triangle::CreateGraphicsPipeline(const VulkanShaderModule* vert_shader_module,
                                                 const VulkanShaderModule* frag_shader_module,
                                                 const VulkanPipelineLayout* layout,
                                                 const VulkanRenderPass* render_pass) const {
    /* typedef struct VkGraphicsPipelineCreateInfo {
        VkStructureType                                  sType;
        const void*                                      pNext;
        VkPipelineCreateFlags                            flags;
        uint32_t                                         stageCount;
        const VkPipelineShaderStageCreateInfo*           pStages;
        const VkPipelineVertexInputStateCreateInfo*      pVertexInputState;
        const VkPipelineInputAssemblyStateCreateInfo*    pInputAssemblyState;
        const VkPipelineTessellationStateCreateInfo*     pTessellationState;
        const VkPipelineViewportStateCreateInfo*         pViewportState;
        const VkPipelineRasterizationStateCreateInfo*    pRasterizationState;
        const VkPipelineMultisampleStateCreateInfo*      pMultisampleState;
        const VkPipelineDepthStencilStateCreateInfo*     pDepthStencilState;
        const VkPipelineColorBlendStateCreateInfo*       pColorBlendState;
        const VkPipelineDynamicStateCreateInfo*          pDynamicState;
        VkPipelineLayout                                 layout;
        VkRenderPass                                     renderPass;
        uint32_t                                         subpass;
        VkPipeline                                       basePipelineHandle;
        int32_t                                          basePipelineIndex;
    } VkGraphicsPipelineCreateInfo; */
    VkGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    std::vector<VkPipelineShaderStageCreateInfo> shader_stage_create_infos =
            GetPipelineShaderStageCreateInfos(vert_shader_module, frag_shader_module);
    pipeline_info.stageCount = static_cast<uint32_t>(shader_stage_create_infos.size());
    pipeline_info.pStages = shader_stage_create_infos.data();
    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = GetPipelineVertexInputStateCreateInfo();
    pipeline_info.pVertexInputState = &vertex_input_state_create_info;
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info = GetPipelineInputAssemblyStateCreateInfo();
    pipeline_info.pInputAssemblyState = &input_assembly_state_create_info;
    pipeline_info.pTessellationState = nullptr;
    VkPipelineViewportStateCreateInfo viewport_state_create_info = GetPipelineViewportStateCreateInfo();
    pipeline_info.pViewportState = &viewport_state_create_info;
    VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = GetPipelineRasterizationStateCreateInfo();
    pipeline_info.pRasterizationState = &rasterization_state_create_info;
    VkPipelineMultisampleStateCreateInfo multisample_state_create_info = GetPipelineMultisampleStateCreateInfo();
    pipeline_info.pMultisampleState = &multisample_state_create_info;
    pipeline_info.pDepthStencilState = nullptr;
    VkPipelineColorBlendAttachmentState color_blend_attachment = GetPipelineColorBlendAttachmentState();
    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = GetPipelineColorBlendStateCreateInfo(&color_blend_attachment);
    pipeline_info.pColorBlendState = &color_blend_state_create_info;
    VkPipelineDynamicStateCreateInfo dynamic_state_create_info = GetPipelineDynamicStateCreateInfo(dynamic_states_);
    pipeline_info.pDynamicState = &dynamic_state_create_info;
    pipeline_info.layout = layout->layout();
    pipeline_info.renderPass = render_pass->render_pass();
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipeline_info.basePipelineIndex = -1; // Optional
    return device_->CreateGraphicPipeline(&pipeline_info);
}

std::vector<VkPipelineShaderStageCreateInfo> Triangle::GetPipelineShaderStageCreateInfos(
        const VulkanShaderModule* vert_shader_module, const VulkanShaderModule* frag_shader_module) {
    /*typedef struct VkPipelineShaderStageCreateInfo {
        VkStructureType                     sType;
        const void*                         pNext;
        VkPipelineShaderStageCreateFlags    flags;
        VkShaderStageFlagBits               stage;
        VkShaderModule                      module;
        const char*                         pName;
        const VkSpecializationInfo*         pSpecializationInfo;
    } VkPipelineShaderStageCreateInfo;*/
//    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages(2);
    shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shader_stages[0].module = vert_shader_module->shader_module();
    shader_stages[0].pName = "main";

    shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader_stages[1].module = frag_shader_module->shader_module();
    shader_stages[1].pName = "main";
    return shader_stages;
}

VkPipelineVertexInputStateCreateInfo Triangle::GetPipelineVertexInputStateCreateInfo() {
    /* typedef struct VkPipelineVertexInputStateCreateInfo {
        VkStructureType                             sType;
        const void*                                 pNext;
        VkPipelineVertexInputStateCreateFlags       flags;
        uint32_t                                    vertexBindingDescriptionCount;
        const VkVertexInputBindingDescription*      pVertexBindingDescriptions;
        uint32_t                                    vertexAttributeDescriptionCount;
        const VkVertexInputAttributeDescription*    pVertexAttributeDescriptions;
    } VkPipelineVertexInputStateCreateInfo; */
    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 0;
    vertex_input_info.pVertexBindingDescriptions = nullptr; // Optional
    vertex_input_info.vertexAttributeDescriptionCount = 0;
    vertex_input_info.pVertexAttributeDescriptions = nullptr;
    return vertex_input_info;
}

VkPipelineInputAssemblyStateCreateInfo Triangle::GetPipelineInputAssemblyStateCreateInfo() {
    /*typedef struct VkPipelineInputAssemblyStateCreateInfo {
        VkStructureType                            sType;
        const void*                                pNext;
        VkPipelineInputAssemblyStateCreateFlags    flags;
        VkPrimitiveTopology                        topology;
        VkBool32                                   primitiveRestartEnable;
    } VkPipelineInputAssemblyStateCreateInfo;*/
    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;
    return input_assembly;
}

VkPipelineViewportStateCreateInfo Triangle::GetPipelineViewportStateCreateInfo() {
    /*typedef struct VkPipelineViewportStateCreateInfo {
        VkStructureType                       sType;
        const void*                           pNext;
        VkPipelineViewportStateCreateFlags    flags;
        uint32_t                              viewportCount;
        const VkViewport*                     pViewports;
        uint32_t                              scissorCount;
        const VkRect2D*                       pScissors;
    } VkPipelineViewportStateCreateInfo;*/
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    return viewportState;
}

VkPipelineRasterizationStateCreateInfo Triangle::GetPipelineRasterizationStateCreateInfo() {
    /* typedef struct VkPipelineRasterizationStateCreateInfo {
        VkStructureType                            sType;
        const void*                                pNext;
        VkPipelineRasterizationStateCreateFlags    flags;
        VkBool32                                   depthClampEnable;
        VkBool32                                   rasterizerDiscardEnable;
        VkPolygonMode                              polygonMode;
        VkCullModeFlags                            cullMode;
        VkFrontFace                                frontFace;
        VkBool32                                   depthBiasEnable;
        float                                      depthBiasConstantFactor;
        float                                      depthBiasClamp;
        float                                      depthBiasSlopeFactor;
        float                                      lineWidth;
    } VkPipelineRasterizationStateCreateInfo; */
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE; // 设置为VK_TRUE，则超出近平面和远平面的片段将被夹紧，而不是丢弃它们。这在某些特殊情况下很有用，例如阴影贴图。使用此功能需要启用 GPU 功能
    rasterizer.rasterizerDiscardEnable = VK_FALSE; // 设置为VK_TRUE，则几何图形永远不会通过光栅化阶段。这基本上禁用了帧缓冲区的任何输出
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
    rasterizer.lineWidth = 1.0f;
    return rasterizer;
}

VkPipelineMultisampleStateCreateInfo Triangle::GetPipelineMultisampleStateCreateInfo() {
    /* typedef struct VkPipelineMultisampleStateCreateInfo {
        VkStructureType                          sType;
        const void*                              pNext;
        VkPipelineMultisampleStateCreateFlags    flags;
        VkSampleCountFlagBits                    rasterizationSamples;
        VkBool32                                 sampleShadingEnable;
        float                                    minSampleShading;
        const VkSampleMask*                      pSampleMask;
        VkBool32                                 alphaToCoverageEnable;
        VkBool32                                 alphaToOneEnable;
    } VkPipelineMultisampleStateCreateInfo; */
    VkPipelineMultisampleStateCreateInfo multisample{};
    multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample.sampleShadingEnable = VK_FALSE;
    multisample.minSampleShading = 1.0f; // Optional
    multisample.pSampleMask = nullptr; // Optional
    multisample.alphaToCoverageEnable = VK_FALSE; // Optional
    multisample.alphaToOneEnable = VK_FALSE; // Optional
    return multisample;
}

VkPipelineColorBlendStateCreateInfo Triangle::GetPipelineColorBlendStateCreateInfo(
        const VkPipelineColorBlendAttachmentState* color_blend_attachment) {
    /* typedef struct VkPipelineColorBlendStateCreateInfo {
        VkStructureType                               sType;
        const void*                                   pNext;
        VkPipelineColorBlendStateCreateFlags          flags;
        VkBool32                                      logicOpEnable;
        VkLogicOp                                     logicOp;
        uint32_t                                      attachmentCount;
        const VkPipelineColorBlendAttachmentState*    pAttachments;
        float                                         blendConstants[4];
    } VkPipelineColorBlendStateCreateInfo;*/
    VkPipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE; // 设置为VK_TRUE,将禁用Attachments混合
    color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = color_blend_attachment;
    color_blending.blendConstants[0] = 0.0f; // Optional
    color_blending.blendConstants[1] = 0.0f; // Optional
    color_blending.blendConstants[2] = 0.0f; // Optional
    color_blending.blendConstants[3] = 0.0f; // Optional
    return color_blending;
}

VkPipelineColorBlendAttachmentState Triangle::GetPipelineColorBlendAttachmentState() {
    /* typedef struct VkPipelineColorBlendAttachmentState {
        VkBool32                 blendEnable;
        VkBlendFactor            srcColorBlendFactor;
        VkBlendFactor            dstColorBlendFactor;
        VkBlendOp                colorBlendOp;
        VkBlendFactor            srcAlphaBlendFactor;
        VkBlendFactor            dstAlphaBlendFactor;
        VkBlendOp                alphaBlendOp;
        VkColorComponentFlags    colorWriteMask;
    } VkPipelineColorBlendAttachmentState; */
    VkPipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.blendEnable = VK_TRUE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    return color_blend_attachment;
}

VkPipelineDynamicStateCreateInfo Triangle::GetPipelineDynamicStateCreateInfo(const std::vector<VkDynamicState>& dynamic_states) {
    /* typedef struct VkPipelineDynamicStateCreateInfo {
        VkStructureType                      sType;
        const void*                          pNext;
        VkPipelineDynamicStateCreateFlags    flags;
        uint32_t                             dynamicStateCount;
        const VkDynamicState*                pDynamicStates;
    } VkPipelineDynamicStateCreateInfo; */
    VkPipelineDynamicStateCreateInfo dynamic_state_create_info{};
    dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_create_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamic_state_create_info.pDynamicStates = dynamic_states.data();
    return dynamic_state_create_info;
}