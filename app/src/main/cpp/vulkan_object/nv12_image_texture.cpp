//
// Created by hj6231 on 2024/1/3.
//

#include "nv12_image_texture.h"

#include "log.h"

const char Nv12ImageTexture::kVertShaderSource[] =
        "#version 450\n"
        "layout(location = 0) in vec2 positions;\n"
        "layout(location = 1) in vec2 coordinateIn;\n"
        "layout(location = 0) out vec2 coordinate;\n"
        "void main() {\n"
        "    gl_Position = vec4(positions, 0.0, 1.0);\n"
        "    coordinate = coordinateIn;\n"
        "}\n";

const char Nv12ImageTexture::kFragShaderSource[] =
        "#version 450\n"
        "layout(location = 0) in vec2 coordinate;\n"
        "layout(binding = 0) uniform sampler2D texSampler;\n"
        "layout(location = 0) out vec4 outColor;\n"
        "void main() {\n"
        "    outColor = texture(texSampler, coordinate);\n"
        "}\n";

Nv12ImageTexture::Nv12ImageTexture(AAssetManager* asset_manager,
                                   VulkanCommandPool* command_pool,
                                   VulkanQueue* graphic_queue,
                                   VulkanLogicDevice* device) :
        VulkanObject(device), asset_manager_(asset_manager),
        command_pool_(command_pool), graphic_queue_(graphic_queue){
    vertex_str_ = kVertShaderSource;
    fragment_str_ = kFragShaderSource;
}

int Nv12ImageTexture::CreatePipeline(const VulkanRenderPass* render_pass) {
    VulkanShaderModule* vert_shader_module;
    VulkanShaderModule* frag_shader_module;
    VulkanPipeline* pipeline;
    VulkanDescriptorSetLayout* descriptor_set_layout;
    VulkanSamplerYcbcrConversion* ycbcr_conversion = CreateSamplerYcbcrConversion();
    /* typedef struct VkSamplerYcbcrConversionInfo {
        VkStructureType             sType;
        const void*                 pNext;
        VkSamplerYcbcrConversion    conversion;
    } VkSamplerYcbcrConversionInfo; */
    VkSamplerYcbcrConversionInfo ycbcr_conversion_info{};
    ycbcr_conversion_info.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO;
    ycbcr_conversion_info.conversion = ycbcr_conversion->ycbcr_conversion();

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

    vulkan_descriptor_pool_ = CreateDescriptorPool();
    assert(vulkan_descriptor_pool_);

    CreateTextureImage();
    CreateTextureImageView(&ycbcr_conversion_info);
    CreateTextureSampler(&ycbcr_conversion_info);
    VulkanLogicDevice::DestroySamplerYcbcrConversion(&ycbcr_conversion);

    ////////////////////////////////////////////////////////
    descriptor_set_layout = CreateDescriptorSetLayout();
    assert(descriptor_set_layout);
    pipeline_layout_ = CreatePipelineLayout(descriptor_set_layout);
    assert(pipeline_layout_);
    vulkan_descriptor_set_ = CreateDescriptorSet(vulkan_descriptor_pool_, descriptor_set_layout);
    assert(vulkan_descriptor_set_);
    ////////////////////////////////////////////////////////

    pipeline = CreateGraphicsPipeline(vert_shader_module, frag_shader_module, pipeline_layout_, render_pass);
    assert(pipeline);
    if (pipeline == nullptr) {
        goto ERROR_EXIT;
    }
    pipeline_ = pipeline;

    ////////////////////////////////////////////////////////
    CreateVertexBuffer();
    CopyDataToVertexBuffer();
    BindDescriptorSetWithBuffer();

    VulkanLogicDevice::DestroyDescriptorSetLayout(&descriptor_set_layout);
    ////////////////////////////////////////////////////////
    VulkanLogicDevice::DestroyShaderModule(&vert_shader_module);
    VulkanLogicDevice::DestroyShaderModule(&frag_shader_module);
    LOG_D("", "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
    return 0;
    ERROR_EXIT:
    DestroyPipeline();
    return -1;
}

void Nv12ImageTexture::DestroyPipeline() {
    LOG_D("", "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA 11111111111111111111111\n");
    VulkanLogicDevice::DestroySampler(&texture_image_sampler_);
    VulkanLogicDevice::DestroyImageView(&texture_image_view_);
    VulkanLogicDevice::FreeMemory(&texture_image_memory_);
    VulkanLogicDevice::DestroyImage(&texture_image_);
    DestroyVertexBuffer();
    if (pipeline_ != nullptr) {
        VulkanLogicDevice::DestroyPipelines(&pipeline_);
    }

    vulkan_descriptor_pool_->FreeDescriptorSet(&vulkan_descriptor_set_);
    VulkanLogicDevice::DestroyDescriptorPool(&vulkan_descriptor_pool_);
    VulkanLogicDevice::DestroyPipelineLayout(&pipeline_layout_);
}

void Nv12ImageTexture::Draw(const VulkanCommandBuffer* command_buffer,
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
    VkDescriptorSet descriptor_sets[] = {vulkan_descriptor_set_->descriptor_set()};
    command_buffer->CmdBindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_->layout(), 0, 1, descriptor_sets, 0, nullptr);
    command_buffer->CmdDraw(4, 1, 0, 0);
    command_buffer->CmdEndRenderPass();
    command_buffer->EndCommandBuffer();
}

