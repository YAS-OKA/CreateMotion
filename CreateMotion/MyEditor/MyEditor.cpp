#include "../stdafx.h"
#include "MyEditor.h"
#include"CreatMotionsJson.h"

MyEditor::MyEditor()
{
	working = true;
	//index = 0;
	
	AddComponent<mj::RegisterParts>();
	AddComponent<mj::EditParts>();
	camera = AddComponent<mj::EditorsCamera>();
	AddComponent<mj::StartMotion>();
	AddComponent<mj::SaveParts>();
	AddComponent<mj::ErasePartsOperate>();
	AddComponent<mj::LoadJson>();
	AddComponent<mj::PartsColliders>();
	AddComponent<mj::RotateCenter>();
	AddComponent<mj::LightUpParts>();
	AddComponent<mj::MakeHitbox>();
	//AddComponent<mj::MakeRectF>();
}

void MyEditor::update(double dt)
{
	update_components(dt);

	updateParentPos(GetComponentArr<mj::Parts>());

	mj::MoveParts* comp = GetComponent<mj::MoveParts>();
	if (comp != nullptr)
	{
		comp->parts_key_parent = parts_key_parent;
	};

	//SimpleGUI::RadioButtons(index, { U"移動",U"回転" }, { 10,170 });

	mj::MakeHitbox* hitboxSetter=GetComponent<mj::MakeHitbox>();

	Array<mj::Parts*>partsArray = hitboxSetter->setting ? hitboxSetter->hitboxs : GetComponentArr<mj::Parts>(false);

	if (SimpleGUI::TextBox(selectText, { Scene::Width() - 205,300 })) {
		for (auto& parts : partsArray)
		{
			if (parts->name == selectText.text)
			{
				select(parts);
				break;
			}
		}
	}
	//パーツの選択
	if (MouseL.up())
	{
		const auto t = camera->getTransformer2D(true);
		for (auto& parts : partsArray)
		{
			if(GetComponent<mj::PartsColliders>()->mouseOver(parts))
			{
				select(parts);
				break;
			}
		}
	}
	//パーツを動かす つかむ
	if (MouseR.down())
	{
		const auto t = camera->getTransformer2D(true);

		if (GetComponent<mj::RotateCenter>()->mouseOver())
		{
			/*if(index==0)*/AddComponent<mj::MoveRotateCenter>()->select(GetComponent<mj::EditParts>()->get_selectedParts());
		}
		else {
			for (auto& parts : partsArray)
			{
				if (GetComponent<mj::EditParts>()->get_selectedParts()==parts and GetComponent<mj::PartsColliders>()->mouseOver(parts))
				{
					/*if (index == 0)*/AddComponent<mj::MoveParts>()->select(parts);
					//else AddComponent<mj::RotateParts>()->select(parts);
					break;
				}
			}
		}
	}
	//パーツを離す
	else if (MouseR.up())
	{
		remove<mj::MoveParts>();
		remove<mj::MoveRotateCenter>();
		//remove<mj::RotateParts>();
	}

	//当たり判定配置モードの切り替え
	if (SimpleGUI::Button(hitboxSetter->setting ? U"戻る" : U"当たり判定", {10,300}))
	{
		hitboxSetter->setting = not hitboxSetter->setting;
		GetComponent<mj::EditParts>()->releaseParts();
		GetComponent<mj::RotateCenter>()->releaseParts();
		GetComponent<mj::LightUpParts>()->releaseParts();
	}
}

void MyEditor::draw()const
{
	{
		const auto t = camera->getTransformer2D(true);
		draw_components();
	}
	Shape2D::Plus(6, 1, Scene::Center()).draw(Palette::White);
}

void MyEditor::select(mj::Parts* parts)
{
	GetComponent<mj::EditParts>()->select(parts);
	GetComponent<mj::RotateCenter>()->setParts(parts);
	GetComponent<mj::LightUpParts>()->select(parts);
}


void MyEditor::updateParentPos(Array<mj::Parts*> parts_list)
{
	parts_key_parent.clear();
	Array<mj::Parts*>parents;
	Array<mj::Parts*>nextParents;
	//最初の親(親なしのパーツ)を探しつつparts_key_parentを構築
	for (auto& parts : parts_list)
	{
		if (not parts_list.contains(parts->PartsParent))
		{
			parts->PartsParent = nullptr;
			parts->ParentPos = { 0,0 };
			parents << parts;
		}
		else {
			parts_key_parent[parts->PartsParent] << parts;
		}
	}
	//幅優先で更新していく
	while (not parents.isEmpty()) {
		for (const auto& parent : parents)
		{
			if (not parts_key_parent.contains(parent))continue;

			for (auto& partsChiled : parts_key_parent[parent])
			{
				//更新
				partsChiled->ParentPos = parent->absPos();
				nextParents << partsChiled;
			}
		}
		parents.clear();
		for (auto& nextParent : nextParents)
		{
			parents << nextParent;
		}
		nextParents.clear();
	}
}
