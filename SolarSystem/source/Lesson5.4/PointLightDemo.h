#pragma once

#include "DrawableGameComponent.h"
#include "RenderStateHelper.h"
#include "PointLight.h"
#include <DirectXMath.h>
#include <DirectXColors.h>

#include "CelestialBodies.h"

namespace Library
{
	class Mesh;
	class ProxyModel;
	class KeyboardComponent;	
}

namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}

namespace Rendering
{
	class PointLightDemo final : public Library::DrawableGameComponent
	{
		RTTI_DECLARATIONS(PointLightDemo, Library::DrawableGameComponent)

	public:
		//PointLightDemo(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);
		PointLightDemo(Library::Game& game, const std::shared_ptr<Library::Camera>& camera, float orbitRadius, float scale, float orbPer, float rotPer, float axTilt, std::wstring texFilename, std::wstring specFilename);

		bool AnimationEnabled() const;
		void SetAnimationEnabled(bool enabled);

		virtual void Initialize() override;
		virtual void Update(const Library::GameTime& gameTime) override;
		virtual void Draw(const Library::GameTime& gameTime) override;

	private:
		struct VSCBufferPerFrame
		{
			DirectX::XMFLOAT3 LightPosition;
			float LightRadius;

			VSCBufferPerFrame() :
				LightPosition(Library::Vector3Helper::Zero), LightRadius(100000.0f) { }
			VSCBufferPerFrame(const DirectX::XMFLOAT3 lightPosition, float lightRadius) :
				LightPosition(lightPosition), LightRadius(lightRadius) { }
		};

		struct VSCBufferPerObject
		{
			DirectX::XMFLOAT4X4 WorldViewProjection;
			DirectX::XMFLOAT4X4 World;

			VSCBufferPerObject() = default;
			VSCBufferPerObject(const DirectX::XMFLOAT4X4& wvp, const DirectX::XMFLOAT4X4& world) :
				WorldViewProjection(wvp), World(world) { }
		};

		struct PSCBufferPerFrame
		{
			DirectX::XMFLOAT3 CameraPosition;
			float Padding;
			DirectX::XMFLOAT3 AmbientColor;
			float Padding2;
			DirectX::XMFLOAT3 LightPosition;
			float Padding3;
			DirectX::XMFLOAT3 LightColor;
			float Padding4;

			PSCBufferPerFrame() :
				CameraPosition(Library::Vector3Helper::Zero), AmbientColor(Library::Vector3Helper::Zero),
				LightPosition(Library::Vector3Helper::Zero), LightColor(Library::Vector3Helper::Zero)
			{
			}

			PSCBufferPerFrame(const DirectX::XMFLOAT3& cameraPosition, const DirectX::XMFLOAT3& ambientColor, const DirectX::XMFLOAT3& lightPosition, const DirectX::XMFLOAT3& lightColor) :
				CameraPosition(cameraPosition), AmbientColor(ambientColor),
				LightPosition(lightPosition), LightColor(lightColor)
			{
			}
		};

		struct PSCBufferPerObject
		{
			DirectX::XMFLOAT3 SpecularColor;
			float SpecularPower;

			PSCBufferPerObject() :
				SpecularColor(1.0f, 1.0f, 1.0f), SpecularPower(128.0f) { }

			PSCBufferPerObject(const DirectX::XMFLOAT3& specularColor, float specularPower) :
				SpecularColor(specularColor), SpecularPower(specularPower) { }
		};

		void CreateVertexBuffer(const Library::Mesh& mesh, ID3D11Buffer** vertexBuffer) const;
		void ToggleAnimation();
		void UpdateAmbientLight(const Library::GameTime& gameTime);
		void UpdatePointLight(const Library::GameTime& gameTime);
		void UpdateSpecularLight(const Library::GameTime& gameTime);
				
		static const float LightModulationRate;
		static const float LightMovementRate;
		static const float SunAmbientColor;
		static const float PlanetAmbientColor;

