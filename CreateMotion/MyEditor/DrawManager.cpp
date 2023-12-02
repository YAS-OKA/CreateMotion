#include "../stdafx.h"
#include "DrawManager.h"
#include"Draw.h"
#include"Camera.h"

void DrawManager::setting(Camera* camera, std::function<bool(IDrawing*)> f)
{
	m_camera = camera;
	canDraw = f;
}

void DrawManager::set3D(IDraw3D* drawing)
{
	m_drawings3D << drawing;
}

void DrawManager::set2D(IDraw2D* drawing)
{
	m_drawings2D << drawing;
}

void DrawManager::remove3D(IDraw3D* drawing)
{
	m_drawings3D.remove(drawing);
}

void DrawManager::remove2D(IDraw2D* drawing)
{
	m_drawings2D.remove(drawing);
}

MSRenderTexture DrawManager::getRenderTexture()const
{
	return renderTexture;
}

Camera* DrawManager::getCamera()const
{
	return m_camera;
}

void DrawManager::update()
{
	std::stable_sort(
		m_drawings3D.begin(),
		m_drawings3D.end(),
		[=](const IDraw3D* d1, const IDraw3D* d2) {return d1->distanceFromCamera().length() < d2->distanceFromCamera().length(); }
	);

	std::stable_sort(
		m_drawings2D.begin(),
		m_drawings2D.end(),
		[=](const IDraw2D* d1, const IDraw2D* d2) {return d1->distanceFromCamera().length() < d2->distanceFromCamera().length(); }
	);
}

void DrawManager::draw()const
{
	// 3D シーンにカメラを設定
	Graphics3D::SetCameraTransform(m_camera->getCamera());
	{
		const ScopedRenderTarget3D target{ renderTexture.clear(Palette::Black) };
		for (const auto& drawing : m_drawings3D)drawing->draw();
	}

	// 3D シーンを 2D シーンに描画
	{
		Graphics3D::Flush();
		renderTexture.resolve();
		Shader::LinearToScreen(renderTexture);
	}
	//2Dの描画
	for (const auto& drawing : m_drawings2D)
	{
		drawing->draw();
	}
}
