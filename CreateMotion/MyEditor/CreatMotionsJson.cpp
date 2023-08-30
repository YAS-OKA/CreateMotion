#include "../stdafx.h"
#include "CreatMotionsJson.h"
#include"MyEditor.h"

using namespace mj;

void RegisterParts::start()
{
	pos = Vec2{ 10,50 };
}

void RegisterParts::update(double dt)
{
	if (SimpleGUI::Button(U"画像選択",pos))
	{
		addParts(Dialog::OpenFiles({ FileFilter::AllImageFiles() }));
	}
}

void RegisterParts::addParts(const Array<FilePath>& path)
{
	for (const auto& p : path)
	{
		AddComponent<Parts>(new Parts(p));
	}
}

Parts::Parts(const FilePath& path)
	:tex(Texture{ path ,TextureDesc::Mipped}),name(FileSystem::BaseName(path))
{
	ParentPos = Vec2{ 0,0 };
	bool flag = false;
	PartsParent = nullptr;

	for (const auto& elem : JsonElems)
	{
		if (elem.first == U"TexturePath")
		{
			m_params.emplace(elem.first, FileSystem::RelativePath(path,startPath));
			flag = true;
		}
		else {
			m_params.emplace(elem.first, *elem.second);
		}
	}
	if (not flag)throw Error{ U"TexturePathがありません。" };
}

const Circle& Parts::getRotateCenterCircle()
{
	return Circle{ absPos() + Parse<Vec2>(params(U"RotateCenter")),3};
}

const RectF& Parts::get_region()
{
	return tex.scaled(Parse<double>(params(U"Scale"))).regionAt(absPos());
}

Vec2 Parts::absPos() const
{
	return ParentPos+Parse<Vec2>(params(U"Position"));
}

String Parts::params(const String& name)const
{
	for (const auto& param : m_params)
	{
		if (param.first == name)
		{
			if (name == U"Position")
			{
				auto a = param.second;
				auto b = ParentPos;
				return Format(Parse<Vec2>(param.second) - ParentPos);
			}

			return param.second;
		}
	}
	throw Error{ name + U"がありません。" };
}

void Parts::set_params(const String& name, const String& param)
{
	if (not m_params.contains(name))throw Error{ name + U"がありません。" };

	if (name == U"TexturePath")
	{		
		if (m_params[name]!=param and (System::MessageBoxOKCancel(U"TexturePathを変更しますか？") == MessageBoxResult::OK))
		{
			m_params[name] = param;
			tex = Texture{ FileSystem::FullPath(U"CharacterImages/" + m_params[U"TexturePath"]),TextureDesc::Mipped };
		}
		return;
	}
	else if (name == U"Position")
	{

		m_params[name] = Format(Parse<Vec2>(param) - ParentPos);
	}
	else if (name == U"Parent")
	{
		bool flag = false;
		for (auto& parts : GetComponentArr<Parts>())if (parts->name == param) {
			ParentPos = parts->absPos();
			PartsParent = parts;
			flag = true;
			break;
		}
		if (not flag) {
			ParentPos = { 0,0 }; PartsParent = nullptr;
		}
	}

	m_params[name] = param;
}

void Parts::MoveBy(const String& target,const Vec2& delta)
{
	set_params(target, Format(Parse<Vec2>(params(target)) + delta));
}

void Parts::update(double dt)
{
	priority.setDraw(Parse<double>(params(U"Z")));
}

void Parts::draw()const
{
	tex.scaled(Parse<double>(params(U"Scale"))).drawAt(absPos());
	
	(absPos() + Parse<Vec2>(params(U"RotateCenter"))).asCircle(3).draw(Palette::Red);
}

void MoveParts::start()
{
	selectedParts = nullptr;
}

void MoveParts::update(double dt)
{
	//カメラの位置や拡大縮小を考慮
	const auto t = GetComponent<EditorsCamera>()->getTransformer2D(true);
	if (selectedParts != nullptr)selectedParts->MoveBy(U"Position",Cursor::DeltaF());
}

void MoveParts::select(Parts* parts)
{
	selectedParts = parts;
}

void MoveRotateCenter::update(double dt)
{
	//カメラの位置や拡大縮小を考慮
	const auto t = GetComponent<EditorsCamera>()->getTransformer2D(true);
	if (selectedParts != nullptr)selectedParts->MoveBy(U"RotateCenter",Cursor::DeltaF());
}

