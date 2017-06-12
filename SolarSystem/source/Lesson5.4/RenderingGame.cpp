#include "pch.h"

using namespace std;
using namespace DirectX;
using namespace Library;

namespace Rendering
{
	//const XMVECTORF32 RenderingGame::BackgroundColor = Colors::CornflowerBlue;
	const XMVECTORF32 RenderingGame::BackgroundColor = Colors::Black;
	const float RenderingGame::DistanceMultiplier = 50.0f;
	const float RenderingGame::OrbitalPeriodMultipler = 0.1f;

	RenderingGame::RenderingGame(std::function<void*()> getWindowCallback, std::function<void(SIZE&)> getRenderTargetSizeCallback) :
		Game(getWindowCallback, getRenderTargetSizeCallback), mRenderStateHelper(*this), mOrbitalPeriods(), mRotationalPeriods()
	{
	}

	void RenderingGame::Initialize()
	{
		RasterizerStates::Initialize(mDirect3DDevice.Get());
		SamplerStates::Initialize(mDirect3DDevice.Get());

		mKeyboard = make_shared<KeyboardComponent>(*this);
		mComponents.push_back(mKeyboard);
		mServices.AddService(KeyboardComponent::TypeIdClass(), mKeyboard.get());

		mMouse = make_shared<MouseComponent>(*this);
		mComponents.push_back(mMouse);
		mServices.AddService(MouseComponent::TypeIdClass(), mMouse.get());

		mGamePad = make_shared<GamePadComponent>(*this);
		mComponents.push_back(mGamePad);
		mServices.AddService(GamePadComponent::TypeIdClass(), mGamePad.get());

		mCamera = make_shared<FirstPersonCamera>(*this);
		mComponents.push_back(mCamera);
		mServices.AddService(Camera::TypeIdClass(), mCamera.get());

		//mGrid = make_shared<Grid>(*this, mCamera);
		//mComponents.push_back(mGrid);

		mMercury = make_shared<PointLightDemo>(*this, mCamera, (.387f * DistanceMultiplier), .382f, mOrbitalPeriods.Mercury, 
			mRotationalPeriods.Mercury, mTextureFilenames[0], mSpecularFilenames[1]);
		mComponents.push_back(mMercury);

		mVenus = make_shared<PointLightDemo>(*this, mCamera, (.723f * DistanceMultiplier), .949f, mOrbitalPeriods.Venus, 
			mRotationalPeriods.Venus, mTextureFilenames[1], mSpecularFilenames[1]);
		mComponents.push_back(mVenus);

		mEarth = make_shared<PointLightDemo>(*this, mCamera, (1.0f * DistanceMultiplier), 1.0f, mOrbitalPeriods.Earth, 
			mRotationalPeriods.Earth, mTextureFilenames[2], mSpecularFilenames[1]);
		mComponents.push_back(mEarth);

		mMars = make_shared<PointLightDemo>(*this, mCamera, (1.524f * DistanceMultiplier), .532f, mOrbitalPeriods.Mars, 
			mRotationalPeriods.Mars, mTextureFilenames[3], mSpecularFilenames[1]);
		mComponents.push_back(mMars);

		mJupiter = make_shared<PointLightDemo>(*this, mCamera, (5.203f * DistanceMultiplier), 11.19f, mOrbitalPeriods.Jupiter, 
			mRotationalPeriods.Jupiter, mTextureFilenames[4], mSpecularFilenames[1]);
		mComponents.push_back(mJupiter);

		mSaturn = make_shared<PointLightDemo>(*this, mCamera, (9.582f * DistanceMultiplier), 9.26f, mOrbitalPeriods.Saturn, 
			mRotationalPeriods.Saturn, mTextureFilenames[5], mSpecularFilenames[1]);
		mComponents.push_back(mSaturn);

		mUranus = make_shared<PointLightDemo>(*this, mCamera, (19.2f * DistanceMultiplier), 4.01f, mOrbitalPeriods.Uranus, 
			mRotationalPeriods.Uranus, mTextureFilenames[6], mSpecularFilenames[1]);
		mComponents.push_back(mUranus);

		mNeptune = make_shared<PointLightDemo>(*this, mCamera, (30.05f * DistanceMultiplier), 3.88f, mOrbitalPeriods.Neptune, 
			mRotationalPeriods.Neptune, mTextureFilenames[7], mSpecularFilenames[1]);
		mComponents.push_back(mNeptune);

		mPluto = make_shared<PointLightDemo>(*this, mCamera, (39.48f * DistanceMultiplier), .18f, mOrbitalPeriods.Pluto, 
			mRotationalPeriods.Pluto, mTextureFilenames[8], mSpecularFilenames[1]);
		mComponents.push_back(mPluto);

		Game::Initialize();

		mFpsComponent = make_shared<FpsComponent>(*this);
		mFpsComponent->Initialize();

		mCamera->SetPosition(0.0f, 2.5f, 25.0f);
	}

	void RenderingGame::Update(const GameTime &gameTime)
	{
		mFpsComponent->Update(gameTime);

		if (mKeyboard->WasKeyPressedThisFrame(Keys::Escape) || mGamePad->WasButtonPressedThisFrame(GamePadButtons::Back))
		{
			Exit();
		}

		if (mKeyboard->WasKeyPressedThisFrame(Keys::G) || mGamePad->WasButtonPressedThisFrame(GamePadButtons::DPadUp))
		{
			//mGrid->SetEnabled(!mGrid->Enabled());
			//mGrid->SetVisible(!mGrid->Visible());
		}

		Game::Update(gameTime);
	}

	void RenderingGame::Draw(const GameTime &gameTime)
	{
		mDirect3DDeviceContext->ClearRenderTargetView(mRenderTargetView.Get(), reinterpret_cast<const float*>(&BackgroundColor));
		mDirect3DDeviceContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		Game::Draw(gameTime);

		mRenderStateHelper.SaveAll();
		mFpsComponent->Draw(gameTime);
		mRenderStateHelper.RestoreAll();

		HRESULT hr = mSwapChain->Present(1, 0);

		// If the device was removed either by a disconnection or a driver upgrade, we must recreate all device resources.
		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			HandleDeviceLost();
		}
		else
		{
			ThrowIfFailed(hr, "IDXGISwapChain::Present() failed.");
		}
	}

	void RenderingGame::Shutdown()
	{
		SamplerStates::Shutdown();
		RasterizerStates::Shutdown();
	}

	void RenderingGame::Exit()
	{
		PostQuitMessage(0);
	}
}