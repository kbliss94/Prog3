#include "pch.h"

using namespace std;
using namespace Library;
using namespace DirectX;

namespace Rendering
{
	RTTI_DEFINITIONS(SolarSystem)

	const float SolarSystem::LightModulationRate = UCHAR_MAX;
	const float SolarSystem::LightMovementRate = 10.0f;
	const float SolarSystem::SunAmbientColor = 0.8f;
	const float SolarSystem::PlanetAmbientColor = 0.0f;
	const float SolarSystem::DistanceMultiplier = 50.0f;
	const float SolarSystem::SpeedFactor = .1f;

	SolarSystem::SolarSystem(Game & game, const shared_ptr<Camera>& camera, float orbitRadius, float scale, float orbPer, float rotPer, float axTilt, wstring texFilename, wstring specFilename) :
		DrawableGameComponent(game, camera), mWorldMatrix(MatrixHelper::Identity), mPointLight(game, XMFLOAT3(0.0f, 0.0f, 0.0f), 100000.0f), mRenderStateHelper(game), mIndexCount(0), 
		mTextPosition(0.0f, 40.0f), mAnimationEnabled(false), mOrbitalDistance(orbitRadius), mTextureFilename(texFilename), mSpecularFilename(specFilename), mScale(scale), 
		mOrbitalPeriod(orbPer), mRotationalPeriod(rotPer), mAxialAngle(0.0f), mOrbitalAngle(0.0f), mAxialTilt(axTilt)
	{

	}

	bool SolarSystem::AnimationEnabled() const
	{
		return mAnimationEnabled;
	}

	void SolarSystem::SetAnimationEnabled(bool enabled)
	{
		mAnimationEnabled = enabled;
	}

	void SolarSystem::Initialize()
	{
		// Load a compiled vertex shader
		vector<char> compiledVertexShader;
		Utility::LoadBinaryFile(L"Content\\Shaders\\PointLightDemoVS.cso", compiledVertexShader);
		ThrowIfFailed(mGame->Direct3DDevice()->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mVertexShader.ReleaseAndGetAddressOf()), "ID3D11Device::CreatedVertexShader() failed.");

