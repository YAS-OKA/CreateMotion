#include "../stdafx.h"
#include "MyEditor.h"
#include"CreatMotionsJson.h"

MyEditor::MyEditor()
{
	working = true;
	index = 0;
	
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
}

void MyEditor::update(double dt)
{
	update_components(dt);

	mj::updateParentPos(GetComponentArr<mj::Parts>());

	SimpleGUI::RadioButtons(index, { U"移動",U"回転" }, { 10,170 });

	SimpleGUI::TextBox(selectText, { Scene::Width() - 205,300 });
	for (auto& parts : GetComponentArr<mj::Parts>(false))
	{
		//if (parts->get_region().mouseOver())
		if (parts->name == selectText.text)
		{
			select(parts);
			break;
		}

	}

	//パーツの選択
	if (MouseL.up())
	{
		const auto t = camera->getTransformer2D(true);
		bool find = false;
		for (auto& parts : GetComponentArr<mj::Parts>(false))
		{
			//if (parts->get_region().mouseOver())
			if(GetComponent<mj::PartsColliders>()->mouseOver(parts))
			{
				select(parts);
				find = true;
				break;
			}
		}
		if (not find)
		{
			remove<mj::EditParts>();
			remove<mj::RotateCenter>();
			remove<mj::LightUpParts>();
			AddComponent<mj::EditParts>();
			AddComponent<mj::RotateCenter>();
			AddComponent<mj::LightUpParts>();
		}
	}
	//パーツを動かす つかむ
	if (MouseR.down())
	{
		const auto t = camera->getTransformer2D(true);

		if (GetComponent<mj::RotateCenter>()->mouseOver())
		{
			if(index==0)AddComponent<mj::MoveRotateCenter>()->select(GetComponent<mj::EditParts>()->get_selectedParts());
		}
		else {
			for (auto& parts : GetComponentArr<mj::Parts>(false))
			{
				if (GetComponent<mj::EditParts>()->get_selectedParts()==parts and GetComponent<mj::PartsColliders>()->mouseOver(parts))
				{
					if (index == 0)AddComponent<mj::MoveParts>()->select(parts);
					else AddComponent<mj::RotateParts>()->select(parts);
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
		remove<mj::RotateParts>();
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
