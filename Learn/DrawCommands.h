#pragma once

#include "SwapChain.h"
#include "CommandPool.h"
#include "CommandBuffer.h"
#include "RenderPass.h"
#include "Framebuffers.h"
#include "Pipeline.h"
#include "Model.h"
#include "DescriptorSets.h"

class DrawCommands {
public:
	DrawCommands(LogicalDevice* device, SwapChain* swapChain, CommandPool* commandPool, 
		RenderPass* renderPass, Framebuffers* framebuffers, Pipeline* pipeline, Model* model, DescriptorSets* descriptorSets);

private:
	void createCommandBuffers();
	void recordCommands();
	void setupRenderPassBeginInfo(VkRenderPassBeginInfo renderPassBeginInfo, std::array<VkClearValue, 2>& clearValues, size_t index);

	LogicalDevice* device;
	CommandPool* commandPool;
	SwapChain* swapChain;
	RenderPass* renderPass;
	Framebuffers* framebuffers;
	Pipeline* pipeline;
	Model* model;
	DescriptorSets* descriptorSets;

	std::vector<CommandBuffer*> commandBuffers;
};

DrawCommands::DrawCommands(LogicalDevice* inDevice, SwapChain* inSwapChain, CommandPool* inCommandPool, 
	RenderPass* inRenderPass, Framebuffers* inFramebuffers, Pipeline* inPipeline, Model* inModel, DescriptorSets* inDescriptorSets) {
	device = inDevice;
	swapChain = inSwapChain;
	commandPool = inCommandPool;
	renderPass = inRenderPass;
	framebuffers = inFramebuffers;
	pipeline = inPipeline;
	model = inModel;
	descriptorSets = inDescriptorSets;
	createCommandBuffers();
}

void DrawCommands::createCommandBuffers() {
	commandBuffers.resize(swapChain->getImageCount());
	for (size_t i = 0; i < commandBuffers.size(); ++i)
		commandBuffers[i] = new CommandBuffer(device, commandPool);
}

void DrawCommands::recordCommands() {
	for (size_t i = 0; i < commandBuffers.size(); ++i) {
		commandBuffers[i]->beginCommands();

		VkRenderPassBeginInfo renderPassBeginInfo{};
		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		setupRenderPassBeginInfo(renderPassBeginInfo, clearValues, i);

		vkCmdBeginRenderPass(commandBuffers[i]->getCommandBuffer(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffers[i]->getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipeline());
		
		VkBuffer vertexBuffers[] = { model->getVertexBufferRef()->getBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffers[i]->getCommandBuffer(), 0, 1, vertexBuffers, offsets);

		vkCmdBindIndexBuffer(commandBuffers[i]->getCommandBuffer(), model->getVertexBufferRef()->getBuffer(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffers[i]->getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipelineLayout(),
			0, 1, &descriptorSets->getDescriptorSet(i), 0, nullptr);

		vkCmdEndRenderPass(commandBuffers[i]->getCommandBuffer());

		commandBuffers[i]->endCommands();
	}
}

void DrawCommands::setupRenderPassBeginInfo(VkRenderPassBeginInfo beginInfo, std::array<VkClearValue, 2>& clearValues, size_t index) {
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.renderPass = renderPass->getRenderPass();
	beginInfo.framebuffer = framebuffers->getFrameBuffer(index);
	beginInfo.renderArea.offset = { 0, 0 };
	beginInfo.renderArea.extent = swapChain->getExtent();
	beginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	beginInfo.pClearValues = clearValues.data();
}