//
// Created by hj6231 on 2024/1/10.
//

#include "depth_triangle.h"
#include <cassert>
#include "log.h"

const char DepthTriangle::kVertShaderSource[] =
        "#version 450\n"
        "layout(location = 0) in vec3 pos;\n"
        "layout(location = 1) in vec3 color;\n"
        "layout(location = 0) out vec3 fragColor;\n"
        "void main() {\n"
        "    gl_Position = vec4(pos, 1.0);\n"
        "    fragColor = color;\n"
        "}\n";

const char DepthTriangle::kFragShaderSource[] =
        "#version 450\n"
        "layout(location = 0) in vec3 fragColor;\n"
        "layout(location = 0) out vec4 outColor;\n"
        "void main() {\n"
        "    outColor = vec4(fragColor, 1.0);\n"
        "}\n";

DepthTriangle::DepthTriangle(VulkanCommandPool* command_pool,
                             VulkanQueue* graphic_queue,
                             VulkanLogicDevice* device,
                             VkFormat swap_chain_image_format,
                             VkExtent2D frame_buffer_size) :
        VulkanObject(device, swap_chain_image_format, frame_buffer_size),
        command_pool_(command_pool),
        graphic_queue_(graphic_queue),
        pipeline_layout_(nullptr),
        vertex_buffer_(nullptr),
        vertex_memory_(nullptr),
        indices_buffer_(nullptr),
        indices_memory_(nullptr) {
    vertex_str_ = kVertShaderSource;
    fragment_str_ = kFragShaderSource;
}

