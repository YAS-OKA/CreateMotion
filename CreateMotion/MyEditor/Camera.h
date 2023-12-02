#pragma once
#include"Object.h"

class Camera :public Object
{
	DebugCamera3D camera;
	Object* followTarget;
public:
	Camera(const DebugCamera3D& camera);

	virtual void update(double dt) override;

	DebugCamera3D getCamera()const;
};
