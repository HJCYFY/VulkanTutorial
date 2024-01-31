//
// Created by hj6231 on 2024/1/25.
//

#include "particle.h"

#include <cassert>
#include <array>
#include <algorithm>
#include <random>

static const char kComputeShaderSource[] =
        "#version 450\n"
        "struct Particle {\n"
        "    vec2 position;\n"
        "    vec2 velocity;\n"
        "    vec4 color;\n"
        "};\n"
        "layout (binding = 0) uniform ParameterUBO {\n"
        "        float deltaTime;\n"
        "} ubo;\n"
        "layout(std140, binding = 1) readonly buffer ParticleSSBOIn {\n"
        "    Particle particlesIn[ ];\n"
        "};\n"
        "layout(std140, binding = 2) buffer ParticleSSBOOut {\n"
        "        Particle particlesOut[ ];\n"
        "};\n"
        "layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;\n"
        "void main() {\n"
        "    uint index = gl_GlobalInvocationID.x;\n"
        "    Particle particleIn = particlesIn[index];\n"
        "    particlesOut[index].position = particleIn.position + particleIn.velocity.xy * ubo.deltaTime;\n"
        "    particlesOut[index].velocity = particleIn.velocity;\n"
        "    if ((particlesOut[index].position.x <= -1.0) || (particlesOut[index].position.x >= 1.0)) {\n"
        "        particlesOut[index].velocity.x = -particlesOut[index].velocity.x;\n"
        "    }\n"
        "    if ((particlesOut[index].position.y <= -1.0) || (particlesOut[index].position.y >= 1.0)) {\n"
        "        particlesOut[index].velocity.y = -particlesOut[index].velocity.y;\n"
        "    }\n"
        "}\n";

Particle::Particle(VulkanLogicDevice* device,
                   VkFormat swap_chain_image_format,
                   VkExtent2D frame_buffer_size) :
        device_(device),
        swap_chain_image_format_(swap_chain_image_format),
        frame_buffer_size_(frame_buffer_size) {
}

int Particle::CreatePipeline() {
    VulkanShaderModule* computer_shader = CreateShaderModule("ComputeShaderSrc",
                                         shaderc_glsl_compute_shader,
                                         kComputeShaderSource);
    assert(computer_shader);

    LoadResource();

    CreateDescriptorPool();
    CreateDescriptorSetLayout();
    CreateDescriptorSet();
    UpdateDescriptorSet();
    CreatePipelineLayout();

    VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = computer_shader->shader_module(),
        .pName = "main",
        .pSpecializationInfo = nullptr
    };

    VkComputePipelineCreateInfo computePipelineCreateInfo {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = pipelineShaderStageCreateInfo,
        .layout = pipeline_layout_->layout(),
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };
    pipeline_ = device_->CreateComputePipeline(&computePipelineCreateInfo);
    assert(pipeline_);
    VulkanLogicDevice::DestroyShaderModule(&computer_shader);
    return 0;
}

void Particle::DestroyPipeline() {
    VulkanLogicDevice::DestroyPipelineLayout(&pipeline_layout_);
    VulkanLogicDevice::DestroyPipelines(&pipeline_);
    DestroyStorageBuffer();
    DestroyUinformBuffer();
}

void Particle::Draw(const VulkanCommandBuffer* command_buffer) const {
    CopyDataToUinformBuffer();

    VkCommandBufferBeginInfo commandBufferBeginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pInheritanceInfo = nullptr
    };
    VkResult ret = command_buffer->BeginCommandBuffer(&commandBufferBeginInfo);
    assert(ret == VK_SUCCESS);
    command_buffer->CmdBindPipeline(VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_->pipeline());
    VkDescriptorSet descriptorSets[] = {descriptor_set_->descriptor_set()};
    command_buffer->CmdBindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout_->layout(), 0, 1,
                                          descriptorSets, 0, nullptr);
    command_buffer->CmdDispatch(PARTICLE_COUNT / 256, 1, 1);
//    command_buffer->EndCommandBuffer();
}

VkBuffer Particle::GetVertexBuffer() const {
    return storage_buffer_->buffer();
}

void Particle::LoadResource() {
    CreateUinformBuffer();
    CreateStorageBuffer();
}

