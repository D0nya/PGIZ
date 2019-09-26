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
		ID3D11Device* g_pd3dDevice = NULL;          // Устройство (для создания объектов)
		ID3D11DeviceContext* g_pImmediateContext = NULL;   // Контекст устройства (рисование)
		IDXGISwapChain* g_pSwapChain = NULL;          // Цепь связи (буфера с экраном)
		ID3D11RenderTargetView* g_pRenderTargetView = NULL;   // Объект заднего буфера

		ID3D11Texture2D* g_pDepthStencil = NULL;             // Текстура буфера глубин
		ID3D11DepthStencilView* g_pDepthStencilView = NULL;          // Объект вида, буфер глубин
		
		ID3D11Buffer* g_pConstantBuffer = NULL;			// Константный буфер
		ConstantBuffer cb;

		int width;
		int height;

		DXGI_SWAP_CHAIN_DESC CreateFrontBuffer(HWND hWnd);
		HRESULT CreateRearBuffer();
		void SetupViewport();

		HRESULT InitMatrixes();
		void SetMatrixes(float fAngle);
		DirectX::XMMATRIX				g_World;                      // Матрица мира
		DirectX::XMMATRIX				g_View;                       // Матрица вида
		DirectX::XMMATRIX				g_Projection;                 // Матрица проекции
		FLOAT					Xeye = 0.0f, Yeye = 1.0f, Zeye = -8.0f;
		FLOAT					Xat = 0.0f, Yat = 1.0f, Zat = 0.0f;
		FLOAT					Xup = 0.0f, Yup = 1.0f, Zup = 0.0f;

		DirectX::XMVECTOR				Eye = DirectX::XMVectorSet(Xeye, Yeye, Zeye, 0.0f);  // Откуда смотрим
		DirectX::XMVECTOR				At = DirectX::XMVectorSet(Xat, Yat, Zat, 0.0f);    // Куда смотрим
		DirectX::XMVECTOR				Up = DirectX::XMVectorSet(Xup, Yup, Zup, 0.0f);    // Направление верха
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
			D3D11_BUFFER_DESC bd;  // Структура, описывающая создаваемый буфер
			ZeroMemory(&bd, sizeof(bd));
			// Создание константного буфера
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(ConstantBuffer);            // размер буфера = размеру структуры
			bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // тип - константный буфер
			bd.CPUAccessFlags = 0;
			hr = g_pd3dDevice->CreateBuffer(&bd, NULL, &g_pConstantBuffer);
			return hr;
		}
	};
}