void EditParts::start()
{	
	selectedParts = nullptr;
	x = 470;
	y = 0;
	w = Scene::Width()-x;
	for (const auto& elem : JsonElems)
	{
		texts.emplace(elem.first, TextEditState());
	}
	font = Font{ 15 };
	hiddenList = {U"Position",U"RotateCenter"};
	h = 50;
	for (const auto& text : texts)
	{
		if (hiddenList.contains(text.first))continue;
		h += 40;
	}
}

void EditParts::update(double dt)
{
	const Transformer2D transformer{ Mat3x2::Translate(x,y), TransformCursor::Yes };
	Rect{ 0,0,w,h }.draw(Palette::White);
	bool enable = selectedParts != nullptr;
	double distance = 10;

	for (auto& text : texts)
	{
		if (hiddenList.contains(text.first))continue;
		font(text.first + U":").draw(Vec2{ 5,distance },Palette::Black );
		double x = font(text.first + U":").region().w + 5;
		SimpleGUI::TextBox(text.second, Vec2{ Max(100.0,x) ,distance }, 200, unspecified, enable);
		distance += 40;
	}

	if (SimpleGUI::Button(U"適用", Vec2{ w - 100,distance },unspecified,enable))
	{
		for (const auto& text : texts)
		{
			if (hiddenList.contains(text.first))continue;
			selectedParts->set_params(text.first, text.second.text);
		};
	}
}

void EditParts::select(Parts* parts)
{
	selectedParts = parts;
	texts.clear();
	for (const auto& elem : JsonElems)
	{
		texts.emplace(elem.first,TextEditState{ parts->params(elem.first) });
	}
}

void EditParts::releaseParts()
{
	selectedParts = nullptr;
	texts.clear();
	for (const auto& elem : JsonElems)
	{
		texts.emplace(elem.first, TextEditState());
	}
}

Parts* EditParts::get_selectedParts()
{
	return selectedParts;
}

void EditorsCamera::start()
{
	sensitivity = 0.1;
	mouse_sensitivity = -10;
	scale_range = 5.0;
	touch_thumb = touch_bar_thumb = false;
	double distance_from_Scene = 20, bar_h = 20, bar_thumb_w = 15, bar_from_range = 10;
	r = 80;
	x = Scene::Width() - r - distance_from_Scene;
	y = Scene::Height() - r - distance_from_Scene;
	range = Circle{ Vec2{x,y},r };
	thumb = Circle{ Vec2{x,y} ,r / 4 };
	bar_thumb = RectF{ Arg::center(x, y -  r - bar_h / 2-bar_from_range),bar_thumb_w,bar_h };
	scale_bar = RectF{ x - r,y - r - bar_h - bar_from_range ,2 * r,bar_h };
	camera = Camera2D{ Scene::Center(),1.0,CameraControl::None_ };
}

void EditorsCamera::update(double dt)
{
	//UIの操作
	if (MouseL.up()){
		touch_thumb = touch_bar_thumb = false;
		thumb.setCenter(x, y);
	}else if(MouseL.down()){
		touch_thumb = thumb.leftClicked();
		if ((not touch_thumb) and range.leftClicked())
		{
			Vec2 p{ x,y };
			p += (Cursor::Pos() - p).setLength(Min((Cursor::Pos() - p).length(), range.r - thumb.r - 1));
			thumb.setCenter(p);
			touch_thumb = true;
		}
		touch_bar_thumb = bar_thumb.leftClicked();
		if ((not touch_bar_thumb) and scale_bar.leftClicked())
		{
			Vec2 p{ scale_bar.center() };
			if (abs(Cursor::Pos().x - p.x) < (scale_bar.w - bar_thumb.w) / 2){
				p.x += Cursor::Pos().x - p.x;
			}else{
				p.x += Cursor::Pos().x - p.x > 0 ? (scale_bar.w - bar_thumb.w) / 2 : -(scale_bar.w - bar_thumb.w) / 2;
			}
			bar_thumb.setCenter(p);
			touch_bar_thumb = true;
		}
	};
	//UIを動かす
	if (touch_thumb) {
		if (thumb.movedBy(Cursor::DeltaF()).intersectsAt(range)->isEmpty())
			thumb.moveBy(Cursor::DeltaF());
	}
	else if (touch_bar_thumb) {
		if (scale_bar.intersects(bar_thumb.movedBy(Cursor::DeltaF().x, 0).left()) and scale_bar.intersects(bar_thumb.movedBy(Cursor::DeltaF().x, 0).right()))
			bar_thumb.moveBy(Cursor::DeltaF().x, 0);
	}

	if (scale_bar.intersects(bar_thumb.movedBy(mouse_sensitivity * int32(Mouse::Wheel()), 0).left())
		and scale_bar.intersects(bar_thumb.movedBy(mouse_sensitivity * int32(Mouse::Wheel()), 0).right()))
	{
		bar_thumb.moveBy(mouse_sensitivity * int32(Mouse::Wheel()), 0);
	}

	camera.jumpTo(camera.getCenter() - sensitivity * (thumb.center - Vec2{ x,y }).asPoint(),
		_scale_func((bar_thumb.centerX() - scale_bar.centerX())*2.0/(scale_bar.w-bar_thumb.w)));
	camera.update(dt);

	scale_bar.draw(ColorF(0.2, 0.2, 0.2));
	bar_thumb.draw(Palette::Gray);
	range.draw(ColorF(0.2, 0.2, 0.2));
	thumb.draw(Palette::Gray);
}

