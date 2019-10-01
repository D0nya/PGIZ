#include "pch.h"
#include "framework.h"
#include "DirectXManager.h"

namespace DxLib
{
	DirectXManager::DirectXManager(HWND& _hWnd) : hWnd(_hWnd)
	{
		HRESULT hr = S_OK;
		RECT rc;
		GetClientRect(hWnd, &rc);
		width = rc.right - rc.left;			// получаем ширину
		height = rc.bottom - rc.top;		// и высоту окна

		UINT createDeviceFlags = 0;
		D3D_DRIVER_TYPE driverTypes[] =
		{
				D3D_DRIVER_TYPE_HARDWARE,
				D3D_DRIVER_TYPE_WARP,
				D3D_DRIVER_TYPE_REFERENCE,
		};
		UINT numDriverTypes = ARRAYSIZE(driverTypes);

		// Тут мы создаем список поддерживаемых версий DirectX
		D3D_FEATURE_LEVEL featureLevels[] =
		{
				D3D_FEATURE_LEVEL_11_0,
				D3D_FEATURE_LEVEL_10_1,
				D3D_FEATURE_LEVEL_10_0,
		};
		UINT numFeatureLevels = ARRAYSIZE(featureLevels);

		auto sd = CreateFrontBuffer(hWnd);

		// проверка совместимости типов драйверов и создание устройства
		for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
		{
			g_driverType = driverTypes[driverTypeIndex];
			hr = D3D11CreateDeviceAndSwapChain
			(
				NULL,
				g_driverType,
				NULL,
				createDeviceFlags,
				featureLevels,
				numFeatureLevels,
				D3D11_SDK_VERSION,
				&sd, &g_pSwapChain,
				&g_pd3dDevice,
				&g_featureLevel,
				&g_pImmediateContext
			);
			if (SUCCEEDED(hr)) // Если устройства созданы успешно, то выходим из цикла
				break;
		}
		if (FAILED(hr))
		{
			CleanupDevice();
			return;
		}
		hr = CreateConstantBuffer();
		if (FAILED(hr))
		{
			CleanupDevice();
			return;
		}
		g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		gameObjects.push_back(new GameObject(g_pd3dDevice, hWnd, nullptr));

		SimpleVertex verticesMorkva[] =
		{  /* координаты X, Y, Z                          цвет R, G, B, A     */
			{ DirectX::XMFLOAT3(0.0f,  3.f,  0.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f) },
			{ DirectX::XMFLOAT3(-0.5f,  0.5f, -0.5f), DirectX::XMFLOAT4(1.0f, 0.5f, 0.0f, 0.0f) },
			{ DirectX::XMFLOAT3(0.5f,  0.5f, -0.5f), DirectX::XMFLOAT4(1.0f, 0.5f, 0.0f, 0.0f) },
			{ DirectX::XMFLOAT3(-0.5f,  0.5f,  0.5f), DirectX::XMFLOAT4(1.0f, 0.5f, 0.0f, 0.0f) },
			{ DirectX::XMFLOAT3(0.5f,  0.5f,  0.5f), DirectX::XMFLOAT4(1.0f, 0.5f, 0.0f, 0.0f) },
			{ DirectX::XMFLOAT3(-0.25f,  0.f,  -0.25f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f) },
			{ DirectX::XMFLOAT3(0.25f,  0.f,  -0.25f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f) },
			{ DirectX::XMFLOAT3(-0.25f,  0.f,  0.25f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f) },
			{ DirectX::XMFLOAT3(0.25f,  0.f,  0.25f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f) },
		};
		WORD indicesMorkva[] =
		{  // индексы массива vertices[], по которым строятся треугольники
			0,2,1,      /* Треугольник 1 = vertices[0], vertices[2], vertices[1] */
			0,3,4,      /* Треугольник 2 = vertices[0], vertices[3], vertices[4] */
			0,1,3,      /* и т. д. */
			0,4,2,

			1,5,7,
			1,6,5,
			1,7,3,
			1,2,6,

			4,3,7,
			8,4,7,
			8,6,2,
			8,2,4,

			5,6,8,
			5,8,7

		};
		ShapeTemplate templ;
		templ.vertices = verticesMorkva;
		templ.indices = indicesMorkva;
		templ.iSize = 42;
		templ.vSize = 9;

		gameObjects.push_back(new GameObject(g_pd3dDevice, hWnd, &templ));

		hr = InitMatrixes();
		if (FAILED(hr))
		{
			CleanupDevice();
			return;
		}
		hr = CreateRearBuffer();
		if (FAILED(hr))
		{
			CleanupDevice();
			return;
		}
		SetupViewport();
	}

