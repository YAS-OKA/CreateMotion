﻿#include "../stdafx.h"
#include "CreatMotionsJson.h"
#include"MyEditor.h"

using namespace mj;

void RegisterParts::start()
{
	pos = Vec2{ 10,50 };
}

void RegisterParts::update(double dt)
{
	if (SimpleGUI::Button(U"画像読込み",pos))
	{
		addParts(Dialog::OpenFiles({ FileFilter::AllImageFiles() }));
	}
}

Array<Parts*> RegisterParts::addParts(const Array<FilePath>& path)
{
	Array<Parts*> parts;
	for (const auto& p : path)
	{
		parts<<AddComponent<Parts>(new Parts(p));
	}
	return parts;
}

Parts::Parts(const FilePath& path)
	:path(path),name(FileSystem::BaseName(path))
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

Parts::Parts(HashTable<String,String> param)
	:path(param[U"TexturePath"])
{
	for (const auto& [key, value] : param)
	{
		if (key == U"TexturePath")
		{
			m_params.emplace(key, FileSystem::RelativePath(value, startPath));
			continue;
		}
		m_params.emplace(key, value);
	}
}

//const Circle& Parts::getRotateCenterCircle()
//{
//	return Circle{ absPos() + Parse<Vec2>(params(U"RotateCenter")),3};
//}
//
//const RectF& Parts::get_region()
//{
//	return tex.scaled(Parse<double>(params(U"Scale"))).regionAt(absPos());
//}

Vec2 Parts::absPos() const
{
	return ParentPos+Parse<Vec2>(params(U"Position"));
}