void Particle::CreateUinformBuffer() {
    VkBufferCreateInfo bufferCreateInfo {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = sizeof(delta_time_t),
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr
    };
    uniform_buffer_ = device_->CreateBuffer(&bufferCreateInfo);
    assert(uniform_buffer_);
    VkMemoryRequirements mem_requirements{};
    uniform_buffer_->GetBufferMemoryRequirements(&mem_requirements);
    VkPhysicalDeviceMemoryProperties mem_properties{};
    device_->GetPhysicalDeviceMemoryProperties(&mem_properties);

    uint32_t memoryTypeIndex = 0;
    VkResult ret = VulkanLogicDevice::GetMemoryType(&mem_properties, mem_requirements.memoryTypeBits,
                                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                    &memoryTypeIndex);
    assert(ret == VK_SUCCESS);
    VkMemoryAllocateInfo alloc_info{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = mem_requirements.size,
        .memoryTypeIndex = memoryTypeIndex
    };
    uniform_buffer_memory_ = device_->AllocateMemory(&alloc_info);
    assert(uniform_buffer_memory_);
    uniform_buffer_memory_->BindBufferMemory(uniform_buffer_->buffer(), 0);
    uniform_buffer_memory_->MapMemory(0, mem_requirements.size, &uniform_buffer_data_);
}

void Particle::DestroyUinformBuffer() {
    uniform_buffer_memory_->UnmapMemory();
    uniform_buffer_data_ = nullptr;
    VulkanLogicDevice::FreeMemory(&uniform_buffer_memory_);
    VulkanLogicDevice::DestroyBuffer(&uniform_buffer_);
}

void Particle::CopyDataToUinformBuffer() const {
    delta_time_t delta_time;
    delta_time.t = 30.f * 2.0f;
    memcpy(uniform_buffer_data_, &delta_time, sizeof(delta_time_t));
}

void Particle::CreateStorageBuffer() {
    VkBufferCreateInfo bufferCreateInfo {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .size = sizeof (particle_t) * PARTICLE_COUNT,
            .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr
    };
    storage_buffer_ = device_->CreateBuffer(&bufferCreateInfo);
    assert(storage_buffer_);
    VkMemoryRequirements mem_requirements{};
    storage_buffer_->GetBufferMemoryRequirements(&mem_requirements);
    VkPhysicalDeviceMemoryProperties mem_properties{};
    device_->GetPhysicalDeviceMemoryProperties(&mem_properties);

    uint32_t memoryTypeIndex = 0;
    VkResult ret = VulkanLogicDevice::GetMemoryType(&mem_properties, mem_requirements.memoryTypeBits,
                                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                    &memoryTypeIndex);
    assert(ret == VK_SUCCESS);
    VkMemoryAllocateInfo alloc_info {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = nullptr,
            .allocationSize = mem_requirements.size,
            .memoryTypeIndex = memoryTypeIndex
    };
    storage_buffer_memory_ = device_->AllocateMemory(&alloc_info);
    assert(storage_buffer_memory_);
    storage_buffer_memory_->BindBufferMemory(storage_buffer_->buffer(), 0);
    void* storage_buffer_data;
    storage_buffer_memory_->MapMemory(0, mem_requirements.size, &storage_buffer_data);
    CopyDataToyStorageBuffer(storage_buffer_data, mem_requirements.size);
    storage_buffer_memory_->UnmapMemory();
}

void Particle::DestroyStorageBuffer() {
    VulkanLogicDevice::FreeMemory(&storage_buffer_memory_);
    VulkanLogicDevice::DestroyBuffer(&storage_buffer_);
}

void Particle::CopyDataToyStorageBuffer(void* data, size_t size) {
    // Initialize particles
    std::default_random_engine rndEngine((unsigned)time(nullptr));
    std::uniform_real_distribution<float> rndDist(0.0f, 1.0f);

    // Initial particle positions on a circle
    std::vector<particle_t> particles(PARTICLE_COUNT);
    for (auto& particle : particles) {
        float r = 0.25f * sqrt(rndDist(rndEngine));
        float theta = rndDist(rndEngine) * 2.0f * 3.14159265358979323846f;
        float x = r * cos(theta) * (float)frame_buffer_size_.height / (float)frame_buffer_size_.width;
        float y = r * sin(theta);
        particle.pos = glm::vec2(x, y);
        particle.velocity = glm::normalize(glm::vec2(x,y)) * 0.00025f;
        particle.color = glm::vec4(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine), 1.0f);
    }
    memcpy(data, particles.data(), size);
}

