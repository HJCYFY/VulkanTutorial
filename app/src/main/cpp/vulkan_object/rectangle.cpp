//
// Created by hj6231 on 2023/12/29.
//

#include "rectangle.h"

const char Rectangle::kVertShaderSource[] =
        "#version 450\n"
        "layout(location = 0) in vec2 inPosition;\n"
        "layout(location = 1) in vec3 inColor;\n"
        "layout(location = 0) out vec3 fragColor;\n"
        "void main() {\n"
        "    gl_Position = vec4(inPosition, 0.0, 1.0);\n"
        "    fragColor = inColor;\n"
        "}\n";

const char Rectangle::kFragShaderSource[] =
        "#version 450\n"
        "layout(location = 0) in vec3 fragColor;\n"
        "layout(location = 0) out vec4 outColor;\n"
        "void main() {\n"
        "    outColor = vec4(fragColor, 1.0);\n"
        "}\n";

Rectangle::Rectangle(VulkanLogicDevice* device) :
        VulkanObject(device),
        vertex_buffer_(nullptr),
        indexes_buffer_(nullptr),
        vertex_memory_(nullptr),
        indexes_memory_(nullptr) {
    vertex_str_ = kVertShaderSource;
    fragment_str_ = kFragShaderSource;
}

int Rectangle::CreatePipeline(const VulkanRenderPass* render_pass) {
    VulkanShaderModule* vert_shader_module;
    VulkanShaderModule* frag_shader_module;
    VulkanPipelineLayout* pipeline_layout;
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
    pipeline_layout = CreatePipelineLayout();
    assert(pipeline_layout);
    if (pipeline_layout == nullptr) {
        goto ERROR_EXIT;
    }

    pipeline = CreateGraphicsPipeline(vert_shader_module, frag_shader_module, pipeline_layout, render_pass);
    assert(pipeline);
    if (pipeline == nullptr) {
        goto ERROR_EXIT;
    }
    pipeline_ = pipeline;

    VulkanLogicDevice::DestroyPipelineLayout(&pipeline_layout);
    VulkanLogicDevice::DestroyShaderModule(&vert_shader_module);
    VulkanLogicDevice::DestroyShaderModule(&frag_shader_module);

    CreateVertexBuffer();
    CopyDataToVertexBuffer();
    return 0;
    ERROR_EXIT:
    DestroyPipeline();
    return -1;
}

void Rectangle::DestroyPipeline() {
    DestroyVertexBuffer();
    if (pipeline_ != nullptr) {
        VulkanLogicDevice::DestroyPipelines(&pipeline_);
    }
}

void Rectangle::Draw(const VulkanCommandBuffer* command_buffer,
                     const VulkanRenderPass* render_pass,
                     const VulkanFrameBuffer* frame_buffer,
                     uint32_t frame_buffer_width,
                     uint32_t frame_buffer_height) const {
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
    render_pass_info.renderPass = render_pass->render_pass();
    render_pass_info.framebuffer = frame_buffer->frame_buffer();
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = {frame_buffer_width, frame_buffer_height};
    VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_color;
    command_buffer->CmdBeginRenderPass(&render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    command_buffer->CmdBindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_->pipeline());
    VkBuffer vertex_buffers[] = {vertex_buffer_->buffer()};
    VkDeviceSize offsets[] = {0};
    command_buffer->CmdBindVertexBuffers(0, 1, vertex_buffers, offsets);
    VkViewport viewport = GetVkViewport(frame_buffer_width, frame_buffer_height);
    command_buffer->CmdSetViewport(1, &viewport);
    VkRect2D scissor = GetScissor(frame_buffer_width, frame_buffer_height);
    command_buffer->CmdSetScissor(1, &scissor);
    command_buffer->CmdDraw(4, 1, 0, 0);
    command_buffer->CmdEndRenderPass();
    command_buffer->EndCommandBuffer();
}