int DepthTriangle::CreatePipeline() {
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
    LoadResource();
    CreateRenderPass();
    CreateVertexBuffer();
    CreateIndexBuffer();

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

void DepthTriangle::DestroyPipeline() {
    if (pipeline_ != nullptr) {
        VulkanLogicDevice::DestroyPipelines(&pipeline_);
    }
    if (pipeline_layout_) {
        VulkanLogicDevice::DestroyPipelineLayout(&pipeline_layout_);
    }
    DestroyVertexBuffer();
    DestroyIndexBuffer();
    DestroyRenderPass();
    DestroyDepthImageView();
    DestroyDepthImage();
}

void DepthTriangle::Draw(const VulkanCommandBuffer* command_buffer,
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
    VkClearValue clear_colors[2];

    clear_colors[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clear_colors[1].depthStencil = {1.0f, 0};

    render_pass_info.clearValueCount = 2;
    render_pass_info.pClearValues = clear_colors;
    command_buffer->CmdBeginRenderPass(&render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    command_buffer->CmdBindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_->pipeline());
    VkViewport viewport = GetVkViewport();
    command_buffer->CmdSetViewport(1, &viewport);
    VkRect2D scissor = GetScissor();
    command_buffer->CmdSetScissor(1, &scissor);

    command_buffer->CmdBindIndexBuffer(indices_buffer_->buffer(), 0, VK_INDEX_TYPE_UINT16);
    VkBuffer vertexes[] = {vertex_buffer_->buffer()};
    VkDeviceSize offsets[] = {0};
    command_buffer->CmdBindVertexBuffers(0, 1, vertexes, offsets);

    command_buffer->CmdDrawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    command_buffer->CmdEndRenderPass();
    command_buffer->EndCommandBuffer();
}

void DepthTriangle::LoadResource() {
    CreateDepthImage();
    CreateDepthImageView();
}

void DepthTriangle::CreateRenderPass() {
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

    VkAttachmentDescription depth_attachment{};
    depth_attachment.format = VK_FORMAT_D32_SFLOAT;
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    /*typedef struct VkAttachmentReference {
        uint32_t         attachment;
        VkImageLayout    layout;
    } VkAttachmentReference;*/
    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

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
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

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
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription attachments[] = {color_attachment, depth_attachment};
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
    render_pass_info.attachmentCount = 2;
    render_pass_info.pAttachments = attachments;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;
    render_pass_ = device_->CreateRenderPass(&render_pass_info);
}

void DepthTriangle::CreateVertexBuffer() {
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
    ret = vertex_memory_->BindBufferMemory(vertex_buffer_->buffer(), 0);
    assert(ret == VK_SUCCESS);

    void* data = nullptr;
    ret = vertex_memory_->MapMemory(0, buffer_info.size, &data);
    assert(ret == VK_SUCCESS);
    memcpy(data, vertices_.data(), buffer_info.size);
    vertex_memory_->UnmapMemory();
}

void DepthTriangle::DestroyVertexBuffer() {
    VulkanLogicDevice::FreeMemory(&vertex_memory_);
    VulkanLogicDevice::DestroyBuffer(&vertex_buffer_);
}

void DepthTriangle::CreateIndexBuffer() {
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
    buffer_info.size = sizeof(indices[0]) * indices.size();
    buffer_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    indices_buffer_ = device_->CreateBuffer(&buffer_info);
    assert(indices_buffer_);
    VkMemoryRequirements mem_requirements{};
    indices_buffer_->GetBufferMemoryRequirements(&mem_requirements);
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
    indices_memory_ = device_->AllocateMemory(&alloc_info);
    assert(indices_memory_);
    ret = indices_memory_->BindBufferMemory(indices_buffer_->buffer(), 0);
    assert(ret == VK_SUCCESS);

    void* data = nullptr;
    ret = indices_memory_->MapMemory(0, buffer_info.size, &data);
    assert(ret == VK_SUCCESS);
    memcpy(data, indices.data(), buffer_info.size);
    indices_memory_->UnmapMemory();
}

void DepthTriangle::DestroyIndexBuffer() {
    VulkanLogicDevice::FreeMemory(&indices_memory_);
    VulkanLogicDevice::DestroyBuffer(&indices_buffer_);
}

void DepthTriangle::CreateDepthImage() {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = frame_buffer_size_.width;
    imageInfo.extent.height = frame_buffer_size_.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_D32_SFLOAT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    depth_attachment_image_ = device_->CreateImage(&imageInfo);
    assert(depth_attachment_image_);

    VkMemoryRequirements mem_requirements;
    depth_attachment_image_->GetImageMemoryRequirements(&mem_requirements);

    VkPhysicalDeviceMemoryProperties mem_properties{};
    device_->GetPhysicalDeviceMemoryProperties(&mem_properties);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    VkResult ret = VulkanLogicDevice::GetMemoryType(&mem_properties, mem_requirements.memoryTypeBits,
                                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                    &alloc_info.memoryTypeIndex);
    assert(ret == VK_SUCCESS);
    depth_attachment_image_memory_ = device_->AllocateMemory(&alloc_info);
    depth_attachment_image_memory_->BindImageMemory(depth_attachment_image_->image(), 0);
//    TransitionImageLayout();
}

void DepthTriangle::DestroyDepthImage() {
    VulkanLogicDevice::FreeMemory(&depth_attachment_image_memory_);
    VulkanLogicDevice::DestroyImage(&depth_attachment_image_);
}

void DepthTriangle::CreateDepthImageView() {
    VkImageViewCreateInfo view_info{};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = depth_attachment_image_->image();
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = VK_FORMAT_D32_SFLOAT;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;
    depth_attachment_image_view_ = device_->CreateImageView(&view_info);
}

void DepthTriangle::DestroyDepthImageView() {
    VulkanLogicDevice::DestroyImageView(&depth_attachment_image_view_);
}

void DepthTriangle::TransitionImageLayout() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = command_pool_->command_pool();
    allocInfo.commandBufferCount = 1;
    VulkanCommandBuffer* command_buffer =
            command_pool_->AllocateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    command_buffer->BeginCommandBuffer(&begin_info);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = depth_attachment_image_->image();
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    command_buffer->CmdPipelineBarrier(
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

    command_buffer->EndCommandBuffer();
    VkCommandBuffer command_buffers[] = {command_buffer->command_buffer()};
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = command_buffers;
    graphic_queue_->QueueSubmit( 1, &submitInfo, VK_NULL_HANDLE);
    graphic_queue_->QueueWaitIdle();
    VulkanCommandPool::FreeCommandBuffer(&command_buffer);
}

VulkanPipelineLayout* DepthTriangle::CreatePipelineLayout() const {
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

VulkanPipeline* DepthTriangle::CreateGraphicsPipeline(const VulkanShaderModule* vert_shader_module,
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
    VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info = GetPipelineDepthStencilStateCreateInfo();
    pipeline_info.pDepthStencilState = &depth_stencil_state_create_info;
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

std::vector<VkPipelineShaderStageCreateInfo> DepthTriangle::GetPipelineShaderStageCreateInfos(
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

VkPipelineVertexInputStateCreateInfo DepthTriangle::GetPipelineVertexInputStateCreateInfo() const{
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

VkPipelineInputAssemblyStateCreateInfo DepthTriangle::GetPipelineInputAssemblyStateCreateInfo() {
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

VkPipelineViewportStateCreateInfo DepthTriangle::GetPipelineViewportStateCreateInfo() {
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

VkPipelineRasterizationStateCreateInfo DepthTriangle::GetPipelineRasterizationStateCreateInfo() {
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

VkPipelineMultisampleStateCreateInfo DepthTriangle::GetPipelineMultisampleStateCreateInfo() {
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

VkPipelineDepthStencilStateCreateInfo DepthTriangle::GetPipelineDepthStencilStateCreateInfo() {
    /* typedef struct VkPipelineDepthStencilStateCreateInfo {
        VkStructureType                           sType;
        const void*                               pNext;
        VkPipelineDepthStencilStateCreateFlags    flags;
        VkBool32                                  depthTestEnable;
        VkBool32                                  depthWriteEnable;
        VkCompareOp                               depthCompareOp;
        VkBool32                                  depthBoundsTestEnable;
        VkBool32                                  stencilTestEnable;
        VkStencilOpState                          front;
        VkStencilOpState                          back;
        float                                     minDepthBounds;
        float                                     maxDepthBounds;
    } VkPipelineDepthStencilStateCreateInfo; */
    VkPipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.minDepthBounds = 0.0f; // Optional
    depth_stencil.maxDepthBounds = 1.0f; // Optional
    depth_stencil.stencilTestEnable = VK_FALSE;
    depth_stencil.front = {}; // Optional
    depth_stencil.back = {}; // Optional

    return depth_stencil;
}

VkPipelineColorBlendStateCreateInfo DepthTriangle::GetPipelineColorBlendStateCreateInfo(
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

VkPipelineColorBlendAttachmentState DepthTriangle::GetPipelineColorBlendAttachmentState() {
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

VkPipelineDynamicStateCreateInfo DepthTriangle::GetPipelineDynamicStateCreateInfo(const std::vector<VkDynamicState>& dynamic_states) {
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