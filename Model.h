#pragma once
#include <string>
#include <vector>
#include <mutex>
#include "Vertex.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "RConstBuffer.h"
#include "Material.h"

//assimp
#include <Importer.hpp>
#include <scene.h>
#include <postprocess.h>

#ifdef _DEBUG
//#pragma comment(lib, "zlibstaticd.lib")
#pragma comment(lib, "assimp-vc143-mtd.lib")
#else
//#pragma comment(lib, "zlibstatic.lib")
#pragma comment(lib, "assimp-vc143-mt.lib")
#endif

typedef std::string ModelHandle;

class ModelData
{
public:
	std::string name;
	VertexBuffer vertexBuff;
	IndexBuffer indexBuff;
	RConstBuffer<MaterialBuffer> materialBuff;

	std::vector<VertexPNU> vertexs;
	std::vector<UINT> indices;
	Material material;

	void UpdateMaterial() {
		material.Transfer(materialBuff.constMap);
	}

	bool operator==(const ModelData& o) const;
	bool operator!=(const ModelData& o) const;
};

class Model
{
public:
	std::string name;
	std::string path;
	std::vector<std::shared_ptr<ModelData>> data;

	static ModelHandle Load(std::string filepath, std::string filename, ModelHandle handle = "");
	static ModelHandle Register(ModelHandle handle, Model model);

	static ModelHandle LoadWithAIL(std::string directoryPath, std::string filename, ModelHandle handle = "");
};

class ModelManager
{
public:
	ModelManager() {
		Init();
	}

	static ModelManager* GetInstance() {
		static ModelManager manager;
		return &manager;
	}

	static Model* Get(ModelHandle handle);

	std::map<ModelHandle, Model> modelMap;
	std::recursive_mutex mutex;

private:
	void Init();
};