void Rectangle::CreateVertexBuffer() {
    /* typedef struct VkBufferCreateInfo {
        VkStructureType        sType;
        const void*            pNext;
        VkBufferCreateFlags    flags;
        VkDeviceSize           size;
        VkBufferUsageFlags     usage;
        VkSharingMode          sharingMode;
        uint32_t               queueFamilyIndexCount;
        const uint32_t*        pQueueFamilyIndices;
    } VkBufferCreateInfo; */
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = sizeof(vertices_[0]) * vertices_.size();
    buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vertex_buffer_ = device_->CreateBuffer(&buffer_info);
    assert(vertex_buffer_);
    VkMemoryRequirements mem_requirements{};
    vertex_buffer_->GetBufferMemoryRequirements(&mem_requirements);
    VkPhysicalDeviceMemoryProperties mem_properties{};
    device_->GetPhysicalDeviceMemoryProperties(&mem_properties);

    /* typedef struct VkMemoryAllocateInfo {
        VkStructureType    sType;
        const void*        pNext;
        VkDeviceSize       allocationSize;
        uint32_t           memoryTypeIndex;
    } VkMemoryAllocateInfo; */
    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    VkResult ret = VulkanLogicDevice::GetMemoryType(&mem_properties, mem_requirements.memoryTypeBits,
                                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                    &alloc_info.memoryTypeIndex);
    assert(ret == VK_SUCCESS);
    vertex_memory_ = device_->AllocateMemory(&alloc_info);
    assert(vertex_memory_);
    vertex_memory_->BindBufferMemory(vertex_buffer_->buffer(), 0);
}

void Rectangle::CopyDataToVertexBuffer() {
    void* data;
    size_t size =  sizeof (vertices_[0]) * vertices_.size();
    vertex_memory_->MapMemory(0, size, &data);
    memcpy(data, vertices_.data(), size);
    vertex_memory_->UnmapMemory();
}

void Rectangle::DestroyVertexBuffer() {
    VulkanLogicDevice::FreeMemory(&vertex_memory_);
    VulkanLogicDevice::DestroyBuffer(&vertex_buffer_);
}

VulkanPipelineLayout* Rectangle::CreatePipelineLayout() const {
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

VulkanPipeline* Rectangle::CreateGraphicsPipeline(const VulkanShaderModule* vert_shader_module,
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

std::vector<VkPipelineShaderStageCreateInfo> Rectangle::GetPipelineShaderStageCreateInfos(
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

VkPipelineVertexInputStateCreateInfo Rectangle::GetPipelineVertexInputStateCreateInfo() const {
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
    vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(vertex_binding_descriptions_.size());
    vertex_input_info.pVertexBindingDescriptions = vertex_binding_descriptions_.data(); // Optional
    vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_attribute_descriptions_.size());
    vertex_input_info.pVertexAttributeDescriptions = vertex_attribute_descriptions_.data();
    return vertex_input_info;
}

VkPipelineInputAssemblyStateCreateInfo Rectangle::GetPipelineInputAssemblyStateCreateInfo() {
    /*typedef struct VkPipelineInputAssemblyStateCreateInfo {
        VkStructureType                            sType;
        const void*                                pNext;
        VkPipelineInputAssemblyStateCreateFlags    flags;
        VkPrimitiveTopology                        topology;
        VkBool32                                   primitiveRestartEnable;
    } VkPipelineInputAssemblyStateCreateInfo;*/
    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    input_assembly.primitiveRestartEnable = VK_FALSE;
    return input_assembly;
}

VkPipelineViewportStateCreateInfo Rectangle::GetPipelineViewportStateCreateInfo() {
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

VkPipelineRasterizationStateCreateInfo Rectangle::GetPipelineRasterizationStateCreateInfo() {
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
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
    rasterizer.lineWidth = 1.0f;
    return rasterizer;
}

VkPipelineMultisampleStateCreateInfo Rectangle::GetPipelineMultisampleStateCreateInfo() {
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

VkPipelineColorBlendStateCreateInfo Rectangle::GetPipelineColorBlendStateCreateInfo(
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

VkPipelineColorBlendAttachmentState Rectangle::GetPipelineColorBlendAttachmentState() {
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

VkPipelineDynamicStateCreateInfo Rectangle::GetPipelineDynamicStateCreateInfo(const std::vector<VkDynamicState>& dynamic_states) {
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