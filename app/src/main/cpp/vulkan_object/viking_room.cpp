//
// Created by hj6231 on 2024/1/15.
//

#include "viking_room.h"
#include "log.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <sstream>
#include <map>
#include <unordered_map>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>

const char VikingRoom::kVertShaderSource[] =
        "#version 450\n"
        "layout(binding = 0) uniform UniformBufferObject {\n"
        "    mat4 model;\n"
        "    mat4 view;\n"
        "    mat4 proj;\n"
        "} ubo;"
        "layout(location = 0) in vec3 pos;\n"
        "layout(location = 1) in vec2 coordinate;\n"
        "layout(location = 0) out vec2 fragCoordinate;\n"
        "void main() {\n"
        "    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(pos, 1.0);\n"
        "    fragCoordinate = coordinate;\n"
        "}\n";

const char VikingRoom::kFragShaderSource[] =
        "#version 450\n"
        "layout(binding = 1) uniform sampler2D texSampler;\n"
        "layout(location = 0) in vec2 fragCoordinate;\n"
        "layout(location = 0) out vec4 outColor;\n"
        "void main() {\n"
        "    outColor = texture(texSampler, fragCoordinate);\n"
        "}\n";

VikingRoom::VikingRoom(AAssetManager* asset_manager,
                       VulkanCommandPool* command_pool,
                       VulkanQueue* graphic_queue,
                       VulkanLogicDevice* device,
                       VkFormat swap_chain_image_format,
                       VkExtent2D frame_buffer_size) :
        VulkanObject(device, swap_chain_image_format, frame_buffer_size),
        asset_manager_(asset_manager),
        command_pool_(command_pool),
        graphic_queue_(graphic_queue),
        pipeline_layout_(nullptr),
        vertex_buffer_(nullptr),
        vertex_memory_(nullptr),
        indices_buffer_(nullptr),
        indices_memory_(nullptr),
        mvp_buffer_(nullptr),
        mvp_memory_(nullptr) ,
        uniform_buffer_mapped_(nullptr) ,
        texture_image_(nullptr) ,
        texture_image_memory_(nullptr) ,
        texture_image_view_(nullptr) ,
        texture_image_sampler_(nullptr) ,
        descriptor_pool_(nullptr) ,
        descriptor_set_layout_(nullptr) ,
        descriptor_set_(nullptr) {
    vertex_str_ = kVertShaderSource;
    fragment_str_ = kFragShaderSource;
}

int VikingRoom::CreatePipeline() {
    VulkanShaderModule* vert_shader_module;
    VulkanShaderModule* frag_shader_module;
    VulkanPipeline* pipeline;
    std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
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
    CreateDescriptorSets();
    CreateRenderPass();
    CreateVertexBuffer();
    CreateIndexBuffer();

    descriptor_set_layouts.push_back(descriptor_set_layout_->descriptor_set_layout());
    pipeline_layout_ = CreatePipelineLayout(descriptor_set_layouts);
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

void VikingRoom::DestroyPipeline() {
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
    DestroyTextureImageViewAndSampler();
    DestroyTextureImage();
    DestroyMvpBuffer();
}

void VikingRoom::Draw(const VulkanCommandBuffer* command_buffer,
                      const VulkanFrameBuffer* frame_buffer) const {
    CopyDataToUniformBuffer();
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

//    command_buffer->CmdBindIndexBuffer(indices_buffer_->buffer(), 0, VK_INDEX_TYPE_UINT16);
    VkBuffer vertexes[] = {vertex_buffer_->buffer()};
    VkDeviceSize offsets[] = {0};
    command_buffer->CmdBindVertexBuffers(0, 1, vertexes, offsets);
    VkDescriptorSet descriptor_sets[] = {descriptor_set_->descriptor_set()};
    command_buffer->CmdBindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_->layout(), 0, 1, descriptor_sets, 0, nullptr);
//    command_buffer->CmdDrawIndexed(static_cast<uint32_t>(indices_.size()), 1, 0, 0, 0);
    command_buffer->CmdDraw(static_cast<uint32_t>(vertices_.size()), 1, 0, 0);
    command_buffer->CmdEndRenderPass();
    command_buffer->EndCommandBuffer();
}

