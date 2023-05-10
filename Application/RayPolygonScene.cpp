#include <TimeManager.h>
#include "RayPolygonScene.h"
#include <RImGui.h>
#include <Quaternion.h>
#include <Renderer.h>

RayPolygonScene::RayPolygonScene()
{
	sphere = ModelObj(Model::Load("./Resources/Model/", "Sphere.obj", "Sphere", true));
	ray = ModelObj(Model::Load("./Resources/Model/", "Cube.obj", "Cube", false));

	sphere.transform.position = { 0, 5, 0 };
	sphere.transform.scale = { 0.1f, 0.1f, 0.1f };
	sphere.transform.UpdateMatrix();

	colRay = { {0, 5, 0}, {0, -1, 0} };

	polygonPipeline = RDirectX::GetDefPipeline();
	polygonPipeline.desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	polygonPipeline.Create();

	VertexPNU verts[3] = {
		{posA, {0, 0, 0}, {0, 0}},
		{posB, {0, 0, 0}, {0, 0}},
		{posC, {0, 0, 0}, {0, 0}}
	};

	vertBuff.Init(verts, 3);

	vertBuffT = SRBufferAllocator::Alloc(sizeof(VertexPNU) * 3, 1);

	//GPU��̃o�b�t�@�ɑΉ��������z���������擾
	//����͒��_�o�b�t�@�̃}�b�s���O
	VertexPNU* vertMap = reinterpret_cast<VertexPNU*>(vertBuffT.Get());
	//�S���_�ɑ΂���
	for (UINT i = 0; i < 3; i++) {
		vertMap[i] = verts[i];
	}

	//���_�o�b�t�@�r���[�̍쐬
	vertBuffView.BufferLocation = vertBuffT.GetGPUVirtualAddress(); //GPU���z�A�h���X
	vertBuffView.SizeInBytes = sizeof(VertexPNU) * 3; //���_�o�b�t�@�̃T�C�Y
	vertBuffView.StrideInBytes = sizeof(VertexPNU); //���_��̃T�C�Y

	camera.viewProjection.eye = { 0, 3, -10 };
	camera.viewProjection.target = { 0, 3, 0 };
	camera.viewProjection.UpdateMatrix();
}

void RayPolygonScene::Init()
{
	Camera::nowCamera = &camera;
	LightGroup::nowLight = &light;
}

void RayPolygonScene::Update()
{
	//�e�X�gGUI
	{
		ImGui::SetNextWindowPos({ ImGui::GetMainViewport()->WorkPos.x + 800, ImGui::GetMainViewport()->WorkPos.y + 10 }, ImGuiCond_Once);
		ImGui::SetNextWindowSize({ 400, 250 });

		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoResize;
		ImGui::Begin("Control", NULL, window_flags);

		if (ImGui::Button("Reset")) {
			colRay.start = { 0, 5, 0 };
			posA = { 0, 0, 1 };
			posB = { -1, 0, -1 };
			posC = { 1, 0, -1 };
		}

		ImGui::Text("Ray");

		ImGui::Checkbox("AutoMove", &autoMove);
		ImGui::DragFloat3("Position", &colRay.start.x, 0.01f);

		ImGui::Text("Polygon");
		ImGui::DragFloat3("Position1", &posA.x, 0.01f);
		ImGui::DragFloat3("Position2", &posB.x, 0.01f);
		ImGui::DragFloat3("Position3", &posC.x, 0.01f);

		ImGui::End();
	}

	if (autoMove) {
		colRay.start.x += 5.0f * TimeManager::deltaTime * moveDir;
		if (colRay.start.x > 5
			|| colRay.start.x < -5) {
			colRay.start.x = Util::Clamp(colRay.start.x, -5.0f, 5.0f);
			moveDir *= -1;
		}
	}

	sphere.transform.position = colRay.start;
	sphere.transform.UpdateMatrix();

	colPolygon.p0 = posA;
	colPolygon.p1 = posC;
	colPolygon.p2 = posB;

	Vector3 v1 = posC - posA;
	Vector3 v2 = posB - posC;

	Vector3 normal = v1.Cross(v2);
	normal.Normalize();
	colPolygon.normal = normal;

	float dis = 0;
	Vector3 inter;
	if (ColPrimitive3D::CheckRayToTriangle(colRay, colPolygon, &dis, &inter)) {
		sphere.tuneMaterial.color = { 1, 0, 0, 1 };
		ray.tuneMaterial.color = { 0, 0, 1, 1 };
		ray.transform.position = colRay.start;
		ray.transform.position += colRay.dir * dis / 2.0f;
		ray.transform.scale = { 0.1f, dis, 0.1f };
	}
	else {
		sphere.tuneMaterial.color = { 1, 1, 1, 1 };
		ray.tuneMaterial.color = { 1, 1, 1, 1 };
		ray.transform.position = colRay.start;
		ray.transform.position += colRay.dir * 50;
		ray.transform.scale = { 0.1f, 100, 0.1f };
	}
	ray.transform.UpdateMatrix();

	light.Update();
	sphere.TransferBuffer(Camera::nowCamera->viewProjection);
	ray.TransferBuffer(Camera::nowCamera->viewProjection);

	VertexPNU verts[3] = {
		{posA, {0, 0, 0}, {0, 0}},
		{posC, {0, 0, 0}, {0, 0}},
		{posB, {0, 0, 0}, {0, 0}}
	};
	vertBuff.Update(verts, 3);

	VertexPNU* vertMap = reinterpret_cast<VertexPNU*>(vertBuffT.Get());
	vertMap[0] = posA;
	vertMap[1] = posC;
	vertMap[2] = posB;

	transformBuff.constMap->matrix = Matrix4();
	materialBuff.constMap->color = { 1, 1, 1, 1 };
	materialBuff.constMap->ambient = { 1, 1, 1 };
	materialBuffT->color = { 1, 1, 1, 1 };
	materialBuffT->ambient = { 1, 1, 1 };
	Camera::nowCamera->viewProjection.Transfer(viewProjectionBuff.constMap);
	Camera::nowCamera->viewProjection.Transfer(viewProjectionBuffT.Get());
}

void RayPolygonScene::Draw()
{
	sphere.Draw();
	ray.Draw();
	
	RenderOrder polygon;
	polygon.pipelineState = polygonPipeline.ptr.Get();
	polygon.rootData = {
		{TextureManager::Get("").gpuHandle},
		{RootDataType::CBV, materialBuffT.buff.GetGPUVirtualAddress()},
		{RootDataType::CBV, transformBuff.constBuff->GetGPUVirtualAddress()},
		{RootDataType::CBV, viewProjectionBuffT.buff.GetGPUVirtualAddress()},
		{RootDataType::CBV, LightGroup::nowLight->buffer.constBuff->GetGPUVirtualAddress()},
	};
	polygon.vertView = &vertBuffView;
	polygon.indexCount = 3;

	Renderer::DrawCall("Opaque", polygon);
}