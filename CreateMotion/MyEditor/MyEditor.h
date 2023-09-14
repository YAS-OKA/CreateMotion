#pragma once
#include"Entity.hpp"


namespace mj
{
	class Parts;
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
	void select(mj::Parts* parts);

	void updateParentPos(Array<mj::Parts*> parts_list);

	//移動、回転を切り替える
	//size_t index;
	//パーツを検索して選択する
	TextEditState selectText;

	bool working;

	HashTable<mj::Parts*, Array<mj::Parts*>>parts_key_parent;
};

