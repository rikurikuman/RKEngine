#include "Renderer.h"
#include <Texture.h>
#include <RDirectX.h>
#include <RenderTarget.h>
#include <RWindow.h>

#include <OpaqueRenderStage.h>
#include <TransparentRenderStage.h>
#include <SpriteRenderStage.h>
#include <FinalRenderStage.h>

void Renderer::Execute()
{
	Renderer* instance = GetInstance();
	RenderTarget::SetToTexture("RenderingImage");
	RenderTarget::GetRenderTargetTexture("RenderingImage")->ClearRenderTarget();
	RenderTarget::GetRenderTargetTexture("RenderingImage")->ClearDepthStencil();
	for (auto itr = instance->stages.begin(); itr != instance->stages.end(); itr++) {
		IRenderStage* stage = itr->get();
		if (stage->enabled) stage->Render();
		stage->orders.clear();
	}
}

void Renderer::DrawCall(std::string stageID, D3D12_VERTEX_BUFFER_VIEW* vertView, D3D12_INDEX_BUFFER_VIEW* indexView, UINT indexCount, const std::vector<RootData>& rootData, Vector3& anchorPoint)
{
	RenderOrder order;
	order.anchorPoint = anchorPoint;
	order.vertView = vertView;
	order.indexView = indexView;
	order.indexCount = indexCount;
	order.rootData = rootData;
	Renderer::DrawCall(stageID, order);
}

void Renderer::DrawCall(std::string stageID, RenderOrder order)
{
	Renderer* instance = GetInstance();

	if (stageID.empty()) {
#ifdef _DEBUG
		OutputDebugStringA("RKEngine WARNING: Renderer::DrawCall() called without an stageID.\n");
#endif
		return;
	}

	for (auto itr = instance->stages.begin(); itr != instance->stages.end(); itr++) {
		IRenderStage* stage = itr->get();
		if (stage->GetTypeIndentifier() == stageID) {

			//���ݒ�̍��ڂ������_���[�̐ݒ�Ŏ����ŕ⊮����
			//�����ł����ݒ�̂܂܂ɂȂ����ꍇ�̓����_�[�X�e�[�W�ɔC����
			if(order.renderTargets.empty()) order.renderTargets = instance->renderTargets;
			if(order.primitiveTopology == D3D_PRIMITIVE_TOPOLOGY_UNDEFINED) order.primitiveTopology = instance->primitiveTopology;
			if(order.viewports.empty()) order.viewports = instance->viewports;
			if(order.scissorRects.empty()) order.scissorRects = instance->scissorRects;
			if(order.rootSignature == nullptr) order.rootSignature = instance->rootSignature;
			if(order.pipelineState == nullptr) order.pipelineState = instance->pipelineState;

			stage->orders.push_back(order);
			return;
		}
	}

#ifdef _DEBUG
	OutputDebugStringA(("RKEngine WARNING: Renderer::DrawCall() : RenderStage(" + stageID + ") is not found.\n").c_str());
#endif
}

void Renderer::InitRenderStages()
{
	Renderer* instance = GetInstance();
}

void Renderer::RemoveRenderStage(std::string id)
{
	Renderer* instance = GetInstance();

	if (id.empty()) {
#ifdef _DEBUG
		OutputDebugStringA("RKEngine WARNING: Renderer::RemoveRenderStage() called without an stageID.\n");
#endif
		return;
	}

	for (auto itr = instance->stages.begin(); itr != instance->stages.end(); itr++) {
		IRenderStage* stage = itr->get();
		if (stage->GetTypeIndentifier() == id) {
			instance->stages.erase(itr);
			return;
		}
	}

#ifdef _DEBUG
	OutputDebugStringA(("RKEngine WARNING: Renderer::RemoveRenderStage() : RenderStage(" + id + ") is not found.\n").c_str());
#endif
}

void Renderer::SetAllParamaterToAuto()
{
	SetRenderTargetToAuto();
	SetRootSignatureToAuto();
	SetPipelineToAuto();

	Viewport viewport{};
	viewport.width = static_cast<float>(RWindow::GetWidth());
	viewport.height = static_cast<float>(RWindow::GetHeight());
	viewport.topleftX = 0;
	viewport.topleftY = 0;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	Rect scissorRect{};
	scissorRect.left = 0;
	scissorRect.right = scissorRect.left + RWindow::GetWidth();
	scissorRect.top = 0;
	scissorRect.bottom = scissorRect.top + RWindow::GetHeight();

	GetInstance()->viewports.clear();
	GetInstance()->viewports.push_back(viewport);
	GetInstance()->scissorRects.clear();
	GetInstance()->scissorRects.push_back(scissorRect);
}

void Renderer::Init()
{
	RenderTarget::CreateRenderTargetTexture(RWindow::GetWidth(), RWindow::GetHeight(), { 0, 0, 0, 1 }, "RenderingImage");
	
	AddDefRenderStageBack<OpaqueRenderStage>();
	AddDefRenderStageBack<TransparentRenderStage>();
	AddDefRenderStageBack<SpriteRenderStage>();
	AddDefRenderStageBack<FinalRenderStage>();
}