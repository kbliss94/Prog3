#pragma once

#include "Game.h"
#include "RenderStateHelper.h"
#include <windows.h>
#include <functional>

namespace Library
{
	class KeyboardComponent;
	class MouseComponent;
	class GamePadComponent;
	class FpsComponent;
	class Camera;
	class Grid;
}

namespace Rendering
{
	class SolarSystem;
	class CelestialBodies;

	class RenderingGame final : public Library::Game
	{
	public:
		RenderingGame(std::function<void*()> getWindowCallback, std::function<void(SIZE&)> getRenderTargetSizeCallback);

		virtual void Initialize() override;
		virtual void Update(const Library::GameTime& gameTime) override;
		virtual void Draw(const Library::GameTime& gameTime) override;
		virtual void Shutdown() override;

		void Exit();

	private:
		float SunOrbitalVelocity = 0.0f;
		float SunRotationalVelocity = 0.0f;
		float SunAxialTilt = 0.0f;

		static const DirectX::XMVECTORF32 BackgroundColor;
		static const float DistanceMultiplier;
		static const float OrbitalPeriodMultipler;
		static const int NumberOfPlanets = 9;

		Library::RenderStateHelper mRenderStateHelper;
		std::shared_ptr<Library::KeyboardComponent> mKeyboard;
		std::shared_ptr<Library::MouseComponent> mMouse;
		std::shared_ptr<Library::GamePadComponent> mGamePad;
		std::shared_ptr<Library::FpsComponent> mFpsComponent;
		std::shared_ptr<Library::Camera> mCamera;
		std::shared_ptr<SolarSystem> mSolarSystem;

		const std::wstring mSunTextureFilename = L"Content\\Textures\\SunComposite.dds";
		const std::wstring mSunSpecularFilename = L"Content\\Textures\\MarsSpecularMap.png";
	};
}
