#pragma once
#include"EC.hpp"
#include"Transform.h"
#include"Camera.h"

class DrawManager;
class Camera;

class IDrawing:public Component
{
public:
	bool visible = true;
	//transformの影響
	bool transformDirectionAffectable = true;
	bool transformScaleAffectable = true;

	ColorF color = Palette::White;
	//xyz比
	Transform::Scale aspect;

	Transform* transform;
	//相対座標
	Vec3 relative{ 0,0,0 };

	Transform::Direction direction;

	Vec3 rotateCenter{ 0,0,0 };

	DrawManager* manager;

	IDrawing(DrawManager* manager);

	virtual ~IDrawing() {};

	virtual void start();

	virtual void draw()const = 0;

	Vec3 getDrawPos()const;

	Vec3 distanceFromCamera()const;
};

#include"DrawManager.h"
//3D描画
class IDraw3D :public IDrawing
{
public:
	using IDrawing::IDrawing;

	void start() override {
		IDrawing::start();
		manager->set3D(this);
	};

	virtual void draw()const = 0;
};
//2D描画
class IDraw2D :public IDrawing
{
public:
	using IDrawing::IDrawing;

	//カメラに影響される度合い まだ使ってない
	double cameraInfluence = 1.0;

	void start() override{
		IDrawing::start();
		manager->set2D(this);
	};

	virtual void draw()const = 0;
};

class DrawTexture3D :public IDraw3D
{
public:
	Mesh mesh{};
	String assetName{};

	DrawTexture3D(DrawManager* manager, const MeshData& data);

	virtual void setAssetName(const String& name);

	virtual void draw()const override;
};

class Billboard final :public DrawTexture3D
{
public:
	Camera* camera;

	Billboard(DrawManager* manager);

	void draw()const override;
};


template<class Drawing2D>
class Draw2D :public IDraw2D
{
public:
	Drawing2D drawing;
	template<class... Args>
	Draw2D(DrawManager* manager, Args&&... args)
		:IDraw2D(manager)
		, drawing(args...)
	{}

	virtual void draw()const override
	{
		if (not visible)return;
		Vec2 aspect_ = (transformScaleAffectable ? transform->getAspect() * aspect.aspect.vec : aspect.aspect.vec).xy();
		double rotation = transformDirectionAffectable ? transform->getDirection().xy().getAngle() - Vec2{ 1,0 }.getAngle() + direction.vector.xy().getAngle() : direction.vector.xy().getAngle();
		rotation -= Vec2{ 1,0 }.getAngle();

		const Transformer2D t0{ Mat3x2::Translate(getDrawPos().xy()),TransformCursor::Yes };
		const Transformer2D t1{ Mat3x2::Rotate(rotation,rotateCenter.xy()),TransformCursor::Yes };
		const Transformer2D t2{ Mat3x2::Scale(aspect_),TransformCursor::Yes };
		drawing.draw(color);
	};
};

using Draw2DRect = Draw2D<Rect>;
using Draw2DRectF = Draw2D<RectF>;
using Draw2DCircle = Draw2D<Circle>;
using Draw2DTexture = Draw2D<Texture>;
