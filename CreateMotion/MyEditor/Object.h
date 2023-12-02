#pragma once
#include"EC.hpp"

class Transform;

class Object :public Entity
{
public:
	Object();

	void update(double dt)override;

	Transform* transform;
};
