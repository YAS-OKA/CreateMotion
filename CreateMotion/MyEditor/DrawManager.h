#pragma once

class Camera;
class IDrawing;
class IDraw2D;
class IDraw3D;

class DrawManager
{
private:
	Array<IDraw3D*> m_drawings3D;
	Array<IDraw2D*> m_drawings2D;
	std::function<bool(IDrawing*)> canDraw;
	Camera* m_camera;
	const MSRenderTexture renderTexture{ Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes };
public:
	void setting(Camera* camera, std::function<bool(IDrawing*)> f = [](IDrawing*) {return true; });
	void set3D(IDraw3D* drawing);
	void set2D(IDraw2D* drawing);
	void remove3D(IDraw3D* drawing);
	void remove2D(IDraw2D* drawing);
	MSRenderTexture getRenderTexture()const;
	Camera* getCamera()const;
	virtual void update();
	virtual void draw()const;
};