void Particle::CreateDescriptorPool() {
    std::array<VkDescriptorPoolSize, 2> descriptorPoolSize{};
    descriptorPoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorPoolSize[0].descriptorCount = 1;
    descriptorPoolSize[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorPoolSize[1].descriptorCount = 2;
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .maxSets = 1,
            .poolSizeCount = static_cast<uint32_t>(descriptorPoolSize.size()),
            .pPoolSizes = descriptorPoolSize.data(),
    };
    descriptor_pool_ = device_->CreateDescriptorPool(&descriptorPoolCreateInfo);
    assert(descriptor_pool_);
}

void Particle::CreateDescriptorSetLayout() {
    std::array<VkDescriptorSetLayoutBinding, 3> descriptorSetLayoutBindings;
    descriptorSetLayoutBindings[0].binding = 0;
    descriptorSetLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorSetLayoutBindings[0].descriptorCount = 1;
    descriptorSetLayoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    descriptorSetLayoutBindings[0].pImmutableSamplers = nullptr;

    descriptorSetLayoutBindings[1].binding = 1;
    descriptorSetLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorSetLayoutBindings[1].descriptorCount = 1;
    descriptorSetLayoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    descriptorSetLayoutBindings[1].pImmutableSamplers = nullptr;

    descriptorSetLayoutBindings[2].binding = 2;
    descriptorSetLayoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorSetLayoutBindings[2].descriptorCount = 1;
    descriptorSetLayoutBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    descriptorSetLayoutBindings[2].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .bindingCount = 3,
            .pBindings = descriptorSetLayoutBindings.data()
    };

    descriptor_set_layout_ = device_->CreateDescriptorSetLayout(&descriptorSetLayoutCreateInfo);
    assert(descriptor_set_layout_);
}

void Particle::CreateDescriptorSet() {
    VkDescriptorSetLayout layouts[] = {descriptor_set_layout_->descriptor_set_layout()};
    descriptor_set_ = descriptor_pool_->AllocateDescriptorSet(layouts);
    assert(descriptor_set_);
}

void Particle::UpdateDescriptorSet() {
    VkDescriptorBufferInfo uniformDescriptorBufferInfo {
        .buffer = uniform_buffer_->buffer(),
        .offset = 0,
        .range = sizeof(delta_time_t)
    };
    VkDescriptorBufferInfo storageDescriptorBufferInfo {
        .buffer = storage_buffer_->buffer(),
        .offset = 0,
        .range = sizeof(particle_t) * PARTICLE_COUNT
    };

    std::array<VkWriteDescriptorSet, 3> descriptorWrites;
    descriptorWrites[0]  = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = descriptor_set_->descriptor_set(),
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pImageInfo = nullptr,
        .pBufferInfo = &uniformDescriptorBufferInfo,
        .pTexelBufferView = nullptr
    };
    descriptorWrites[1] = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = descriptor_set_->descriptor_set(),
        .dstBinding = 1,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pImageInfo = nullptr,
        .pBufferInfo = &storageDescriptorBufferInfo,
        .pTexelBufferView = nullptr
    };
    descriptorWrites[2] = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = descriptor_set_->descriptor_set(),
        .dstBinding = 2,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pImageInfo = nullptr,
        .pBufferInfo = &storageDescriptorBufferInfo,
        .pTexelBufferView = nullptr
    };
    device_->UpdateDescriptorSets(3, descriptorWrites.data());
}

void Particle::CreatePipelineLayout() {
    VkDescriptorSetLayout descriptorSetLayouts[] = { descriptor_set_layout_->descriptor_set_layout() };
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .setLayoutCount = 1,
            .pSetLayouts = descriptorSetLayouts,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr
    };
    pipeline_layout_ =  device_->CreatePipelineLayout(&pipelineLayoutCreateInfo);
    assert(pipeline_layout_);
}

VulkanShaderModule* Particle::CreateShaderModule(const std::string& name,
                                                     shaderc_shader_kind kind,
                                                     const std::string& source) const {
    std::vector<uint32_t> code =
            VulkanObject::CompileFile(name, kind, source);
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