String Parts::params(const String& name)const
{
	if (name == U"name")
	{
		return this->name;
	}
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
	if (name == U"name")
	{
		this->name = param;
		return;
	}

	if (not m_params.contains(name))throw Error{ name + U"がありません。" };

	if (name == U"TexturePath")
	{		
		if (m_params[name]!=param and (System::MessageBoxOKCancel(U"TexturePathを変更しますか？") == MessageBoxResult::OK))
		{
			m_params[name] = param;
			tex = Texture{ FileSystem::FullPath(U"Characters/" + m_params[U"TexturePath"]),TextureDesc::Mipped };
		}
		return;
	}
	else if (name == U"Position")
	{
		m_params[name] = Format(Parse<Vec2>(param));
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
	//RotateCenterだったらparamsで、Positionだったらm_paramsで渡す。
	//二つの違いは相対座標か絶対座標か。気持ち悪いけどこうするしかない...
	if (target == U"RotateCenter")
	{
		set_params(target, Format(Parse<Vec2>(params(target))+delta));
	}
	else {
		set_params(target, Format(Parse<Vec2>(m_params[U"Position"]) + delta));
	}
}

//void Parts::Rotate(double d)
//{
//	rad += d;
//}
//
//void Parts::RotateAt(const Vec2& pos, double d)
//{
//	rad += d;
//	Vec2 d_pos = absPos().rotateAt(pos, d) + absPos().rotateAt(Parse<Vec2>(params(U"RotateCenter")) + absPos(), d);
//	MoveBy(d_pos,)
//}

void Parts::start()
{
	egaku = true;
	rad = 0;
	tex = Texture{ path,TextureDesc::Mipped };
	GetComponent<PartsColliders>()->makeCollider(this, path);
}

void Parts::update(double dt)
{
	//Print << U"                                  " << name << U":" << m_params[U"Position"] << U"," << params(U"Position");
	priority.setDraw(Parse<double>(params(U"Z")));
}

void Parts::draw()const
{
	if (not egaku)return;
	auto t = Transformer2D{ Mat3x2::Rotate(rad,	absPos() + Parse<Vec2>(params(U"RotateCenter")))};
	tex.scaled(Parse<double>(params(U"Scale"))).drawAt(absPos());
}

HitboxParts::HitboxParts(const FilePath& path)
	:Parts(path)
{
	name = U"Hitbox";
}

void HitboxParts::start()
{
	egaku = true;
	rad = 0;
	tex = Texture{ path,TextureDesc::Mipped };
	Image{ Texture{ path,TextureDesc::Mipped }.size(),ColorF{Palette::Red} }.save(path);
	GetComponent<PartsColliders>()->makeCollider(this, path);
	Image{ Texture{ path,TextureDesc::Mipped }.size(),ColorF{0,0,0,0} }.save(path);
	priority.setDraw(Math::Inf);
}

void HitboxParts::update(double dt)
{
	egaku = GetComponent<MakeHitbox>()->setting;
}

void HitboxParts::draw()const
{
	if (not egaku)return;
	Print << 1;
	tex.drawAt(absPos());
}

void RotateCenter::setParts(Parts* p)
{
	parts = p;
}

bool RotateCenter::mouseOver()
{
	return circle.mouseOver();
}

void RotateCenter::releaseParts()
{
	parts = nullptr;
}

void RotateCenter::start()
{
	parts = nullptr;
	circle = { 0,0,3 };
	priority.setDraw(Math::Inf);
}

void RotateCenter::update(double dt)
{
	if (parts == nullptr)return;
	circle.setCenter(parts->absPos() + Parse<Vec2>(parts->params(U"RotateCenter")));
}

void RotateCenter::draw()const
{
	if (parts == nullptr)return;
	circle.draw(Palette::Red);
	circle.drawFrame(0,1, Palette::Darkblue);
}

void PartsColliders::makeCollider(Parts* parts, const String& path)
{
	Image im{ path };
	MultiPolygon polys = Image{ path }.alphaToPolygons();
	polys.moveBy(-im.size() / 2);
	colliders.emplace(parts,polys);
}

bool PartsColliders::mouseOver(Parts* parts)
{
	return colliders[parts].movedBy(parts->absPos())
		.rotateAt(parts->absPos() + Parse<Vec2>(parts->params(U"RotateCenter")),parts->rad)
		.mouseOver();
}

void PartsColliders::removeColliderOf(Parts* parts)
{
	colliders.erase(parts);
}

MultiPolygon PartsColliders::getCollider(Parts* parts)const
{
	return colliders.at(parts).movedBy(parts->absPos()).rotatedAt(parts->absPos() + Parse<Vec2>(parts->params(U"RotateCenter")), parts->rad);
}

void MoveParts::start()
{
	selectedParts = nullptr;
}

void MoveParts::update(double dt)
{
	//カメラの位置や拡大縮小を考慮
	const auto t = GetComponent<EditorsCamera>()->getTransformer2D(true);
	if (selectedParts == nullptr)return;
	selectedParts->MoveBy(U"Position", Cursor::DeltaF());

	moveChiled(Cursor::DeltaF());
}

void MoveParts::moveChiled(const Vec2& delta)
{
	Array<Parts*> nextParts = { selectedParts };
	while (not nextParts.isEmpty())
	{
		for (auto& parts : parts_key_parent[nextParts[0]])
		{
			parts->MoveBy(U"Position", delta);
			nextParts << parts;
		}
		nextParts.pop_front();
	}
}

void MoveParts::select(Parts* parts)
{
	selectedParts = parts;
}

//void RotateParts::start()
//{
//	selectedParts = nullptr;
//}
//
//void RotateParts::update(double dt)
//{
//	const auto t = GetComponent<EditorsCamera>()->getTransformer2D(true);
//	if (selectedParts == nullptr) return;
//	Vec2 center = selectedParts->absPos() + Parse<Vec2>(selectedParts->params(U"RotateCenter"));
//	selectedParts->Rotate((Cursor::PosF() - center).getAngle() - (Cursor::PosF() - Cursor::DeltaF() - center).getAngle());
//	rotateChiled((Cursor::PosF() - center).getAngle() - (Cursor::PosF() - Cursor::DeltaF() - center).getAngle());
//}
//
//void RotateParts::select(Parts* parts)
//{
//	selectedParts = parts;
//}

//void RotateParts::rotateChiled(double delta)
//{
//	Array<Parts*> nextParts = { selectedParts };
//	while (not nextParts.isEmpty())
//	{
//		for (auto& parts : parts_key_parent[nextParts[0]])
//		{
//			parts->MoveBy(U"Position",{parts})
//			nextParts << parts;
//		}
//		nextParts.pop_front();
//	}
//}

void LightUpParts::start()
{
	selectedParts = nullptr;
	thick = 2;
	lineColor = ColorF{ Palette::Cyan };
	priority.setDraw(Math::Inf);
}

void LightUpParts::update(double dt)
{
	if (selectedParts == nullptr)return;
	hitbox = GetComponent<PartsColliders>()->getCollider(selectedParts);
}

void LightUpParts::draw()const
{
	if (selectedParts == nullptr)return;
	hitbox.drawFrame(thick,lineColor);
}

void LightUpParts::select(Parts* parts)
{
	selectedParts = parts;
	hitbox=GetComponent<PartsColliders>()->getCollider(selectedParts);
}

void LightUpParts::releaseParts()
{
	selectedParts = nullptr;
}

void MoveRotateCenter::update(double dt)
{
	//カメラの位置や拡大縮小を考慮
	const auto t = GetComponent<EditorsCamera>()->getTransformer2D(true);
	if (selectedParts != nullptr)
	{
		selectedParts->MoveBy(U"RotateCenter", Cursor::DeltaF());
	}
}

void EditParts::start()
{	
	selectedParts = nullptr;
	x = Scene::Width()-330;
	y = 0;
	w = Scene::Width()-x;
	if (selectedParts == nullptr)texts.emplace(U"name", U"");
	else texts.emplace(U"name", selectedParts->name);

	for (const auto& elem : JsonElems)
	{
		texts.emplace(elem.first, TextEditState());
	}
	font = Font{ 15 };
	//ScaleはRotateCenter中心に変更　まだ対応してないので非表示に
	hiddenList = {U"Position",U"RotateCenter",U"Scale"};
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
	if (selectedParts == nullptr)texts.emplace(U"name", U"");
	else texts.emplace(U"name", selectedParts->name);
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
		if (thumb.movedBy(Cursor::DeltaF()).intersectsAt(range).has_value() and thumb.movedBy(Cursor::DeltaF()).intersectsAt(range)->isEmpty())
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

double EditorsCamera::get_x() { return x; }

double EditorsCamera::get_y() { return y; }

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
		//あたりはんていパーツ
		for (const auto& hitbox : GetComponentArr<HitboxParts>())
		{
			hitbox->set_params(U"Parent", mainParts->name);
			for (const auto& elem : JsonElems)
			{
				if (elem.first == U"TexturePath")
				{//テクスチャーパスをStartPathからの相対に
					json[U"Body"][hitbox->name][elem.first] = hitbox->startPath + hitbox->params(elem.first);
					continue;
				}
				else if ((elem.first == U"Scale") or (elem.first == U"Z"))
				{//ScaleやZを数値に変換
					json[U"Body"][hitbox->name][elem.first] = Parse<double>(hitbox->params(elem.first));
					continue;
				}
				json[U"Body"][hitbox->name][elem.first] = hitbox->params(elem.first);
			}
		}

		Optional<FilePath> path = Dialog::SaveFile({ FileFilter::JSON()});
		if (path)
		{
			json.save(*path);
		}
	}
}