		float mOrbitalDistance;
		float mScale;
		float mOrbitalPeriod;
		float mRotationalPeriod;
		std::wstring mTextureFilename;
		std::wstring mSpecularFilename;

		float mAxialAngle;
		float mOrbitalAngle;
		float mAxialTilt;

		static const int NumCelestialBodies = 11;
		static const int MoonIndex = NumCelestialBodies - 1;
		static const int EarthIndex = 3;
		static const float DistanceMultiplier;

		PSCBufferPerFrame mPSCBufferPerFrameData;
		DirectX::XMFLOAT4X4 mWorldMatrix;
		VSCBufferPerFrame mVSCBufferPerFrameData;
		VSCBufferPerObject mVSCBufferPerObjectData;		
		PSCBufferPerObject mPSCBufferPerObjectData;		
		Library::PointLight mPointLight;
		Library::RenderStateHelper mRenderStateHelper;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mIndexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVSCBufferPerFrame;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVSCBufferPerObject;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPSCBufferPerFrame;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPSCBufferPerObject;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mColorTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mSpecularMap;
		std::unique_ptr<Library::ProxyModel> mProxyModel;
		Library::KeyboardComponent* mKeyboard;
		std::uint32_t mIndexCount;
		std::unique_ptr<DirectX::SpriteBatch> mSpriteBatch;
		std::unique_ptr<DirectX::SpriteFont> mSpriteFont;
		DirectX::XMFLOAT2 mTextPosition;
		bool mAnimationEnabled;

		//std::shared_ptr<CelestialBodies> mMercury;

		std::vector<std::shared_ptr<CelestialBodies>> mCelestialBodies;

		//orbital periods in years
		//std::vector<float> mOrbitalVelocities = { 0.0f, .241f, .616f, 1.0f, 1.88f, 11.86f, 29.41f, 84.04f, 163.72f, 247.93f, .074f };

		//rotational periods in years
		//std::vector<float> mRotationalVelocities = { 0.0f, .161f, .666f, .003f, .003f, .001f, .001f, .002f, .002f, .017f, .074f };

		std::vector<float> mOrbitRadii = { 0.0f, .387f, .723f, 1.0f, 1.524f, 5.203f, 9.582f, 19.2f, 30.05f, 39.48f, .2f };//.003f };
		std::vector<float> mScales = { 1.0f, .382f, .949f, 1.0f, .532f, 11.19f, 9.26f, 4.01f, 3.88f, .18f, .272f };
		std::vector<float> mOrbitalVelocities = { 0.0f, 1.606f, 1.174f, 1.0f, .811f, .439f, .326f, .228f, .184f, .159f, .035f };
		////std::vector<float> mRotationalVelocities = { 0.0f, .0007f, .0004f, .10f, .0532f, 2.732f, 2.1746f, .5595f, .578f, .0028f, .01f };
		std::vector<float> mRotationalVelocities = { 0.0f, .007f, .004f, 1.0f, .532f, 27.32f, 21.746f, 5.595f, 5.78f, .028f, .01f };
		std::vector<float> mAxialTilts = { 0.0f, 0.0f, 3.096f, .410f, .436f, .052f, .471f, 1.709f, .517f, 2.129f, .026f };
		std::vector<std::wstring> mTextureFilenames = 
		{
			L"Content\\Textures\\SunComposite.dds",
			L"Content\\Textures\\MercuryComposite.dds",
			L"Content\\Textures\\VenusComposite.dds",
			L"Content\\Textures\\EarthComposite.dds",
			L"Content\\Textures\\MarsComposite.dds",
			L"Content\\Textures\\JupiterComposite.dds",
			L"Content\\Textures\\SaturnComposite.dds",
			L"Content\\Textures\\UranusComposite.dds",
			L"Content\\Textures\\NeptuneComposite.dds",
			L"Content\\Textures\\PlutoComposite.dds",
			L"Content\\Textures\\MoonComposite.dds"
		};

		std::wstring mSpecularFilenames = L"Content\\Textures\\MarsSpecularMap.png";
	};
}
