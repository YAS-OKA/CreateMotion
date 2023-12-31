﻿#include "../stdafx.h"
#include "MyEditor.h"
#include"CreatMotionsJson.h"

MyEditor::MyEditor()
{
	working = true;
	AddComponent<mj::RegisterParts>();
	AddComponent<mj::EditParts>();
	camera = AddComponent<mj::EditorsCamera>();
	AddComponent<mj::StartMotion>();
	AddComponent<mj::SaveParts>();
	AddComponent<mj::ErasePartsOperate>();
	AddComponent<mj::LoadJson>();
	AddComponent<mj::PartsColliders>();
	AddComponent<mj::RotateCenter>()->priority.setDraw(Math::Inf);
}

void MyEditor::update(double dt)
{
	update_components(dt);

	mj::updateParentPos(GetComponentArr<mj::Parts>());
	
	//パーツの選択
	if (MouseL.up())
	{
		const auto t = camera->getTransformer2D(true);
		for (auto& parts : GetComponentArr<mj::Parts>(false))
		{
			//if (parts->get_region().mouseOver())
			if(GetComponent<mj::PartsColliders>()->mouseOver(parts))
			{
				GetComponent<mj::EditParts>()->select(parts);
				GetComponent<mj::RotateCenter>()->setParts(parts);
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
			AddComponent<mj::MoveRotateCenter>()->select(GetComponent<mj::EditParts>()->get_selectedParts());
		}
		else {
			for (auto& parts : GetComponentArr<mj::Parts>(false))
			{				
				/*if (GetComponent<mj::RotateCenter>()->mouseOver())
				{
					AddComponent<mj::MoveRotateCenter>()->select(parts);
					break;
				}*/
				if (GetComponent<mj::PartsColliders>()->mouseOver(parts))
				{
					//remove<mj::MoveParts>();//削除されてるはずだけど、一応削除しておく。
					AddComponent<mj::MoveParts>()->select(parts);
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