void ErasePartsOperate::start()
{
	double y = 350;
	double d = 55;
	pos1 = { Scene::Width()-145,y};
	pos2 = { Scene::Width()-165,y+d };
}

void ErasePartsOperate::update(double dt)
{
	if (SimpleGUI::Button(U"パーツ削除", pos1))
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
			GetComponent<RotateCenter>()->releaseParts();
			GetComponent<LightUpParts>()->releaseParts();
			GetComponent<PartsColliders>()->removeColliderOf(p);
			if (GetComponent<MakeHitbox>()->setting)p->removeSelf<HitboxParts>();
			else p->removeSelf<Parts>();
		}
	}

	if (SimpleGUI::Button(U"全パーツ削除", pos2))
	{
		if (System::MessageBoxOKCancel(U"パーツを全て削除しますか？") == MessageBoxResult::Cancel)return;
		GetComponent<EditParts>()->releaseParts();
		GetComponent<RotateCenter>()->releaseParts();
		Array<Parts*> parts = GetComponentArr<Parts>();
		for (auto& p : parts)
		{
			GetComponent<PartsColliders>()->removeColliderOf(p);
			p->removeSelf<Parts>();
		}
	}
}

void StartMotion::start()
{
	pos = { 20,Scene::Height() - 55};
}

void StartMotion::update(double dt)
{
	if (SimpleGUI::Button(U"モーションテストへ",pos))
	{
		MyEditor* editor = dynamic_cast<MyEditor*>(Parent);
		editor->working = false;
	}
}

