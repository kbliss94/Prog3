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
	class PointLightDemo;
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
		const struct OrbitalPeriods
		{
			float Sun;
			float Mercury;
			float Venus;
			float Earth;
			float Mars;
			float Jupiter;
			float Saturn;
			float Uranus;
			float Neptune;
			float Pluto;

			OrbitalPeriods() :
				Sun(0.0f), Mercury(1.606f), Venus(1.174f), Earth(1.0f), Mars(0.811f), Jupiter(0.439f), 
				Saturn(0.326f), Uranus(0.228f), Neptune(0.184f), Pluto(0.159f) { }
		};

		const struct RotationalPeriods
		{
			float Sun;
			float Mercury;
			float Venus;
			float Earth;
			float Mars;
			float Jupiter;
			float Saturn;
			float Uranus;
			float Neptune;
			float Pluto;

			RotationalPeriods() :
				Sun(0.0f), Mercury(.0007f), Venus(.0004f), Earth(.10f), Mars(.0532f), Jupiter(2.732f),
				Saturn(2.1746f), Uranus(.5595f), Neptune(.578f), Pluto(.0028f) { }
		};

		const struct AxialTilts
		{
			float Sun;
			float Mercury;
			float Venus;
			float Earth;
			float Mars;
			float Jupiter;
			float Saturn;
			float Uranus;
			float Neptune;
			float Pluto;

			AxialTilts() :
				Sun(0.0f), Mercury(0.0f), Venus(3.096f), Earth(0.410f), Mars(0.436f), Jupiter(0.052f),
				Saturn(0.471f), Uranus(1.709f), Neptune(0.517f), Pluto(2.129f) { }
		};

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
		//std::shared_ptr<Library::Grid> mGrid;

		//std::shared_ptr<PointLightDemo> mMercury;
		//std::shared_ptr<PointLightDemo> mVenus;
		//std::shared_ptr<PointLightDemo> mEarth;
		//std::shared_ptr<PointLightDemo> mMars;
		//std::shared_ptr<PointLightDemo> mJupiter;
		//std::shared_ptr<PointLightDemo> mSaturn;
		//std::shared_ptr<PointLightDemo> mUranus;
		//std::shared_ptr<PointLightDemo> mNeptune;
		//std::shared_ptr<PointLightDemo> mPluto;
		std::shared_ptr<PointLightDemo> mSun;

		std::shared_ptr<CelestialBodies> mMercury;

		OrbitalPeriods mOrbitalPeriods;
		RotationalPeriods mRotationalPeriods;
		AxialTilts mAxialTilts;

		const std::wstring mTextureFilenames[NumberOfPlanets + 1]
		{
			L"Content\\Textures\\MercuryComposite.dds",
			L"Content\\Textures\\VenusComposite.dds",
			L"Content\\Textures\\EarthComposite.dds",
			L"Content\\Textures\\MarsComposite.dds",
			L"Content\\Textures\\JupiterComposite.dds",
			L"Content\\Textures\\SaturnComposite.dds",
			L"Content\\Textures\\UranusComposite.dds",
			L"Content\\Textures\\NeptuneComposite.dds",
			L"Content\\Textures\\PlutoComposite.dds",
			L"Content\\Textures\\SunComposite.dds"
		};

		const std::wstring mSpecularFilenames[2]
		{
			L"Content\\Textures\\EarthSpecularMap.png",
			L"Content\\Textures\\MarsSpecularMap.png"
		};
	};
}
