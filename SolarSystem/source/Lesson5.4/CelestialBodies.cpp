#include "pch.h"
#include "CelestialBodies.h"

using namespace std;
using namespace Library;
using namespace DirectX;

namespace Rendering
{
	RTTI_DEFINITIONS(CelestialBodies)

	CelestialBodies::CelestialBodies(Game & game, const shared_ptr<Camera>& camera, float orbitRadius, float scale, float orbPer, float rotPer, float axTilt, 
		wstring texFilename, wstring specFilename, Microsoft::WRL::ComPtr<ID3D11Buffer> frameBuffer, Microsoft::WRL::ComPtr<ID3D11Buffer> objectBuffer,
		std::shared_ptr<CelestialBodies> parent) :
		DrawableGameComponent(game, camera), mWorldMatrix(MatrixHelper::Identity), mRenderStateHelper(game), mIndexCount(0),
		mAnimationEnabled(false), mOrbitalDistance(orbitRadius), mTextureFilename(texFilename), mSpecularFilename(specFilename), mScale(scale), 
		mOrbitalPeriod(orbPer), mRotationalPeriod(rotPer), mAxialAngle(0.0f), mOrbitalAngle(0.0f), mAxialTilt(axTilt), 
		mVSCBufferPerFrame(frameBuffer), mVSCBufferPerObject(objectBuffer), mParent(parent)
	{
	}

	bool CelestialBodies::AnimationEnabled() const
	{
		return mAnimationEnabled;
	}

	void CelestialBodies::SetAnimationEnabled(bool enabled)
	{
		mAnimationEnabled = enabled;
	}

	void CelestialBodies::Initialize()
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

		// Initialize shaders
		mGame->Direct3DDeviceContext()->UpdateSubresource(mVSCBufferPerFrame.Get(), 0, nullptr, &mVSCBufferPerFrameData, 0, 0);
		mGame->Direct3DDeviceContext()->UpdateSubresource(mVSCBufferPerObject.Get(), 0, nullptr, &mVSCBufferPerObjectData, 0, 0);

		// Load textures for the color and specular maps
		ThrowIfFailed(CreateDDSTextureFromFile(mGame->Direct3DDevice(), mTextureFilename.c_str(), nullptr, mColorTexture.ReleaseAndGetAddressOf()), "CreateDDSTextureFromFile() failed.");
		ThrowIfFailed(CreateWICTextureFromFile(mGame->Direct3DDevice(), mSpecularFilename.c_str(), nullptr, mSpecularMap.ReleaseAndGetAddressOf()), "CreateWICTextureFromFile() failed.");

		// Retrieve the keyboard service
		mKeyboard = reinterpret_cast<KeyboardComponent*>(mGame->Services().GetService(KeyboardComponent::TypeIdClass()));
	}

	void CelestialBodies::Update(const GameTime& gameTime)
	{
		static float angle = 0.0f;

		XMMATRIX matTrans;
		XMMATRIX matScale;
		XMMATRIX matAxialRot;
		XMMATRIX matOrbitalRot;
		XMMATRIX matAxialTilt;

		if (mAnimationEnabled)
		{
			if (mParent == nullptr)
			{
				mAxialAngle += gameTime.ElapsedGameTimeSeconds().count() * (1 / mRotationalPeriod) * RotationalSpeedFactor;
				mOrbitalAngle += gameTime.ElapsedGameTimeSeconds().count() * (1 / mOrbitalPeriod) * OrbitalSpeedFactor;

				matScale = XMMatrixScaling(mScale, mScale, mScale);
				matAxialRot = XMMatrixRotationY(mAxialAngle);
				matAxialTilt = XMMatrixRotationZ(mAxialTilt);
				matOrbitalRot = XMMatrixRotationY(mOrbitalAngle);
				matTrans = XMMatrixTranslation(angle, angle, mOrbitalDistance);
				XMStoreFloat4x4(&mWorldMatrix, (matScale * matAxialRot * matAxialTilt * matTrans * matOrbitalRot));
			}
			else
			{
				mAxialAngle += gameTime.ElapsedGameTimeSeconds().count() * (1 / mRotationalPeriod) * RotationalSpeedFactor;
				mOrbitalAngle += gameTime.ElapsedGameTimeSeconds().count() * (1 / mOrbitalPeriod) * OrbitalSpeedFactor;

				matScale = XMMatrixScaling(mScale, mScale, mScale);
				matAxialRot = XMMatrixRotationY(mAxialAngle);
				matAxialTilt = XMMatrixRotationZ(mAxialTilt);
				matOrbitalRot = XMMatrixRotationY(mOrbitalAngle);
				matTrans = XMMatrixTranslation(angle, angle, mOrbitalDistance);

				XMMATRIX parentMatrix = XMLoadFloat4x4(&mParent->mWorldMatrix);

				XMStoreFloat4x4(&mWorldMatrix, (matScale * matAxialRot * matAxialTilt * matTrans * matOrbitalRot * parentMatrix));
			}
		}

		if (mKeyboard != nullptr)
		{
			if (mKeyboard->WasKeyPressedThisFrame(Keys::Space))
			{
				ToggleAnimation();
			}
		}
	}

	void CelestialBodies::Draw(const GameTime& gameTime)
	{
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

		ID3D11ShaderResourceView* PSShaderResources[] = { mColorTexture.Get(), mSpecularMap.Get() };
		direct3DDeviceContext->PSSetShaderResources(0, ARRAYSIZE(PSShaderResources), PSShaderResources);
		direct3DDeviceContext->PSSetSamplers(0, 1, SamplerStates::TrilinearWrap.GetAddressOf());

		direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);
	}

	void CelestialBodies::CreateVertexBuffer(const Mesh& mesh, ID3D11Buffer** vertexBuffer) const
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

	void CelestialBodies::ToggleAnimation()
	{
		mAnimationEnabled = !mAnimationEnabled;
	}
}