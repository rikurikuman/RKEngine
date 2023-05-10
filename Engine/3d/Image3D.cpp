#include "Image3D.h"
#include "RDirectX.h"
#include "Vertex.h"
#include "LightGroup.h"
#include <Renderer.h>

Image3D::Image3D(TextureHandle texture, Vector2 size, bool forceSize)
{
	this->texture = texture;

	if (forceSize) {
		this->size.x = size.x;
		this->size.y = size.y;
	}
	else {
		scale = size;
		Texture tex = TextureManager::Get(texture);
		this->size.x = tex.resource->GetDesc().Width / (float)tex.resource->GetDesc().Height * scale.x;
		this->size.y = scale.y;
	}

	Init();
}
	
void Image3D::Init()
{
	//���_�f�[�^
	VertexPNU vertices[] = {
		{{ -0.5f * size.x, -0.5f * size.y, 0.0f }, {}, {0.0f, 1.0f}}, //����
		{{ -0.5f * size.x,  0.5f * size.y, 0.0f }, {}, {0.0f, 0.0f}}, //����
		{{  0.5f * size.x, -0.5f * size.y, 0.0f }, {}, {1.0f, 1.0f}}, //�E��
		{{  0.5f * size.x,  0.5f * size.y, 0.0f }, {}, {1.0f, 0.0f}}, //�E��
	};

	//���_�C���f�b�N�X�f�[�^
	UINT indices[] = {
		0, 1, 2,
		1, 3, 2
	};

	VertexPNU::CalcNormalVec(vertices, indices, _countof(indices));

	vertBuff.Init(vertices, _countof(vertices));
	indexBuff.Init(indices, _countof(indices));
}

void Image3D::TransferBuffer(ViewProjection viewprojection)
{
	material.Transfer(materialBuff.constMap);
	transform.Transfer(transformBuff.constMap);
	viewProjectionBuff.constMap->matrix = viewprojection.matrix;
}

void Image3D::Draw()
{
	std::vector<RootData> rootData{
		{TextureManager::Get(texture).gpuHandle},
		{RootDataType::CBV, materialBuff.constBuff->GetGPUVirtualAddress()},
		{RootDataType::CBV, transformBuff.constBuff->GetGPUVirtualAddress()},
		{RootDataType::CBV, viewProjectionBuff.constBuff->GetGPUVirtualAddress()},
		{RootDataType::CBV, LightGroup::nowLight->buffer.constBuff->GetGPUVirtualAddress()},
	};

	Renderer::DrawCall("Opaque", &vertBuff.view, &indexBuff.view, 6, rootData);
}

void Image3D::DrawCommands()
{
	//���_�o�b�t�@�r���[�̐ݒ�R�}���h
	RDirectX::GetCommandList()->IASetVertexBuffers(0, 1, &vertBuff.view);

	//�C���f�b�N�X�o�b�t�@�r���[�̐ݒ�R�}���h
	RDirectX::GetCommandList()->IASetIndexBuffer(&indexBuff.view);

	//�萔�o�b�t�@�r���[�̐ݒ�R�}���h
	RDirectX::GetCommandList()->SetGraphicsRootConstantBufferView(1, materialBuff.constBuff->GetGPUVirtualAddress());
	RDirectX::GetCommandList()->SetGraphicsRootConstantBufferView(2, transformBuff.constBuff->GetGPUVirtualAddress());
	RDirectX::GetCommandList()->SetGraphicsRootConstantBufferView(3, viewProjectionBuff.constBuff->GetGPUVirtualAddress());
	RDirectX::GetCommandList()->SetGraphicsRootConstantBufferView(4, LightGroup::nowLight->buffer.constBuff->GetGPUVirtualAddress());

	//SRV�q�[�v����K�v�ȃe�N�X�`���f�[�^���Z�b�g����(�w�i)
	RDirectX::GetCommandList()->SetGraphicsRootDescriptorTable(0, TextureManager::Get(texture).gpuHandle);

	//�`��R�}���h
	RDirectX::GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0); // �S�Ă̒��_���g���ĕ`��
}