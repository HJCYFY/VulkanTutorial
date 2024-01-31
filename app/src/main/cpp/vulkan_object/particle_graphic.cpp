//
// Created by hj6231 on 2024/1/26.
//

#include "particle_graphic.h"

#include <array>

static const char kVertShaderSource[] =
        "#version 450\n"
        "\n"
        "layout(location = 0) in vec2 inPosition;\n"
        "layout(location = 1) in vec4 inColor;\n"
        "\n"
        "layout(location = 0) out vec3 fragColor;\n"
        "\n"
        "void main() {\n"
        "\n"
        "    gl_PointSize = 14.0;\n"
        "    gl_Position = vec4(inPosition.xy, 1.0, 1.0);\n"
        "    fragColor = inColor.rgb;\n"
        "}\n";

static const char kFragShaderSource[] =
        "#version 450\n"
        "\n"
        "layout(location = 0) in vec3 fragColor;\n"
        "\n"
        "layout(location = 0) out vec4 outColor;\n"
        "\n"
        "void main() {\n"
        "\n"
        "    vec2 coord = gl_PointCoord - vec2(0.5);\n"
        "    outColor = vec4(fragColor, 0.5 - length(coord));\n"
        "}\n";

ParticleGraphic::ParticleGraphic(VulkanLogicDevice* device,
                                 VkFormat swap_chain_image_format,
                                 VkExtent2D frame_buffer_size,
                                 Particle* particle) :
        VulkanObject(device, swap_chain_image_format, frame_buffer_size),
        particle_(particle) {

}

int ParticleGraphic::CreatePipeline() {
    VulkanShaderModule* vertexShaderModule = CreateShaderModule("kVertShaderSource", shaderc_vertex_shader, kVertShaderSource);
    VulkanShaderModule* fragmentShaderModule =CreateShaderModule("kFragShaderSource", shaderc_fragment_shader, kFragShaderSource);
    assert(vertexShaderModule);
    assert(fragmentShaderModule);

    std::array<VkPipelineShaderStageCreateInfo, 2> pipelineShaderStageCreateInfos;
    pipelineShaderStageCreateInfos[0] = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module= vertexShaderModule->shader_module(),
        .pName = "main",
        .pSpecializationInfo = nullptr
    };
    pipelineShaderStageCreateInfos[1] = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module= fragmentShaderModule->shader_module(),
            .pName = "main",
            .pSpecializationInfo = nullptr
    };

    std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions = {
            {0, sizeof (particle_t), VK_VERTEX_INPUT_RATE_VERTEX}
    };

    std::vector<VkVertexInputAttributeDescription>  vertexInputAttributeDescriptions = {
            { 0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(particle_t, pos) },
            { 1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(particle_t, color) }
    };

    VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindingDescriptions.size()),
        .pVertexBindingDescriptions = vertexInputBindingDescriptions.data(),
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributeDescriptions.size()),
        .pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data()
    };

    VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    VkViewport viewport = {0, 0,
                           (float)frame_buffer_size_.width, (float)frame_buffer_size_.height,
                           0.0f, 1.0f};
    VkRect2D scissor = {{0, 0,}, {frame_buffer_size_.width, frame_buffer_size_.height}};
    VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor
    };

    VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthClampEnable = VK_FALSE,
        .lineWidth = 1.0
    };

    VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 0.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE
    };

    VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthTestEnable = VK_FALSE,
        .depthWriteEnable = VK_FALSE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
        .front = {},
        .back = {},
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 1.0f
    };

    VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState {
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor= VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_CLEAR,
        .attachmentCount = 1,
        .pAttachments = &pipelineColorBlendAttachmentState,
        .blendConstants = {}
    };

    VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .dynamicStateCount = 0,
        .pDynamicStates = nullptr
    };

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr
    };
    pipeline_layout_ = device_->CreatePipelineLayout(&pipelineLayoutCreateInfo);
    assert(pipeline_layout_);

    VkAttachmentDescription attachmentDescription {
        .flags = 0,
        .format = swap_chain_image_format_,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference attachmentReference{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpassDescription {
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentReference,
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment = nullptr,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = nullptr
    };

    VkRenderPassCreateInfo renderPassCreateInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .attachmentCount = 1,
        .pAttachments = &attachmentDescription,
        .subpassCount = 1,
        .pSubpasses = &subpassDescription
    };
    render_pass_ = device_->CreateRenderPass(&renderPassCreateInfo);

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stageCount = static_cast<uint32_t>(pipelineShaderStageCreateInfos.size()),
        .pStages = pipelineShaderStageCreateInfos.data(),
        .pVertexInputState = &pipelineVertexInputStateCreateInfo,
        .pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo,
        .pTessellationState = nullptr,
        .pViewportState = &pipelineViewportStateCreateInfo,
        .pRasterizationState = &pipelineRasterizationStateCreateInfo,
        .pMultisampleState = &pipelineMultisampleStateCreateInfo,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &pipelineColorBlendStateCreateInfo,
        .pDynamicState = &pipelineDynamicStateCreateInfo,
        .layout = pipeline_layout_->layout(),
        .renderPass = render_pass_->render_pass(),
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };
    pipeline_ = device_->CreateGraphicPipeline(&graphicsPipelineCreateInfo);
    assert(pipeline_);

    VulkanLogicDevice::DestroyShaderModule(&vertexShaderModule);
    VulkanLogicDevice::DestroyShaderModule(&fragmentShaderModule);
    return 0;
}

void ParticleGraphic::DestroyPipeline() {
    VulkanLogicDevice::DestroyPipelines(&pipeline_);
    VulkanLogicDevice::DestroyRenderPass(&render_pass_);
    VulkanLogicDevice::DestroyPipelineLayout(&pipeline_layout_);
}

void ParticleGraphic::Draw(const VulkanCommandBuffer* command_buffer,
          const VulkanFrameBuffer* frame_buffer) const {
    VkCommandBufferBeginInfo commandBufferBeginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pInheritanceInfo = nullptr
    };
//    VkResult ret = command_buffer->BeginCommandBuffer(&commandBufferBeginInfo);
//    assert(ret == VK_SUCCESS);
    VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    VkRenderPassBeginInfo renderPassBeginInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = render_pass_->render_pass(),
        .framebuffer = frame_buffer->frame_buffer(),
        .renderArea.offset = {0, 0},
        .renderArea.extent = {frame_buffer_size_.width, frame_buffer_size_.height},
        .clearValueCount = 1,
        .pClearValues = &clear_color
    };
    command_buffer->CmdBeginRenderPass(&renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    command_buffer->CmdBindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_->pipeline());
    VkBuffer vertex_buffers[] = {particle_->GetVertexBuffer()};
    VkDeviceSize offsets[] = {0};
    command_buffer->CmdBindVertexBuffers(0, 1, vertex_buffers, offsets);
    command_buffer->CmdDraw(Particle::PARTICLE_COUNT, 1, 0, 0);
    command_buffer->CmdEndRenderPass();
    command_buffer->EndCommandBuffer();
}