void VikingRoom::LoadResource() {
    ReadVerticesIndexes();
    CreateMvpBuffer();
    CreateTextureImage();
    CreateTextureImageViewAndSampler();
    CreateDepthImage();
    CreateDepthImageView();
}

void VikingRoom::CreateRenderPass() {
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

void VikingRoom::CreateVertexBuffer() {
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

void VikingRoom::DestroyVertexBuffer() {
    VulkanLogicDevice::FreeMemory(&vertex_memory_);
    VulkanLogicDevice::DestroyBuffer(&vertex_buffer_);
}

void VikingRoom::CreateIndexBuffer() {
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
    buffer_info.size = sizeof(indices_[0]) * indices_.size();
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
    memcpy(data, indices_.data(), buffer_info.size);
    indices_memory_->UnmapMemory();
}

void VikingRoom::DestroyIndexBuffer() {
    VulkanLogicDevice::FreeMemory(&indices_memory_);
    VulkanLogicDevice::DestroyBuffer(&indices_buffer_);
}

void VikingRoom::CreateDescriptorSets() {
    descriptor_pool_ = CreateDescriptorPool();
    assert(descriptor_pool_);
    descriptor_set_layout_ = CreateDescriptorSetLayout();
    assert(descriptor_set_layout_);

    VkDescriptorSetLayout set_layout1 = descriptor_set_layout_->descriptor_set_layout();
    descriptor_set_ = descriptor_pool_->AllocateDescriptorSet(&set_layout1);
    assert(descriptor_set_);
    BindMvpDescriptorSetWithBuffer();
    BindTextureDescriptorSetWithImage();
}

void VikingRoom::ReadVerticesIndexes() {
    AAsset* file = AAssetManager_open(asset_manager_,
                                      "viking_room.obj.txt", AASSET_MODE_BUFFER);
    assert(file);
    size_t file_length = AAsset_getLength(file);
    char* file_content = new char[file_length + 1];
    memset(file_content, 0, file_length + 1);
    int len = AAsset_read(file, file_content, file_length);
    AAsset_close(file);
    std::stringstream sstr(file_content);
    delete[] file_content;
    LOG_D("", "file_length %d len %d\n", file_length, len);
    LOG_D("", "sstr %d \n", sstr.str().length());

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
    bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, &sstr);
    assert(success);
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            vertex_t vertex{};

            vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.coordinate = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };
            vertices_.push_back(vertex);
//            indices_.push_back(static_cast<uint32_t>(vertices_.size()));
        }
    }
    LOG_D("", "vertices_ %d \n", vertices_.size());
}

void VikingRoom::CreateMvpBuffer() {
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
    buffer_info.size = sizeof(mvp_t);
    buffer_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    mvp_buffer_ = device_->CreateBuffer(&buffer_info);
    assert(mvp_buffer_);
    VkMemoryRequirements mem_requirements{};
    mvp_buffer_->GetBufferMemoryRequirements(&mem_requirements);
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
    mvp_memory_ = device_->AllocateMemory(&alloc_info);
    assert(mvp_memory_);
    ret = mvp_memory_->BindBufferMemory(mvp_buffer_->buffer(), 0);
    assert(ret == VK_SUCCESS);

    ret = mvp_memory_->MapMemory(0, buffer_info.size, &uniform_buffer_mapped_);
    assert(ret == VK_SUCCESS);
}

void VikingRoom::DestroyMvpBuffer() {
    mvp_memory_->UnmapMemory();
    VulkanLogicDevice::FreeMemory(&mvp_memory_);
    VulkanLogicDevice::DestroyBuffer(&mvp_buffer_);
}