		// Load a compiled pixel shader
		vector<char> compiledPixelShader;
		Utility::LoadBinaryFile(L"Content\\Shaders\\PointLightDemoPS.cso", compiledPixelShader);
		ThrowIfFailed(mGame->Direct3DDevice()->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, mPixelShader.ReleaseAndGetAddressOf()), "ID3D11Device::CreatedPixelShader() failed.");

		// Create an input layout
		D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		ThrowIfFailed(mGame->Direct3DDevice()->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), &compiledVertexShader[0], compiledVertexShader.size(), mInputLayout.ReleaseAndGetAddressOf()), "ID3D11Device::CreateInputLayout() failed.");

		// Load the model
		Library::Model model("Content\\Models\\Sphere.obj.bin");

		// Create vertex and index buffers for the model
		Library::Mesh* mesh = model.Meshes().at(0).get();
		CreateVertexBuffer(*mesh, mVertexBuffer.ReleaseAndGetAddressOf());
		mesh->CreateIndexBuffer(*mGame->Direct3DDevice(), mIndexBuffer.ReleaseAndGetAddressOf());
		mIndexCount = static_cast<uint32_t>(mesh->Indices().size());

		// Create constant buffers
		D3D11_BUFFER_DESC constantBufferDesc = { 0 };
		constantBufferDesc.ByteWidth = sizeof(VSCBufferPerFrame);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mVSCBufferPerFrame.ReleaseAndGetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		constantBufferDesc.ByteWidth = sizeof(VSCBufferPerObject);
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mVSCBufferPerObject.ReleaseAndGetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		constantBufferDesc.ByteWidth = sizeof(PSCBufferPerFrame);
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mPSCBufferPerFrame.ReleaseAndGetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		constantBufferDesc.ByteWidth = sizeof(PSCBufferPerObject);
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mPSCBufferPerObject.ReleaseAndGetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		// Load textures for the color and specular maps
		ThrowIfFailed(CreateDDSTextureFromFile(mGame->Direct3DDevice(), mTextureFilename.c_str(), nullptr, mColorTexture.ReleaseAndGetAddressOf()), "CreateDDSTextureFromFile() failed.");
		ThrowIfFailed(CreateWICTextureFromFile(mGame->Direct3DDevice(), mSpecularFilename.c_str(), nullptr, mSpecularMap.ReleaseAndGetAddressOf()), "CreateWICTextureFromFile() failed.");

		// Create text rendering helpers
		mSpriteBatch = make_unique<SpriteBatch>(mGame->Direct3DDeviceContext());
		mSpriteFont = make_unique<SpriteFont>(mGame->Direct3DDevice(), L"Content\\Fonts\\Arial_14_Regular.spritefont");

		// Retrieve the keyboard service
		mKeyboard = reinterpret_cast<KeyboardComponent*>(mGame->Services().GetService(KeyboardComponent::TypeIdClass()));
		
		// Setup the point light
		mVSCBufferPerFrameData.LightPosition = mPointLight.Position();
		mVSCBufferPerFrameData.LightRadius = mPointLight.Radius();
		mPSCBufferPerFrameData.LightPosition = mPointLight.Position();
		mPSCBufferPerFrameData.LightColor = ColorHelper::ToFloat3(mPointLight.Color(), true);

		// Update the vertex and pixel shader constant buffers
		mGame->Direct3DDeviceContext()->UpdateSubresource(mVSCBufferPerFrame.Get(), 0, nullptr, &mVSCBufferPerFrameData, 0, 0);
		mGame->Direct3DDeviceContext()->UpdateSubresource(mPSCBufferPerObject.Get(), 0, nullptr, &mPSCBufferPerObjectData, 0, 0);

		// Load a proxy model for the point light
		mProxyModel = make_unique<ProxyModel>(*mGame, mCamera, "Content\\Models\\Sphere.obj.bin", 1.0f);
		mProxyModel->Initialize();
		mProxyModel->SetPosition(mPointLight.Position());

		// Initializing celestial body data for each of the planets & Earth's moon
		mCelestialBodyDataList.push_back(make_shared<CelestialBodyData>(Mercury));
		mCelestialBodyDataList.push_back(make_shared<CelestialBodyData>(Venus));
		mCelestialBodyDataList.push_back(make_shared<CelestialBodyData>(Earth));
		mCelestialBodyDataList.push_back(make_shared<CelestialBodyData>(Mars));
		mCelestialBodyDataList.push_back(make_shared<CelestialBodyData>(Jupiter));
		mCelestialBodyDataList.push_back(make_shared<CelestialBodyData>(Saturn));
		mCelestialBodyDataList.push_back(make_shared<CelestialBodyData>(Uranus));
		mCelestialBodyDataList.push_back(make_shared<CelestialBodyData>(Neptune));
		mCelestialBodyDataList.push_back(make_shared<CelestialBodyData>(Pluto));
		mCelestialBodyDataList.push_back(make_shared<CelestialBodyData>(Moon));

		mCelestialBodies.resize(NumCelestialBodies);

		for (int i = 0; i < NumCelestialBodies; ++i)
		{
			if (i == NumCelestialBodies - 1)
			{
				mCelestialBodies[i] = make_shared<CelestialBodies>(*mGame, mCamera, mCelestialBodyDataList[i]->OrbitRadius * DistanceMultiplier, mCelestialBodyDataList[i]->Scale,
					mCelestialBodyDataList[i]->OrbitalPeriod, mCelestialBodyDataList[i]->RotationalPeriod, mCelestialBodyDataList[i]->AxialTilt,
					mCelestialBodyDataList[i]->TextureFilename, mCelestialBodyDataList[i]->SpecularFilename, mVSCBufferPerFrame, mVSCBufferPerObject, mCelestialBodies[EarthIndex]);
			}
			else
			{
				mCelestialBodies[i] = make_shared<CelestialBodies>(*mGame, mCamera, mCelestialBodyDataList[i]->OrbitRadius * DistanceMultiplier, mCelestialBodyDataList[i]->Scale,
					mCelestialBodyDataList[i]->OrbitalPeriod, mCelestialBodyDataList[i]->RotationalPeriod, mCelestialBodyDataList[i]->AxialTilt,
					mCelestialBodyDataList[i]->TextureFilename, mCelestialBodyDataList[i]->SpecularFilename, mVSCBufferPerFrame, mVSCBufferPerObject);
			}
		}

		for (int i = 0; i < NumCelestialBodies; ++i)
		{
			mCelestialBodies[i]->Initialize();
		}
	}

	void SolarSystem::Update(const GameTime& gameTime)
	{
		static float angle = 0.0f;

		XMMATRIX matTrans;
		XMMATRIX matScale;
		XMMATRIX matAxialRot;
		XMMATRIX matOrbitalRot;
		XMMATRIX matAxialTilt;

		if (mAnimationEnabled)
		{
			mAxialAngle += gameTime.ElapsedGameTimeSeconds().count() * mRotationalPeriod;
			mOrbitalAngle += gameTime.ElapsedGameTimeSeconds().count() * mOrbitalPeriod;

			matScale = XMMatrixScaling(mScale, mScale, mScale);
			matAxialRot = XMMatrixRotationY(mAxialAngle);
			matAxialTilt = XMMatrixRotationZ(mAxialTilt);
			matOrbitalRot = XMMatrixRotationY(mOrbitalAngle);
			matTrans = XMMatrixTranslation(angle, angle, mOrbitalDistance);
			XMStoreFloat4x4(&mWorldMatrix, (matScale * matAxialRot * matAxialTilt * matTrans * matOrbitalRot));
		}

		if (mKeyboard != nullptr)
		{
			if (mKeyboard->WasKeyPressedThisFrame(Keys::Space))
			{
				ToggleAnimation();
			}
		}

		mProxyModel->Update(gameTime);

		for (int i = 0; i < NumCelestialBodies; ++i)
		{
			mCelestialBodies[i]->Update(gameTime);
		}
	}

	void SolarSystem::Draw(const GameTime& gameTime)
	{
		mPSCBufferPerFrameData.AmbientColor = XMFLOAT3(SunAmbientColor, SunAmbientColor, SunAmbientColor);
		mGame->Direct3DDeviceContext()->UpdateSubresource(mPSCBufferPerFrame.Get(), 0, nullptr, &mPSCBufferPerFrameData, 0, 0);

		UNREFERENCED_PARAMETER(gameTime);
		assert(mCamera != nullptr);

		ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		direct3DDeviceContext->IASetInputLayout(mInputLayout.Get());

		UINT stride = sizeof(VertexPositionTextureNormal);
		UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		direct3DDeviceContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
		direct3DDeviceContext->PSSetShader(mPixelShader.Get(), nullptr, 0);

		XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
		XMMATRIX wvp = worldMatrix * mCamera->ViewProjectionMatrix();

		wvp = XMMatrixTranspose(wvp);

		XMStoreFloat4x4(&mVSCBufferPerObjectData.WorldViewProjection, wvp);
		XMStoreFloat4x4(&mVSCBufferPerObjectData.World, XMMatrixTranspose(worldMatrix));

		direct3DDeviceContext->UpdateSubresource(mVSCBufferPerObject.Get(), 0, nullptr, &mVSCBufferPerObjectData, 0, 0);

		ID3D11Buffer* VSConstantBuffers[] = { mVSCBufferPerFrame.Get(), mVSCBufferPerObject.Get() };
		direct3DDeviceContext->VSSetConstantBuffers(0, ARRAYSIZE(VSConstantBuffers), VSConstantBuffers);

		mPSCBufferPerFrameData.CameraPosition = mCamera->Position();
		direct3DDeviceContext->UpdateSubresource(mPSCBufferPerFrame.Get(), 0, nullptr, &mPSCBufferPerFrameData, 0, 0);

		ID3D11Buffer* PSConstantBuffers[] = { mPSCBufferPerFrame.Get(), mPSCBufferPerObject.Get() };
		direct3DDeviceContext->PSSetConstantBuffers(0, ARRAYSIZE(PSConstantBuffers), PSConstantBuffers);

		ID3D11ShaderResourceView* PSShaderResources[] = { mColorTexture.Get(), mSpecularMap.Get() };
		direct3DDeviceContext->PSSetShaderResources(0, ARRAYSIZE(PSShaderResources), PSShaderResources);
		direct3DDeviceContext->PSSetSamplers(0, 1, SamplerStates::TrilinearWrap.GetAddressOf());

		direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);

		mProxyModel->Draw(gameTime);

		// Draw help text
		mRenderStateHelper.SaveAll();
		mSpriteBatch->Begin();

		wostringstream helpLabel;
		helpLabel << L"Decrease/Increase Rotational & Orbital Velocities (E/R)" << "\n";
		helpLabel << L"Reset Camera to Center of Solar System (Q)" << "\n";
		helpLabel << L"Camera Controls (WASD + Left Mouse)" << "\n";
		helpLabel << L"Toggle Animation (Space)" << "\n";
		helpLabel << L"Exit (Esc)" << "\n";
	
		mSpriteFont->DrawString(mSpriteBatch.get(), helpLabel.str().c_str(), mTextPosition);
		mSpriteBatch->End();
		mRenderStateHelper.RestoreAll();

		mPSCBufferPerFrameData.AmbientColor = XMFLOAT3(PlanetAmbientColor, PlanetAmbientColor, PlanetAmbientColor);
		mGame->Direct3DDeviceContext()->UpdateSubresource(mPSCBufferPerFrame.Get(), 0, nullptr, &mPSCBufferPerFrameData, 0, 0);

		for (int i = 0; i < NumCelestialBodies; ++i)
		{
			mCelestialBodies[i]->Draw(gameTime);
		}
	}

	void SolarSystem::CreateVertexBuffer(const Mesh& mesh, ID3D11Buffer** vertexBuffer) const
	{
		const vector<XMFLOAT3>& sourceVertices = mesh.Vertices();
		const vector<XMFLOAT3>& sourceNormals = mesh.Normals();
		const auto& sourceUVs = mesh.TextureCoordinates().at(0);

		vector<VertexPositionTextureNormal> vertices;
		vertices.reserve(sourceVertices.size());
		for (UINT i = 0; i < sourceVertices.size(); i++)
		{
			const XMFLOAT3& position = sourceVertices.at(i);
			const XMFLOAT3& uv = sourceUVs->at(i);
			const XMFLOAT3& normal = sourceNormals.at(i);

			vertices.push_back(VertexPositionTextureNormal(XMFLOAT4(position.x, position.y, position.z, 1.0f), XMFLOAT2(uv.x, uv.y), normal));
		}
		D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
		vertexBufferDesc.ByteWidth = sizeof(VertexPositionTextureNormal) * static_cast<UINT>(vertices.size());
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData = { 0 };
		vertexSubResourceData.pSysMem = &vertices[0];
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, vertexBuffer), "ID3D11Device::CreateBuffer() failed.");
	}

	void SolarSystem::ToggleAnimation()
	{
		mAnimationEnabled = !mAnimationEnabled;
	}
}