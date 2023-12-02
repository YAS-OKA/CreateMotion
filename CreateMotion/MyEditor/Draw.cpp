#include "../stdafx.h"
#include "Draw.h"
#include"Transform.h"
#include"DrawManager.h"

IDrawing::IDrawing(DrawManager* manager)
	:manager(manager)
{
}

void IDrawing::start()
{
	transform=owner->getComponent<Transform>();
}

Vec3 IDrawing::getDrawPos() const
{
	Vec3 localPos = transformScaleAffectable?relative*transform->getAspect():relative;
	//回転
	auto q1 = direction.accum;

	auto rq = q1.inverse();

	if (transformDirectionAffectable) {
		q1 = q1 * transform->get2Direction().accum;
	}
	localPos = rq * localPos;//基準に戻す
	localPos = q1 * localPos;//目的の方向を向かせる

	return transform->getPos() + localPos;
}

Vec3 IDrawing::distanceFromCamera()const
{
	return getDrawPos() - manager->getCamera()->transform->getPos();
}

DrawTexture3D::DrawTexture3D(DrawManager* manager, const MeshData& data)
	:IDraw3D(manager)
	,mesh(data)
{
}

void DrawTexture3D::setAssetName(const String& name)
{
	assetName = name;
}

void DrawTexture3D::draw()const
{
	mesh.draw(
		getDrawPos()
		, transform->get2Direction().accum
		, TextureAsset(assetName)
		);
}

Billboard::Billboard(DrawManager* manager)
	:DrawTexture3D(manager, MeshData::Billboard({ 1,1 }))
{}

void Billboard::draw()const
{
	auto size = TextureAsset(assetName).size();
	double x = size.x;
	double y = size.y;
	auto max = Max(x, y);
	Vec2 aspect_ = size / max;
		mesh.draw(
			manager->getCamera()->getCamera().billboard(getDrawPos(), aspect_ * transform->getAspect().xy())
		, TextureAsset(assetName)
		, color
	);
}