void VikingRoom::CreateTextureImage() {
    AAsset* file = AAssetManager_open(asset_manager_,
                                      "viking_room.png", AASSET_MODE_BUFFER);
    size_t file_length = AAsset_getLength(file);
    char* file_content = new char[file_length];
    AAsset_read(file, file_content, file_length);
    AAsset_close(file);

    int tex_width, tex_height, tex_channels;
    stbi_uc* pixels = stbi_load_from_memory((stbi_uc*)file_content, file_length, &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    delete[] file_content;
    VkDeviceSize image_size = tex_width * tex_height * 4;

    VulkanBuffer* staging_buffer;
    VulkanMemory* staging_buffer_memory;

    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = image_size;
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    staging_buffer = device_->CreateBuffer(&buffer_info);
    assert(staging_buffer);
    VkMemoryRequirements mem_requirements{};
    staging_buffer->GetBufferMemoryRequirements(&mem_requirements);
    VkPhysicalDeviceMemoryProperties mem_properties{};
    device_->GetPhysicalDeviceMemoryProperties(&mem_properties);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    VkResult ret = VulkanLogicDevice::GetMemoryType(&mem_properties, mem_requirements.memoryTypeBits,
                                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                                    &alloc_info.memoryTypeIndex);
    assert(ret == VK_SUCCESS);
    staging_buffer_memory = device_->AllocateMemory(&alloc_info);
    assert(staging_buffer_memory);
    staging_buffer_memory->BindBufferMemory(staging_buffer->buffer(), 0);

    void* data;
    staging_buffer_memory->MapMemory(0, mem_requirements.size, &data);
    memcpy(data, pixels, image_size);
    staging_buffer_memory->UnmapMemory();
    stbi_image_free(pixels);

    CreateImage(tex_width, tex_height, VK_FORMAT_R8G8B8A8_UNORM,
                VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                texture_image_, texture_image_memory_);

    TransitionImageLayout(texture_image_->image(),
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    CopyBufferToImage(staging_buffer->buffer(), texture_image_->image(),
                      static_cast<uint32_t>(tex_width), static_cast<uint32_t>(tex_height));
    TransitionImageLayout(texture_image_->image(),
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    VulkanLogicDevice::DestroyBuffer(&staging_buffer);
    VulkanLogicDevice::FreeMemory(&staging_buffer_memory);
}

void VikingRoom::DestroyTextureImage() {
    VulkanLogicDevice::DestroyImage(&texture_image_);
    VulkanLogicDevice::FreeMemory(&texture_image_memory_);
}

void VikingRoom::CreateTextureImageViewAndSampler() {
    VkImageViewCreateInfo view_info{};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = texture_image_->image();
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;
    texture_image_view_ = device_->CreateImageView(&view_info);
    /* typedef struct VkSamplerCreateInfo {
        VkStructureType         sType;
        const void*             pNext;
        VkSamplerCreateFlags    flags;
        VkFilter                magFilter;
        VkFilter                minFilter;
        VkSamplerMipmapMode     mipmapMode;
        VkSamplerAddressMode    addressModeU;
        VkSamplerAddressMode    addressModeV;
        VkSamplerAddressMode    addressModeW;
        float                   mipLodBias;
        VkBool32                anisotropyEnable;
        float                   maxAnisotropy;
        VkBool32                compareEnable;
        VkCompareOp             compareOp;
        float                   minLod;
        float                   maxLod;
        VkBorderColor           borderColor;
        VkBool32                unnormalizedCoordinates;
    } VkSamplerCreateInfo; */
    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.anisotropyEnable = VK_FALSE;
    sampler_info.maxAnisotropy = 0;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    texture_image_sampler_ = device_->CreateSampler(&sampler_info);
    assert(texture_image_sampler_);
}

void VikingRoom::DestroyTextureImageViewAndSampler() {
    VulkanLogicDevice::DestroySampler(&texture_image_sampler_);
    VulkanLogicDevice::DestroyImageView(&texture_image_view_);
}

void VikingRoom::CreateDepthImage() {
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
}

void VikingRoom::DestroyDepthImage() {
    VulkanLogicDevice::FreeMemory(&depth_attachment_image_memory_);
    VulkanLogicDevice::DestroyImage(&depth_attachment_image_);
}

void VikingRoom::CreateDepthImageView() {
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

void VikingRoom::DestroyDepthImageView() {
    VulkanLogicDevice::DestroyImageView(&depth_attachment_image_view_);
}

VulkanPipelineLayout* VikingRoom::CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& set_layouts) const {
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
    pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(set_layouts.size()); // Optional
    pipeline_layout_info.pSetLayouts = set_layouts.data(); // Optional
    pipeline_layout_info.pushConstantRangeCount = 0; // Optional
    pipeline_layout_info.pPushConstantRanges = nullptr; // Optional
    return device_->CreatePipelineLayout(&pipeline_layout_info);
}

VulkanPipeline* VikingRoom::CreateGraphicsPipeline(const VulkanShaderModule* vert_shader_module,
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

void VikingRoom::CreateImage(uint32_t width, uint32_t height, VkFormat format,
                             VkImageTiling tiling, VkImageUsageFlags usage,
                             VulkanImage*& image, VulkanMemory*& image_memory) const {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image = device_->CreateImage(&imageInfo);
    assert(image);

    VkMemoryRequirements mem_requirements;
    image->GetImageMemoryRequirements(&mem_requirements);

    VkPhysicalDeviceMemoryProperties mem_properties{};
    device_->GetPhysicalDeviceMemoryProperties(&mem_properties);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    VkResult ret = VulkanLogicDevice::GetMemoryType(&mem_properties, mem_requirements.memoryTypeBits,
                                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                     &alloc_info.memoryTypeIndex);
    assert(ret == VK_SUCCESS);
    image_memory = device_->AllocateMemory(&alloc_info);
    image_memory->BindImageMemory(image->image(), 0);
}

void VikingRoom::TransitionImageLayout(VkImage image, VkImageLayout oldLayout,
                                       VkImageLayout newLayout) const {
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
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    command_buffer->CmdPipelineBarrier(
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
    );

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

void VikingRoom::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = command_pool_->command_pool();
    allocInfo.commandBufferCount = 1;
    VulkanCommandBuffer* command_buffer =
            command_pool_->AllocateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    /*  typedef struct VkCommandBufferBeginInfo {
        VkStructureType                          sType;
        const void*                              pNext;
        VkCommandBufferUsageFlags                flags;
        const VkCommandBufferInheritanceInfo*    pInheritanceInfo;
    } VkCommandBufferBeginInfo; */
    VkCommandBufferBeginInfo command_buffer_begin_info{};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VkResult ret = command_buffer->BeginCommandBuffer(&command_buffer_begin_info);
    assert(ret == VK_SUCCESS);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
            width,
            height,
            1
    };
    command_buffer->CmdCopyBufferToImage(buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

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

VulkanDescriptorPool*  VikingRoom::CreateDescriptorPool() const {
    /* typedef struct VkDescriptorPoolSize {
        VkDescriptorType    type;
        uint32_t            descriptorCount;
    } VkDescriptorPoolSize; */
    VkDescriptorPoolSize pool_sizes[2];
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = 1;
    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[1].descriptorCount = 1;
    /* typedef struct VkDescriptorPoolCreateInfo {
        VkStructureType                sType;
        const void*                    pNext;
        VkDescriptorPoolCreateFlags    flags;
        uint32_t                       maxSets;
        uint32_t                       poolSizeCount;
        const VkDescriptorPoolSize*    pPoolSizes;
    } VkDescriptorPoolCreateInfo; */
    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = 2;
    pool_info.pPoolSizes = pool_sizes;
    return device_->CreateDescriptorPool(&pool_info);
}

VulkanDescriptorSetLayout* VikingRoom::CreateDescriptorSetLayout() const {
    /* typedef struct VkDescriptorSetLayoutBinding {
        uint32_t              binding;
        VkDescriptorType      descriptorType;
        uint32_t              descriptorCount;
        VkShaderStageFlags    stageFlags;
        const VkSampler*      pImmutableSamplers;
    } VkDescriptorSetLayoutBinding; */
    std::array<VkDescriptorSetLayoutBinding, 2> bindings{};
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    /* typedef struct VkDescriptorSetLayoutCreateInfo {
        VkStructureType                        sType;
        const void*                            pNext;
        VkDescriptorSetLayoutCreateFlags       flags;
        uint32_t                               bindingCount;
        const VkDescriptorSetLayoutBinding*    pBindings;
    } VkDescriptorSetLayoutCreateInfo; */
    VkDescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
    layout_info.pBindings = bindings.data();
    return device_->CreateDescriptorSetLayout(&layout_info);
}

void VikingRoom::BindTextureDescriptorSetWithImage() const {
    /* typedef struct VkDescriptorImageInfo {
        VkSampler        sampler;
        VkImageView      imageView;
        VkImageLayout    imageLayout;
    } VkDescriptorImageInfo; */
    VkDescriptorImageInfo  image_info{};
    image_info.sampler = texture_image_sampler_->sampler();
    image_info.imageView = texture_image_view_->image_view();
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    /* typedef struct VkWriteDescriptorSet {
        VkStructureType                  sType;
        const void*                      pNext;
        VkDescriptorSet                  dstSet;
        uint32_t                         dstBinding;
        uint32_t                         dstArrayElement;
        uint32_t                         descriptorCount;
        VkDescriptorType                 descriptorType;
        const VkDescriptorImageInfo*     pImageInfo;
        const VkDescriptorBufferInfo*    pBufferInfo;
        const VkBufferView*              pTexelBufferView;
    } VkWriteDescriptorSet; */
    VkWriteDescriptorSet descriptor_write{};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = descriptor_set_->descriptor_set();
    descriptor_write.dstBinding = 1;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.pImageInfo = &image_info; // Optional
    descriptor_write.pBufferInfo = nullptr;
    descriptor_write.pTexelBufferView = nullptr; // Optional
    device_->UpdateDescriptorSets(1, &descriptor_write);
}

void VikingRoom::BindMvpDescriptorSetWithBuffer() const {
    /* typedef struct VkDescriptorBufferInfo {
        VkBuffer        buffer;
        VkDeviceSize    offset;
        VkDeviceSize    range;
    } VkDescriptorBufferInfo; */
    VkDescriptorBufferInfo  buffer_info{};
    buffer_info.buffer = mvp_buffer_->buffer();
    buffer_info.offset = 0;
    buffer_info.range = sizeof(mvp_t);
    /* typedef struct VkWriteDescriptorSet {
        VkStructureType                  sType;
        const void*                      pNext;
        VkDescriptorSet                  dstSet;
        uint32_t                         dstBinding;
        uint32_t                         dstArrayElement;
        uint32_t                         descriptorCount;
        VkDescriptorType                 descriptorType;
        const VkDescriptorImageInfo*     pImageInfo;
        const VkDescriptorBufferInfo*    pBufferInfo;
        const VkBufferView*              pTexelBufferView;
    } VkWriteDescriptorSet; */
    VkWriteDescriptorSet descriptor_write{};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = descriptor_set_->descriptor_set();
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.pImageInfo = nullptr; // Optional
    descriptor_write.pBufferInfo = &buffer_info;
    descriptor_write.pTexelBufferView = nullptr; // Optional
    device_->UpdateDescriptorSets(1, &descriptor_write);
}

void VikingRoom::CopyDataToUniformBuffer() const {
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    mvp_t ubo;
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(10.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.project = glm::perspective(glm::radians(45.0f), (float)frame_buffer_size_.width/(float)frame_buffer_size_.height, 0.1f, 10.0f);
    ubo.project[1][1] *= -1;
    memcpy(uniform_buffer_mapped_, &ubo, sizeof(ubo));
}

std::vector<VkPipelineShaderStageCreateInfo> VikingRoom::GetPipelineShaderStageCreateInfos(
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

VkPipelineVertexInputStateCreateInfo VikingRoom::GetPipelineVertexInputStateCreateInfo() const{
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

VkPipelineInputAssemblyStateCreateInfo VikingRoom::GetPipelineInputAssemblyStateCreateInfo() {
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

VkPipelineViewportStateCreateInfo VikingRoom::GetPipelineViewportStateCreateInfo() {
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

VkPipelineRasterizationStateCreateInfo VikingRoom::GetPipelineRasterizationStateCreateInfo() {
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
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
    rasterizer.lineWidth = 1.0f;
    return rasterizer;
}

VkPipelineMultisampleStateCreateInfo VikingRoom::GetPipelineMultisampleStateCreateInfo() {
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

VkPipelineDepthStencilStateCreateInfo VikingRoom::GetPipelineDepthStencilStateCreateInfo() {
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

VkPipelineColorBlendStateCreateInfo VikingRoom::GetPipelineColorBlendStateCreateInfo(
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

VkPipelineColorBlendAttachmentState VikingRoom::GetPipelineColorBlendAttachmentState() {
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

VkPipelineDynamicStateCreateInfo VikingRoom::GetPipelineDynamicStateCreateInfo(const std::vector<VkDynamicState>& dynamic_states) {
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