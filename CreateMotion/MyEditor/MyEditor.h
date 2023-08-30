#pragma once
#include"Entity.hpp"


namespace mj
{
	class EditorsCamera;
}

class MyEditor:public component::Entity
{
private:
	const mj::EditorsCamera* camera;

public:
	MyEditor();

	void update(double dt=Scene::DeltaTime());
	void draw()const;

	bool working;
};

