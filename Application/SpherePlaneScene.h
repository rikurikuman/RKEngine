#pragma once
#include "IScene.h"
#include "ColPrimitive3D.h"
#include "ModelObj.h"
#include <Camera.h>
#include <LightGroup.h>

class SpherePlaneScene : public IScene
{
public:
	SpherePlaneScene();

	void Init() override;
	void Update() override;
	void Draw() override;

private:
	Camera camera;
	LightGroup light;

	ColPrimitive3D::Sphere colSphere;
	ColPrimitive3D::Plane colPlane;
	ModelObj sphere;
	ModelObj plane;

	float radius = 1;
	float rotPlane = 0;

	bool autoMove = true;
	float moveDir = 1;
};

