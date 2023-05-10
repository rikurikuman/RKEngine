#pragma once
#include "IScene.h"
#include "ColPrimitive3D.h"
#include "ModelObj.h"
#include <Camera.h>
#include <LightGroup.h>
#include <SRBuffer.h>
#include <SRConstBuffer.h>

class RayPolygonScene : public IScene {
public:
	RayPolygonScene();
	~RayPolygonScene() override {
		SRBufferAllocator::Free(vertBuffT);
	}

	void Init() override;
	void Update() override;
	void Draw() override;

private:
	Camera camera;
	LightGroup light;

	ColPrimitive3D::Ray colRay;
	ColPrimitive3D::Triangle colPolygon;
	ModelObj sphere;
	ModelObj ray;

	float radius = 1;

	bool autoMove = true;
	float moveDir = 1;

	Vector3 posA = { 0, 0, 1 };
	Vector3 posB = { -1, 0, -1 };
	Vector3	posC = { 1, 0, -1 };

	GraphicsPipeline polygonPipeline;
	VertexBuffer vertBuff;
	RConstBuffer<TransformBuffer> transformBuff;
	RConstBuffer<MaterialBuffer> materialBuff;
	RConstBuffer<ViewProjectionBuffer> viewProjectionBuff;

	SRConstBuffer<MaterialBuffer> materialBuffT;
	SRConstBuffer<ViewProjectionBuffer> viewProjectionBuffT;

	SRBufferPtr vertBuffT;
	D3D12_VERTEX_BUFFER_VIEW vertBuffView{};
};