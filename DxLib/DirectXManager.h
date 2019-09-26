#pragma once
#include "framework.h"
#include "GameObject.h"
#include <list>

namespace DxLib
{
	class DirectXManager
	{
	private:
		HWND hWnd;
		D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
		D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
		ID3D11Device* g_pd3dDevice = NULL;          // ���������� (��� �������� ��������)
		ID3D11DeviceContext* g_pImmediateContext = NULL;   // �������� ���������� (���������)
		IDXGISwapChain* g_pSwapChain = NULL;          // ���� ����� (������ � �������)
		ID3D11RenderTargetView* g_pRenderTargetView = NULL;   // ������ ������� ������

		ID3D11Texture2D* g_pDepthStencil = NULL;             // �������� ������ ������
		ID3D11DepthStencilView* g_pDepthStencilView = NULL;          // ������ ����, ����� ������
		
		ID3D11Buffer* g_pConstantBuffer = NULL;			// ����������� �����
		ConstantBuffer cb;

		int width;
		int height;

		DXGI_SWAP_CHAIN_DESC CreateFrontBuffer(HWND hWnd);
		HRESULT CreateRearBuffer();
		void SetupViewport();

		HRESULT InitMatrixes();
		void SetMatrixes(float fAngle);
		DirectX::XMMATRIX				g_World;                      // ������� ����
		DirectX::XMMATRIX				g_View;                       // ������� ����
		DirectX::XMMATRIX				g_Projection;                 // ������� ��������
		FLOAT					Xeye = 0.0f, Yeye = 1.0f, Zeye = -8.0f;
		FLOAT					Xat = 0.0f, Yat = 1.0f, Zat = 0.0f;
		FLOAT					Xup = 0.0f, Yup = 1.0f, Zup = 0.0f;

		DirectX::XMVECTOR				Eye = DirectX::XMVectorSet(Xeye, Yeye, Zeye, 0.0f);  // ������ �������
		DirectX::XMVECTOR				At = DirectX::XMVectorSet(Xat, Yat, Zat, 0.0f);    // ���� �������
		DirectX::XMVECTOR				Up = DirectX::XMVectorSet(Xup, Yup, Zup, 0.0f);    // ����������� �����
		GameObject* go = nullptr;
	public:
		DirectXManager(HWND& hWnd);
		DirectXManager(const DirectXManager&) = delete;
		DirectXManager& operator=(const DirectXManager&) = delete;
		~DirectXManager();

		ID3D11Device* getDevice() { return g_pd3dDevice; }
		D3D_DRIVER_TYPE getDriverType() { return g_driverType; }
		ID3D11DeviceContext* getIContext() { return g_pImmediateContext; }
		void CleanupDevice();
		void Render();
		void UpdateView();

		HRESULT CreateConstantBuffer()
		{
			HRESULT hr = S_OK;
			D3D11_BUFFER_DESC bd;  // ���������, ����������� ����������� �����
			ZeroMemory(&bd, sizeof(bd));
			// �������� ������������ ������
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(ConstantBuffer);            // ������ ������ = ������� ���������
			bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // ��� - ����������� �����
			bd.CPUAccessFlags = 0;
			hr = g_pd3dDevice->CreateBuffer(&bd, NULL, &g_pConstantBuffer);
			return hr;
		}
	};
}