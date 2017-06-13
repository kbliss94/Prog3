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
	class SolarSystem final : public Library::DrawableGameComponent
	{
		RTTI_DECLARATIONS(SolarSystem, Library::DrawableGameComponent)

	public:
		SolarSystem(Library::Game& game, const std::shared_ptr<Library::Camera>& camera, float orbitRadius, float scale, float orbPer, float rotPer, float axTilt, std::wstring texFilename, std::wstring specFilename);

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

		struct CelestialBodyData
		{
			std::string Name;
			float OrbitRadius;
			float Scale;
			float OrbitalPeriod;
			float RotationalPeriod;
			float AxialTilt;
			std::wstring TextureFilename;
			std::wstring SpecularFilename;
			CelestialBodies* Parent;

			CelestialBodyData() = default;
			CelestialBodyData(const std::string& name, float orbitRad, float scale, float orbPer, float rotPer, float axialTilt, std::wstring texFile, std::wstring specFile, CelestialBodies* parent) :
				Name(name), OrbitRadius(orbitRad), Scale(scale), OrbitalPeriod(orbPer), RotationalPeriod(rotPer), AxialTilt(axialTilt), TextureFilename(texFile),
				SpecularFilename(specFile), Parent(parent) { };
		};

		void CreateVertexBuffer(const Library::Mesh& mesh, ID3D11Buffer** vertexBuffer) const;
		void ToggleAnimation();
				
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

		static const int NumCelestialBodies = 10;
		static const int MoonIndex = NumCelestialBodies - 1;
		static const int EarthIndex = 2;
		static const float DistanceMultiplier;
		static const float SpeedFactor;

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

		std::vector<std::shared_ptr<CelestialBodies>> mCelestialBodies;
		std::vector<std::shared_ptr<CelestialBodyData>> mCelestialBodyDataList;

		CelestialBodyData Mercury =
		{
			"Mercury",									//Name
			.387f,										//Orbit radius
			.382f,										//Scale
			.241f,										//Orbital period (yrs)
			.161f,										//Rotational period (yrs)
			0.0f,										//Axial tilt (radians)
			L"Content\\Textures\\MercuryComposite.dds",	//Texture filename
			L"Content\\Textures\\MarsSpecularMap.png",	//Specular filename
			nullptr										//Parent
		};

		CelestialBodyData Venus =
		{
			"Venus",									//Name
			.723f,										//Orbit radius
			.949f,										//Scale
			.616f,										//Orbital period (yrs)
			.666f,										//Rotational period (yrs)
			3.096f,										//Axial tilt (radians)
			L"Content\\Textures\\VenusComposite.dds",	//Texture filename
			L"Content\\Textures\\MarsSpecularMap.png",	//Specular filename
			nullptr										//Parent
		};

		CelestialBodyData Earth =
		{
			"Earth",									//Name
			1.0f,										//Orbit radius
			1.0f,										//Scale
			1.0f,										//Orbital period (yrs)
			.003f,										//Rotational period (yrs)
			.410f,										//Axial tilt (radians)
			L"Content\\Textures\\EarthComposite.dds",	//Texture filename
			L"Content\\Textures\\MarsSpecularMap.png",	//Specular filename
			nullptr										//Parent
		};

		CelestialBodyData Mars =
		{
			"Mars",										//Name
			1.524f,										//Orbit radius
			.532f,										//Scale
			1.88f,										//Orbital period (yrs)
			.003f,										//Rotational period (yrs)
			.436f,										//Axial tilt (radians)
			L"Content\\Textures\\MarsComposite.dds",	//Texture filename
			L"Content\\Textures\\MarsSpecularMap.png",	//Specular filename
			nullptr										//Parent
		};

		CelestialBodyData Jupiter =
		{
			"Jupiter",									//Name
			5.203f,										//Orbit radius
			11.19f,										//Scale
			11.86f,										//Orbital period (yrs)
			.001f,										//Rotational period (yrs)
			.052f,										//Axial tilt (radians)
			L"Content\\Textures\\JupiterComposite.dds",	//Texture filename
			L"Content\\Textures\\MarsSpecularMap.png",	//Specular filename
			nullptr										//Parent
		};

		CelestialBodyData Saturn =
		{
			"Saturn",									//Name
			9.582f,										//Orbit radius
			9.26f,										//Scale
			29.410f,									//Orbital period (yrs)
			.001f,										//Rotational period (yrs)
			.471f,										//Axial tilt (radians)
			L"Content\\Textures\\SaturnComposite.dds",	//Texture filename
			L"Content\\Textures\\MarsSpecularMap.png",	//Specular filename
			nullptr										//Parent
		};

		CelestialBodyData Uranus =
		{
			"Uranus",									//Name
			19.2f,										//Orbit radius
			4.01f,										//Scale
			84.04f,										//Orbital period (yrs)
			.002f,										//Rotational period (yrs)
			1.709f,										//Axial tilt (radians)
			L"Content\\Textures\\UranusComposite.dds",	//Texture filename
			L"Content\\Textures\\MarsSpecularMap.png",	//Specular filename
			nullptr										//Parent
		};

		CelestialBodyData Neptune =
		{
			"Neptune",									//Name
			30.05f,										//Orbit radius
			3.88f,										//Scale
			163.72f,									//Orbital period (yrs)
			.002f,										//Rotational period (yrs)
			.517f,										//Axial tilt (radians)
			L"Content\\Textures\\NeptuneComposite.dds",	//Texture filename
			L"Content\\Textures\\MarsSpecularMap.png",	//Specular filename
			nullptr										//Parent
		};

		CelestialBodyData Pluto =
		{
			"Pluto",									//Name
			39.48f,										//Orbit radius
			.18f,										//Scale
			247.93f,									//Orbital period (yrs)
			.017f,										//Rotational period (yrs)
			2.129f,										//Axial tilt (radians)
			L"Content\\Textures\\PlutoComposite.dds",	//Texture filename
			L"Content\\Textures\\MarsSpecularMap.png",	//Specular filename
			nullptr
		};

		CelestialBodyData Moon =
		{
			"Moon",										//Name
			.2f,										//Orbit radius
			.272f,										//Scale
			.074f,										//Orbital period (yrs)
			.074f,										//Rotational period (yrs)
			.026f,										//Axial tilt (radians)
			L"Content\\Textures\\MoonComposite.dds",	//Texture filename
			L"Content\\Textures\\MarsSpecularMap.png",	//Specular filename
			nullptr										//Parent
		};
	};
}