//#include"Transform.h"
//#include"../GameObject/Camera.h"
//#include"MakeObject.h"
//#include"../GameObject/Object.h"
//
//using namespace snk;
//
//void DrawTexture::start()
//{
//	transform = Parent->GetComponent<Transform>();
//}
//
//void DrawTexture::update(double dt)
//{
//	RectF region = TextureAsset(*name).scaled(baseScale * scale).region();
//
//	Vec2 p = pos;
//	if (center)p += Vec2{ TextureAsset(*name).size() * baseScale * scale / 2 };
//
//	region.moveBy(transform->getPos() - p);
//
//	double _rad = affectedByTransformsDirection ? transform->getDirection() + rad : rad;
//	if (region.rotatedAt(transform->getPos() + rotateCenter, _rad).intersects(gameCamera->getRegion()))
//	{
//		inCamera = true;
//	}
//	else
//	{
//		inCamera=false;
//	}
//}
//
//void DrawTexture::draw()const
//{
//	if (not (name and visible and inCamera))return;
//
//	Vec2 p = pos;
//	if (center)p += Vec2{ TextureAsset(*name).size() * baseScale * scale / 2 };
//	double _rad = affectedByTransformsDirection ? transform->getDirection() + rad : rad;
//	TextureAsset(*name)
//		.scaled(baseScale * scale)
//		.rotatedAt(p + rotateCenter, _rad)
//		.draw(-p + transform->getPos(), color);
//}
//
//void DrawTexture::setTexture(const String& assetName, double s, double r)
//{
//	name = assetName;
//	baseScale = s;
//	rad = r;
//}
//
//void DrawTexture::setPos(const Vec2& pos, bool center)
//{
//	DrawTexture::center = center;
//	DrawTexture::pos = pos;
//}
//
//void DrawTexture::setAssetName(const String& assetName)
//{
//	name = assetName;
//}
//
//void DrawFunc::draw()const
//{
//	drawingFunction(this);
//}
//
//void DrawFigure::start()
//{
//	transform = Parent->GetComponent<Transform>();
//}
//
//void DrawFigure::draw()const
//{
//	/*const Transformer2D t1{ Mat3x2::Scale(scale,transform->getPos()+pos),TransformCursor::Yes};
//	const Transformer2D t0{ Mat3x2::Translate(transform->getPos()+pos*scale),TransformCursor::Yes};
//	const Transformer2D t2{ Mat3x2::Rotate(transform->getDirection() + rad,Vec2{0,0})};
//	fig.draw(color);*/
//	if (not visible)return;
//
//	auto f = fig.scaledAt(pos, scale)
//		.movedBy(transform->getPos() + pos * scale)
//		.rotatedAt(transform->getPos() + (pos + rotateCenter) * scale, transform->getDirection() + rad);
//	f.draw(color);
//	if (drawFrame)f.drawFrame(frameThick, frameColor);
//}
//
//void DrawFigure::setFigure(const Figure& fig, ColorF color, double scale, double rad, Vec2 pos, Vec2 rotateCenter)
//{
//	DrawFigure::fig = fig;
//	DrawFigure::color = color;
//	DrawFigure::scale = scale;
//	DrawFigure::rad = rad;
//	DrawFigure::pos = pos;
//	DrawFigure::rotateCenter = rotateCenter;
//}
//
//void DrawFigures::start()
//{
//	transform = Parent->GetComponent<Transform>();
//	parent = dynamic_cast<Object*>(Parent);
//}
//
//void DrawFigures::update(double dt)
//{
//	for (auto& fig : figures)
//	{
//		fig.second->GetComponent<ObjectBehavior>()->MainObject->z = parent->z;
//	}
//}
//
//DrawFigure* DrawFigures::addFigure(const String& name, const Figure& fig, const ColorF& color, double scale, double rad, Vec2 pos, Vec2 rotateCenter)
//{
//	if (figures.contains(name))
//	{
//		//名前にナンバリング
//		int32 k = 1;
//		while (figures.contains(name + Format(k)))++k;		
//		return setFigure(name+Format(k), fig, color, scale, rad, pos, rotateCenter);
//	}
//	else
//	{
//		return setFigure(name, fig, color, scale, rad, pos, rotateCenter);
//	}
//};
//
//DrawFigure* DrawFigures::setFigure(const String& name, const Figure& fig, const ColorF& color, double scale, double rad, Vec2 pos, Vec2 rotateCenter)
//{
//	auto obj = (new Object(parent->objectManager))->AddComponentImmediately<ObjectBehavior>();
//	obj->transform->setPos(transform->getPos()+pos);
//	obj->makeParent(GetComponent<ObjectBehavior>());
//	auto figure = obj->AddComponentImmediately<DrawFigure>();
//	figure->setFigure(fig, color, scale, rad, {0,0}, rotateCenter);
//	figures[name] = figure;
//	return figure;
//};
//
//void DrawFigures::clearFigures()
//{
//	for (auto& fig : figures)fig.second->GetComponent<ObjectBehavior>()->MainObject->eraseThis();
//	figures.clear();
//}
//
//void DrawFont::start()
//{
//	transform = Parent->GetComponent<Transform>();
//}
//
//void DrawFont::draw()const
//{
//	if (not (name and text and visible))return;
//
//	RectF region = center ? FontAsset(*name)(*text).regionAt(transform->getPos() + pos) : FontAsset(*name)(*text).region(transform->getPos() + pos);
//
//	double _rad = affectedByTransformsDirection ? transform->getDirection() + rad : rad;
//
//	if (region.rotatedAt(transform->getPos() +pos+ rotateCenter, _rad).intersects(gameCamera->getRegion()))
//	{
//		auto t = Transformer2D{ Mat3x2::Scale(baseScale*scale,transform->getPos() + pos),TransformCursor::Yes };
//		auto t1 = Transformer2D{ Mat3x2::Rotate(_rad,transform->getPos() + pos + rotateCenter),TransformCursor::Yes };
//		center?FontAsset(*name)(*text).drawAt(transform->getPos()+pos,color): FontAsset(*name)(*text).draw(transform->getPos() + pos, color);
//	}
//}
//
//void DrawFont::setAssetName(const String& fontAsset)
//{
//	name = fontAsset;
//	if (text)
//	{
//		FontAsset(*name).preload(*text);
//	}
//}
//
//void DrawFont::setPos(const Vec2& pos, bool center)
//{
//	DrawFont::center = center;
//	DrawFont::pos = pos;
//}
//
//void DrawFont::setText(const String& _text)
//{
//	text = _text;
//
//	if (name)
//	{
//		FontAsset(*name).preload(_text);
//	}
//}