void Nv12ImageTexture::CreateVertexBuffer() {
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

void Nv12ImageTexture::CopyDataToVertexBuffer() {
    void* data;
    size_t size =  sizeof (vertices_[0]) * vertices_.size();
    vertex_memory_->MapMemory(0, size, &data);
    memcpy(data, vertices_.data(), size);
    vertex_memory_->UnmapMemory();
}

void Nv12ImageTexture::DestroyVertexBuffer() {
    VulkanLogicDevice::FreeMemory(&vertex_memory_);
    VulkanLogicDevice::DestroyBuffer(&vertex_buffer_);
}


void Nv12ImageTexture::CreateTextureImage() {
    AAsset* file = AAssetManager_open(asset_manager_,
                                      "texture_512x512.NV12", AASSET_MODE_BUFFER);
    size_t file_length = AAsset_getLength(file);
    char* file_content = new char[file_length];
    AAsset_read(file, file_content, file_length);
    AAsset_close(file);

    int tex_width = 512, tex_height = 512;
    VkDeviceSize image_size = tex_width * tex_height * 3 / 2;

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
    memcpy(data, file_content, image_size);
    staging_buffer_memory->UnmapMemory();

    CreateImage(tex_width, tex_height, VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture_image_, texture_image_memory_);

    TransitionImageLayout(texture_image_->image(), VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
                          VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    CopyBufferToImage(staging_buffer->buffer(), texture_image_->image(),
                      static_cast<uint32_t>(tex_width), static_cast<uint32_t>(tex_height));
    TransitionImageLayout(texture_image_->image(), VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    VulkanLogicDevice::DestroyBuffer(&staging_buffer);
    VulkanLogicDevice::FreeMemory(&staging_buffer_memory);
}

void Nv12ImageTexture::CreateTextureImageView(const VkSamplerYcbcrConversionInfo* ycbcr_conversion_info) {
    VkImageViewCreateInfo view_info{};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.pNext = ycbcr_conversion_info;
    view_info.image = texture_image_->image();
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;
    texture_image_view_ = device_->CreateImageView(&view_info);
}

VulkanSamplerYcbcrConversion* Nv12ImageTexture::CreateSamplerYcbcrConversion() {
     /* typedef struct VkSamplerYcbcrConversionCreateInfo {
        VkStructureType                  sType;
        const void*                      pNext;
        VkFormat                         format;
        VkSamplerYcbcrModelConversion    ycbcrModel;
        VkSamplerYcbcrRange              ycbcrRange;
        VkComponentMapping               components;
        VkChromaLocation                 xChromaOffset;
        VkChromaLocation                 yChromaOffset;
        VkFilter                         chromaFilter;
        VkBool32                         forceExplicitReconstruction;
    } VkSamplerYcbcrConversionCreateInfo; */
    VkSamplerYcbcrConversionCreateInfo   create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
    create_info.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    create_info.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020 ;
    create_info.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
//    // NV21
//    create_info.components = {
//            VK_COMPONENT_SWIZZLE_B,
//            VK_COMPONENT_SWIZZLE_IDENTITY,
//            VK_COMPONENT_SWIZZLE_R,
//            VK_COMPONENT_SWIZZLE_IDENTITY,
//    };
    create_info.components = {
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
    };
    create_info.xChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;
    create_info.yChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;
    create_info.chromaFilter = VK_FILTER_LINEAR;
    create_info.forceExplicitReconstruction = VK_FALSE;
    return device_->CreateSamplerYcbcrConversion(&create_info);
}

void Nv12ImageTexture::CreateTextureSampler(const VkSamplerYcbcrConversionInfo* ycbcr_conversion_info) {
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
    sampler_info.pNext = ycbcr_conversion_info;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.anisotropyEnable = VK_FALSE;
    sampler_info.maxAnisotropy = 0;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    texture_image_sampler_ = device_->CreateSampler(&sampler_info);
    assert(texture_image_sampler_);
}

void Nv12ImageTexture::CreateImage(uint32_t width, uint32_t height, VkFormat format,
                                   VkImageTiling tiling, VkImageUsageFlags usage,
                                   VkMemoryPropertyFlags properties, VulkanImage*& image,
                                   VulkanMemory*& image_memory) {
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

void Nv12ImageTexture::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
                                             VkImageLayout newLayout) {
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
    command_pool_->FreeCommandBuffer(&command_buffer);
}

void Nv12ImageTexture::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
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

    /* typedef struct VkBufferImageCopy {
        VkDeviceSize                bufferOffset;
        uint32_t                    bufferRowLength;
        uint32_t                    bufferImageHeight;
        VkImageSubresourceLayers    imageSubresource;
        VkOffset3D                  imageOffset;
        VkExtent3D                  imageExtent;
    } VkBufferImageCopy; */
    VkBufferImageCopy regions[2];
    regions[0].bufferOffset = 0;
    regions[0].bufferRowLength = 0;
    regions[0].bufferImageHeight = 0;
    regions[0].imageSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT ;
    regions[0].imageSubresource.mipLevel = 0;
    regions[0].imageSubresource.baseArrayLayer = 0;
    regions[0].imageSubresource.layerCount = 1;
    regions[0].imageOffset = {0, 0, 0};
    regions[0].imageExtent = {
            width,
            height,
            1
    };
    regions[1].bufferOffset = 512*512;
    regions[1].bufferRowLength = 0;
    regions[1].bufferImageHeight = 0;
    regions[1].imageSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
    regions[1].imageSubresource.mipLevel = 0;
    regions[1].imageSubresource.baseArrayLayer = 0;
    regions[1].imageSubresource.layerCount = 1;
    regions[1].imageOffset = {0, 0, 0};
    regions[1].imageExtent = {
            width / 2,
            height / 2,
            1
    };
    command_buffer->CmdCopyBufferToImage(buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 2, regions);

    command_buffer->EndCommandBuffer();
    VkCommandBuffer command_buffers[] = {command_buffer->command_buffer()};
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = command_buffers;

    graphic_queue_->QueueSubmit( 1, &submitInfo, VK_NULL_HANDLE);
    graphic_queue_->QueueWaitIdle();
    command_pool_->FreeCommandBuffer(&command_buffer);
}

VulkanDescriptorPool*  Nv12ImageTexture::CreateDescriptorPool() {
    /* typedef struct VkDescriptorPoolSize {
        VkDescriptorType    type;
        uint32_t            descriptorCount;
    } VkDescriptorPoolSize; */
    VkDescriptorPoolSize pool_size{};
    pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_size.descriptorCount = 1;
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
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = &pool_size;
    return device_->CreateDescriptorPool(&pool_info);
}

VulkanDescriptorSetLayout* Nv12ImageTexture::CreateDescriptorSetLayout() const {
    /* typedef struct VkDescriptorSetLayoutBinding {
        uint32_t              binding;
        VkDescriptorType      descriptorType;
        uint32_t              descriptorCount;
        VkShaderStageFlags    stageFlags;
        const VkSampler*      pImmutableSamplers;
    } VkDescriptorSetLayoutBinding; */
    VkDescriptorSetLayoutBinding layout_binding{};
    layout_binding.binding = 0;
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    VkSampler samplers[] = {texture_image_sampler_->sampler()};
    layout_binding.pImmutableSamplers = samplers;

    /* typedef struct VkDescriptorSetLayoutCreateInfo {
        VkStructureType                        sType;
        const void*                            pNext;
        VkDescriptorSetLayoutCreateFlags       flags;
        uint32_t                               bindingCount;
        const VkDescriptorSetLayoutBinding*    pBindings;
    } VkDescriptorSetLayoutCreateInfo; */
    VkDescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = 1;
    layout_info.pBindings = &layout_binding;
    return device_->CreateDescriptorSetLayout(&layout_info);
}

VulkanDescriptorSet* Nv12ImageTexture::CreateDescriptorSet(VulkanDescriptorPool* pool, VulkanDescriptorSetLayout* set_layout) {
    VkDescriptorSetLayout set_layout1 = set_layout->descriptor_set_layout();
    return pool->AllocateDescriptorSet(&set_layout1);
}

void Nv12ImageTexture::BindDescriptorSetWithBuffer() const {
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
    descriptor_write.dstSet = vulkan_descriptor_set_->descriptor_set();
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.pImageInfo = &image_info; // Optional
    descriptor_write.pBufferInfo = nullptr;
    descriptor_write.pTexelBufferView = nullptr; // Optional
    device_->UpdateDescriptorSets(1, &descriptor_write);
}

VulkanPipelineLayout* Nv12ImageTexture::CreatePipelineLayout(const VulkanDescriptorSetLayout* descriptor_set_layout) const {
    /*typedef struct VkPipelineLayoutCreateInfo {
        VkStructureType                 sType;
        const void*                     pNext;
        VkPipelineLayoutCreateFlags     flags;
        uint32_t                        setLayoutCount;
        const VkDescriptorSetLayout*    pSetLayouts;
        uint32_t                        pushConstantRangeCount;
        const VkPushConstantRange*      pPushConstantRanges;
    } VkPipelineLayoutCreateInfo; */
    VkDescriptorSetLayout descriptor_set_layouts[] = {descriptor_set_layout->descriptor_set_layout()};
    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1; // Optional
    pipeline_layout_info.pSetLayouts = descriptor_set_layouts; // Optional
    pipeline_layout_info.pushConstantRangeCount = 0; // Optional
    pipeline_layout_info.pPushConstantRanges = nullptr; // Optional
    return device_->CreatePipelineLayout(&pipeline_layout_info);
}

VulkanPipeline* Nv12ImageTexture::CreateGraphicsPipeline(
        const VulkanShaderModule* vert_shader_module,
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

std::vector<VkPipelineShaderStageCreateInfo> Nv12ImageTexture::GetPipelineShaderStageCreateInfos(
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

VkPipelineVertexInputStateCreateInfo Nv12ImageTexture::GetPipelineVertexInputStateCreateInfo() const {
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
    vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_attribute_descriptions_.size());;
    vertex_input_info.pVertexAttributeDescriptions = vertex_attribute_descriptions_.data();
    return vertex_input_info;
}

VkPipelineInputAssemblyStateCreateInfo Nv12ImageTexture::GetPipelineInputAssemblyStateCreateInfo() {
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

VkPipelineViewportStateCreateInfo Nv12ImageTexture::GetPipelineViewportStateCreateInfo() {
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

VkPipelineRasterizationStateCreateInfo Nv12ImageTexture::GetPipelineRasterizationStateCreateInfo() {
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

VkPipelineMultisampleStateCreateInfo Nv12ImageTexture::GetPipelineMultisampleStateCreateInfo() {
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

VkPipelineColorBlendStateCreateInfo Nv12ImageTexture::GetPipelineColorBlendStateCreateInfo(
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

VkPipelineColorBlendAttachmentState Nv12ImageTexture::GetPipelineColorBlendAttachmentState() {
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

VkPipelineDynamicStateCreateInfo Nv12ImageTexture::GetPipelineDynamicStateCreateInfo(const std::vector<VkDynamicState>& dynamic_states) {
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