	DXGI_SWAP_CHAIN_DESC DirectXManager::CreateFrontBuffer(HWND hWnd)
	{
		// Сейчас мы создадим устройства DirectX. Для начала заполним структуру,
		// которая описывает свойства переднего буфера и привязывает его к нашему окну.
		DXGI_SWAP_CHAIN_DESC sd;									// Структура, описывающая цепь связи (Swap Chain)
		ZeroMemory(&sd, sizeof(sd));							// очищаем ее
		sd.BufferCount = 1;																	// у нас один задний буфер
		sd.BufferDesc.Width = width;												// ширина буфера
		sd.BufferDesc.Height = height;											// высота буфера
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// формат пикселя в буфере
		sd.BufferDesc.RefreshRate.Numerator = 75;						// частота обновления экрана
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;		// назначение буфера - задний буфер
		sd.OutputWindow = hWnd;		// привязываем к нашему окну
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;	// не полноэкранный режим

		return sd;
	}
	HRESULT DirectXManager::CreateRearBuffer()
	{
		HRESULT hr = S_OK;
		// Теперь создаем задний буфер. Обратите внимание, в SDK
		// RenderTargetOutput - это передний буфер, а RenderTargetView - задний.
		ID3D11Texture2D* pBackBuffer = NULL;
		hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)& pBackBuffer);
		if (FAILED(hr)) return hr;

		hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_pRenderTargetView);
		pBackBuffer->Release();
		if (FAILED(hr)) return hr;

		// Переходим к созданию буфера глубин
		// Создаем текстуру-описание буфера глубин
		D3D11_TEXTURE2D_DESC descDepth;     // Структура с параметрами
		ZeroMemory(&descDepth, sizeof(descDepth));
		descDepth.Width = width;            // ширина и
		descDepth.Height = height;    // высота текстуры
		descDepth.MipLevels = 1;            // уровень интерполяции
		descDepth.ArraySize = 1;
		descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // формат (размер пикселя)
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;         // вид - буфер глубин
		descDepth.CPUAccessFlags = 0;
		descDepth.MiscFlags = 0;
		// При помощи заполненной структуры-описания создаем объект текстуры
		hr = g_pd3dDevice->CreateTexture2D(&descDepth, NULL, &g_pDepthStencil);
		if (FAILED(hr)) return hr;

		// Теперь надо создать сам объект буфера глубин
		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;            // Структура с параметрами
		ZeroMemory(&descDSV, sizeof(descDSV));
		descDSV.Format = descDepth.Format;         // формат как в текстуре
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;
		// При помощи заполненной структуры-описания и текстуры создаем объект буфера глубин
		hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV, &g_pDepthStencilView);
		if (FAILED(hr)) return hr;

		// Подключаем объект заднего буфера и объект буфера глубин к контексту устройства

		g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);

		return hr;
	}
	void DirectXManager::SetupViewport()
	{
		// Настройка вьюпорта
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)width;
		vp.Height = (FLOAT)height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		// Подключаем вьюпорт к контексту устройства
		g_pImmediateContext->RSSetViewports(1, &vp);
	}

	DirectXManager::~DirectXManager()
	{
		CleanupDevice();
	}
	void DirectXManager::CleanupDevice()
	{
		// Сначала отключим контекст устройства
		if (g_pImmediateContext) g_pImmediateContext->ClearState();
		if (g_pDepthStencil) g_pDepthStencil->Release();
		if (g_pDepthStencilView) g_pDepthStencilView->Release();
		if (g_pRenderTargetView) g_pRenderTargetView->Release();
		if (g_pSwapChain) g_pSwapChain->Release();
		if (g_pImmediateContext) g_pImmediateContext->Release();
		if (g_pd3dDevice) g_pd3dDevice->Release();
	}

	HRESULT DirectXManager::InitMatrixes()
	{
		RECT rc;
		GetClientRect(hWnd, &rc);
		UINT width = rc.right - rc.left;           // получаем ширину
		UINT height = rc.bottom - rc.top;   // и высоту окна

		// Инициализация матрицы мира
		g_World = DirectX::XMMatrixIdentity();

		g_View = DirectX::XMMatrixLookAtLH(Eye, At, Up);

		// Инициализация матрицы проекции
		g_Projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, width / (FLOAT)height, 0.01f, 100.0f);

		return S_OK;
	}
	void DirectXManager::SetMatrixes(float fAngle)
	{
		// Обновление переменной-времени
		static float t = 0.0f;
		if (g_driverType == D3D_DRIVER_TYPE_REFERENCE)
		{
			t += (float)DirectX::XM_PI * 0.0125f;
		}
		else
		{
			static ULONGLONG dwTimeStart = 0;
			ULONGLONG dwTimeCur = GetTickCount64();
			if (dwTimeStart == 0)
				dwTimeStart = dwTimeCur;
			t = (dwTimeCur - dwTimeStart) / 1000.0f;
		}
		// Матрица-орбита: позиция объекта
		DirectX::XMMATRIX mOrbit = DirectX::XMMatrixRotationY(-t + fAngle);
		// Матрица-спин: вращение объекта вокруг своей оси
		DirectX::XMMATRIX mSpin = DirectX::XMMatrixRotationY(t * 2);
		// Матрица-позиция: перемещение на три единицы влево от начала координат
		DirectX::XMMATRIX mTranslate = DirectX::XMMatrixTranslation(-3.0f, 0.0f, 0.0f);
		// Матрица-масштаб: сжатие объекта в 2 раза
		DirectX::XMMATRIX mScale = DirectX::XMMatrixScaling(0.5f, 0.5f, 0.5f);
		// Результирующая матрица
		//  --Сначала мы в центре, в масштабе 1:1:1, повернуты по всем осям на 0.0f.
		//  --Сжимаем -> поворачиваем вокруг Y (пока мы еще в центре) -> переносим влево ->
		//  --снова поворачиваем вокруг Y.
		g_World = mScale * mSpin * mTranslate * mOrbit;
		//Собственно, читаем комментарии, которые я сделал макси

		// Обновить константный буфер
		// создаем временную структуру и загружаем в нее матрицы
		cb.mWorld = XMMatrixTranspose(g_World);
		cb.mView = XMMatrixTranspose(g_View);
		cb.mProjection = XMMatrixTranspose(g_Projection);
		g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cb, 0, 0);
	}
	void DirectXManager::SetMatrixes()
	{
		// Обновление переменной-времени
		static float t = 0.0f;
		if (g_driverType == D3D_DRIVER_TYPE_REFERENCE)
		{
			t += (float)DirectX::XM_PI * 0.0125f;
		}
		else
		{
			static ULONGLONG dwTimeStart = 0;
			ULONGLONG dwTimeCur = GetTickCount64();
			if (dwTimeStart == 0)
				dwTimeStart = dwTimeCur;
			t = (dwTimeCur - dwTimeStart) / 1000.0f;
		}
		// Вращать мир по оси Y на угол t (в радианах)
		g_World = DirectX::XMMatrixRotationY(t);
		// Обновить константный буфер
		// создаем временную структуру и загружаем в нее матрицы
		ConstantBuffer cb;
		cb.mWorld = XMMatrixTranspose(g_World);
		cb.mView = XMMatrixTranspose(g_View);
		cb.mProjection = XMMatrixTranspose(g_Projection);
		// загружаем временную структуру в константный буфер g_pConstantBuffer
		g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cb, 0, 0);
	}
	void DirectXManager::UpdateView()
	{

		if (GetKeyState('A') & 0x8000)
		{
			Xeye -= 0.001F;
		}
		if (GetKeyState('S') & 0x8000)
		{
			Zeye -= 0.001F;
		}
		if (GetKeyState('D') & 0x8000)
		{
			Xeye += 0.001F;
		}
		if (GetKeyState('W') & 0x8000)
		{
			Zeye += 0.001F;
		}
		if (GetKeyState(VK_SPACE) & 0x8000)
		{
			Yeye += 0.001F;
		}
		if (GetKeyState(VK_CONTROL) & 0x8000)
		{
			Yeye -= 0.001F;
		}

		if (GetKeyState(VK_LEFT) & 0x8000)
		{
			Xat -= 0.001F;
		}
		if (GetKeyState(VK_RIGHT) & 0x8000)
		{
			Xat += 0.001F;
		}
		if (GetKeyState(VK_UP) & 0x8000)
		{
			Yat += 0.001F;
		}
		if (GetKeyState(VK_DOWN) & 0x8000)
		{
			Yat -= 0.001F;
		}
		Eye = DirectX::XMVectorSet(Xeye, Yeye, Zeye, 0.0f);  // Откуда смотрим
		At = DirectX::XMVectorSet(Xat, Yat, Zat, 0.0f);    // Куда смотрим
		g_View = DirectX::XMMatrixLookAtLH(Eye, At, Up);
	}
	void DirectXManager::Render()
	{
		UINT stride = sizeof(SimpleVertex);
		UINT offset = 0;

		// загружаем временную структуру в константный буфер g_pConstantBuffer
		g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cb, 0, 0);
		// Очистить задний буфер
		float ClearColor[4] = { 0.0f, 1.0f, 1.0f, 1.0f }; // красный, зеленый, синий, альфа-канал
		g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);
		// Очистить буфер глубин до 1.0 (максимальное значение)
		g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
		int i = 0;
		for (auto go : gameObjects)
		{
			g_pImmediateContext->IASetInputLayout(go->mesh->shader->g_pVertexLayout);
			g_pImmediateContext->IASetVertexBuffers(0, 1, &go->mesh->g_pVertexBuffer, &stride, &offset);
			// Установка буфера индексов
			g_pImmediateContext->IASetIndexBuffer(go->mesh->g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
			if (i == 0)
			{
				for (int i = 0; i < 6; i++)
				{
					// Устанавливаем матрицу, параметр - положение относительно оси Y в радианах
					SetMatrixes(i * (DirectX::XM_PI * 2) / 6);
					// Рисуем i-тую пирамидку
					g_pImmediateContext->VSSetShader(go->mesh->shader->g_pVertexShader, NULL, 0);
					g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
					g_pImmediateContext->PSSetShader(go->mesh->shader->g_pPixelShader, NULL, 0);
					g_pImmediateContext->DrawIndexed(go->iSize, 0, 0);
				}
				i++;
				continue;
			}
			SetMatrixes();
			g_pImmediateContext->VSSetShader(go->mesh->shader->g_pVertexShader, NULL, 0);
			g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
			g_pImmediateContext->PSSetShader(go->mesh->shader->g_pPixelShader, NULL, 0);
			g_pImmediateContext->DrawIndexed(go->iSize, 0, 0);
		}
		g_pSwapChain->Present(0, 0);
	}
}