void LoadJson::start()
{
	pos = { 10,10 };
}

void LoadJson::update(double dt)
{
	if (SimpleGUI::Button(U"モデル読み込み", pos))
	{
		Optional<FilePath> path = Dialog::OpenFile({ FileFilter::JSON()});
		if (not path)
		{
			return;
		}
		JSON json = JSON::Load(*path);
		HashTable<String, HashTable<String,String>> parts_params;
		HashTable<String,Parts*>parts;
		for (auto body:json[U"Body"])
		{
			for (auto param:body.value)
			{
				if (param.key == U"Scale" or param.key == U"Z"){
					parts_params[body.key].emplace(param.key, Format(param.value.get<double>()));
				}
				else {
					parts_params[body.key].emplace(param.key, param.value.getString());
				}
			}
			Parts* p = AddComponent<Parts>(new Parts(parts_params[body.key]));
			p->name = body.key;
			parts.emplace(body.key, p);
		}

		_set_parent(parts, parts_params);

		HashTable<String, Vec2> positions;
		for (auto& p : parts) {
			positions.emplace(p.first, _getAbsPos(p.second));
		}

		for (auto& p : parts) {
			p.second->ParentPos = positions[p.first] - Parse<Vec2>(p.second->params(U"Position"));
			p.second->set_params(U"Position", Format(positions[p.first]));
		}
	}
}

void LoadJson::_set_parent(HashTable<String,Parts*> parts, HashTable<String, HashTable<String, String>> JsonTable)
{
	for (auto& p : parts)
	{
		if (parts.contains(JsonTable[p.first][U"Parent"]))
		{
			p.second->PartsParent = parts[JsonTable[p.first][U"Parent"]];
		}
	}
}

Vec2 LoadJson::_getAbsPos(Parts* parts)
{
	if (parts->PartsParent == nullptr)
	{
		return Parse<Vec2>(parts->params(U"Position"));
	}
	return Parse<Vec2>(parts->params(U"Position"))+_getAbsPos(parts->PartsParent);
}

void MakeHitbox::start()
{
	setting = false;
	startPos = nullptr;
	priority.setDraw(Math::Inf);
}

void MakeHitbox::update(double dt)
{
	if (not setting)return;

	auto t = GetComponent<EditorsCamera>()->getTransformer2D(true);

	if (startPos==nullptr and MouseL.pressedDuration().count()>0.2)
	{
		startPos = new Vec2{ Cursor::PosF() };
	}

	if (startPos != nullptr)
	{
		rect = Rect{ startPos->asPoint(),Cursor::Pos() - startPos->asPoint() };
		if (MouseL.up())
		{
			size_t w = rect.w;
			size_t h = rect.h;
			Image image{ w,h,ColorF{Palette::Red,0.3} };
			Optional<FilePath> path = Dialog::SaveFile({ FileFilter::PNG() });
			if (path)
			{
				image.save(*path);
				HitboxParts* hitbox = AddComponent<HitboxParts>(new HitboxParts(*path));
				hitbox->set_params(U"Position", Format(*startPos+rect.size/2));
				hitboxs << hitbox;
			}
			startPos = nullptr;
		}
	}
}

void MakeHitbox::draw()const
{
	if (startPos == nullptr)return;
	rect.draw(ColorF{ Palette::Red,0.3 });
}

void MakeHitbox::releaseParts(Parts* releaseParts)
{
	for (auto it=hitboxs.begin();it!=hitboxs.end();)
	{
		if (releaseParts == *it)
		{
			it = hitboxs.erase(it);
		}
		else it++;
	}
}

void MakeRectF::update(double dt)
{
	if (MouseL.down())
	{
		startPos = Cursor::PosF();
	}
	if(MouseL.pressed())rect = RectF{ startPos,Cursor::PosF()-startPos };
}

RectF MakeRectF::getRect()
{
	return rect;
}