const Transformer2D EditorsCamera::getTransformer2D(bool cameraAffected)const
{
	if (not cameraAffected)return Transformer2D{ Mat3x2::Identity(),TransformCursor::Yes };
	return Transformer2D(camera.createTransformer());
}

double EditorsCamera::_scale_func(double t)
{
	if (t > 0)
	{
		return 1.0 + scale_range * t;
	}
	else
	{
		return 1.0 + (1 - 1.0 / scale_range) * t;
	}
}

void SaveParts::start()
{
	pos = Vec2{ 10,90 };
}

void SaveParts::update(double dt)
{
	if (SimpleGUI::Button(U"保存", pos))
	{
		JSON json;

		Parts* mainParts = nullptr;
		for (const auto& parts : GetComponentArr<Parts>())
			if (parts->params(U"Parent")==U"__Main__") {
				mainParts = parts;
				break;
			}

		if (mainParts == nullptr and System::MessageBoxOK(U"__Main__パーツを設定してください")==MessageBoxResult::OK)return;

		for (const auto& parts : GetComponentArr<Parts>())
		{
			if (parts->PartsParent == nullptr and parts != mainParts)
			{//親がいなかったらメインパーツを親とする
				parts->set_params(U"Parent", mainParts->name);
			}
			for (const auto& elem : JsonElems)
			{
				if (elem.first == U"TexturePath")
				{//テクスチャーパスをStartPathからの相対に
					json[U"Body"][parts->name][elem.first] = parts->startPath + parts->params(elem.first);
					continue;
				}
				else if ((elem.first==U"Scale") or (elem.first == U"Z"))
				{//ScaleやZを数値に変換
					json[U"Body"][parts->name][elem.first] = Parse<double>(parts->params(elem.first));
					continue;
				}
				json[U"Body"][parts->name][elem.first] = parts->params(elem.first);
			}
		}
		Optional<FilePath> path = Dialog::SaveFile({ FileFilter::JSON()});
		if (path)
		{
			json.save(*path);
		}
		else
		{
			System::MessageBoxOK(U"JSONファイルの保存に失敗しました");
		}
	}
}

void ErasePartsOperate::start()
{
	pos = { 10,130 };
}

void ErasePartsOperate::update(double dt)
{
	if (SimpleGUI::Button(U"パーツ削除", pos))
	{
		Parts* p = GetComponent<EditParts>()->get_selectedParts();
		if (p == nullptr)
		{
			System::MessageBoxOK(U"パーツが選択されていません");
			return;
		};
		if (System::MessageBoxOKCancel(U"選択中のパーツを削除しますか？") == MessageBoxResult::OK)
		{
			GetComponent<EditParts>()->releaseParts();
			p->removeSelf<Parts>();
		}
	}

	if (KeyDelete.down() and GetComponent<EditParts>()->get_selectedParts() != nullptr and System::MessageBoxOKCancel(U"選択中のパーツを削除しますか？") == MessageBoxResult::OK)
	{
		Parts* p = GetComponent<EditParts>()->get_selectedParts();
		GetComponent<EditParts>()->releaseParts();
		p->removeSelf<Parts>();
	}
}

void StartMotion::start()
{
	pos = { 10,10 };
}

void StartMotion::update(double dt)
{
	if (SimpleGUI::Button(U"モーション画面へ",pos))
	{
		MyEditor* editor = dynamic_cast<MyEditor*>(Parent);
		editor->working = false;
	}
}
