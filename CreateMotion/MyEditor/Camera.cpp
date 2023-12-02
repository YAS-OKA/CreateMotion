#include "../stdafx.h"
#include "Camera.h"
#include"Transform.h"

Camera::Camera(const DebugCamera3D& camera)
	:camera(camera),followTarget(nullptr)
{
	name = U"Camera";
}

void Camera::update(double dt)
{
	Object::update(dt);

	if (followTarget == nullptr)return;

	//camera.setTarget(followTarget->transform->getPos());

	camera.update(0.2);
}

DebugCamera3D Camera::getCamera()const
{
	return camera;
}

