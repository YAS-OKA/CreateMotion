#include "../stdafx.h"
#include "Object.h"
#include"Transform.h"

Object::Object()
{
	name = U"Object";
	transform = addComponentNamed<Transform>(U"original");
}

void Object::update(double dt)
{/*
	Print << name;
	Print << U"nowPos: " << transform->getPos();
	Print << U"nowVel: " << transform->getVel();
	Print << U"nowDirection: " << transform->getDirection();
	Print << U"nowScale:" << transform->getAspect();*/
}
