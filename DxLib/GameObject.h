#pragma once
#include "framework.h"


enum ShaderType
{
	VERTEX_SHADER,
	PIXEL_SHADER
};
struct ConstantBuffer
{
	DirectX::XMMATRIX mWorld;       // ������� ����
	DirectX::XMMATRIX mView;        // ������� ����
	DirectX::XMMATRIX mProjection;  // ������� ��������
};
struct SimpleVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT4 Color;
};	
struct ShapeTemplate
{
	WORD* indices;
	int iSize;
	SimpleVertex* vertices;
	int vSize;
};

class Shader
{
public:

	ID3D11VertexShader* g_pVertexShader = NULL;       // ��������� ������
	ID3D11PixelShader* g_pPixelShader = NULL;     // ���������� ������
	ID3D11InputLayout* g_pVertexLayout = NULL;    // �������� ������� ������

	HRESULT CreateShader(LPCSTR shaderPath, ShaderType shaderType, ID3D11Device* device)
	{
		HRESULT hr = S_OK;
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				/* ������������� ���, ������������� ������, ������, �������� ���� (0-15), ����� ������ ������ � ������ ������, ����� ��������� ����� (�� �����), InstanceDataStepRate (�� �����) */
		};
		UINT numElements = ARRAYSIZE(layout);

		// ���������� ���������� ������� �� �����
		ID3DBlob* pBlob = NULL; // ��������������� ������ - ������ ����� � ����������� ������
		switch (shaderType)
		{
		case VERTEX_SHADER:
			hr = CompileShaderFromFile(shaderPath, "VS", "vs_4_0", &pBlob);
			if (FAILED(hr))
			{
				MessageBox(NULL, "���������� �������������� ���� FX. ����������, ��������� ������ ��������� �� �����, ���������� ���� FX.", "������", MB_OK);
				return hr;
			}
			hr = device->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &g_pVertexShader);

			// �������� ������� ������
			hr = device->CreateInputLayout(layout, numElements, pBlob->GetBufferPointer(),
				pBlob->GetBufferSize(), &g_pVertexLayout);

			break;
		case PIXEL_SHADER:
			hr = CompileShaderFromFile(shaderPath, "PS", "ps_4_0", &pBlob);
			if (FAILED(hr))
			{
				MessageBox(NULL, "���������� �������������� ���� FX. ����������, ��������� ������ ��������� �� �����, ���������� ���� FX.", "������", MB_OK);
				return hr;
			}
			hr = device->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &g_pPixelShader);
			break;
		}
		pBlob->Release();
			return hr;
	}
	HRESULT CompileShaderFromFile(LPCSTR szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
	{
		HRESULT hr = S_OK;
		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
		ID3DBlob* pErrorBlob;
		hr = D3DX11CompileFromFile(szFileName, NULL, NULL, szEntryPoint, szShaderModel,
			dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);
		if (FAILED(hr))
		{
			if (pErrorBlob != NULL)
				OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			if (pErrorBlob) pErrorBlob->Release();
			return hr;
		}
		if (pErrorBlob)
			pErrorBlob->Release();

		return S_OK;
	}
};

class Mesh
{
public:
	Shader* shader;
	ID3D11Buffer* g_pIndexBuffer = NULL;        // ����� �������� ������
	ID3D11Buffer* g_pVertexBuffer = NULL;         // ����� ������
	HWND hWnd;

	Mesh(ID3D11Device* device, HWND hWnd, const ShapeTemplate* templ, LPCSTR shaderPath = "..\\DxLib\\DefaultShader.fx") : hWnd(hWnd)
	{
		HRESULT hr = S_OK;
		shader = new Shader();
		hr = shader->CreateShader(shaderPath, VERTEX_SHADER, device);
		if (FAILED(hr))
			return;
		hr = shader->CreateShader(shaderPath, PIXEL_SHADER, device);
		if (FAILED(hr))
			return;

		if (templ != nullptr)
		{
			hr = CreateVertexBuffer(device, templ->vSize, (*templ).vertices);
			if (FAILED(hr))
				return;
			hr = CreateIndexBuffer(device, templ->iSize, (*templ).indices);
			if (FAILED(hr))
				return;
		}
		else
		{
			hr = CreateVertexBuffer(device);
			if (FAILED(hr))
				return;
			hr = CreateIndexBuffer(device);
			if (FAILED(hr))
				return;
		}
	}

	HRESULT CreateIndexBuffer(ID3D11Device* device)
	{
		HRESULT hr = S_OK;
		D3D11_BUFFER_DESC bd;  // ���������, ����������� ����������� �����
		ZeroMemory(&bd, sizeof(bd));
		D3D11_SUBRESOURCE_DATA InitData; // ���������, ���������� ������ ������
		ZeroMemory(&InitData, sizeof(InitData)); // ������� 

		WORD indices[] =
		{
			0, 2, 1,  //2,1,0    /* ����������� 1 = vertices[0], vertices[2], vertices[1] */
			0, 3, 4,      /* ����������� 2 = vertices[0], vertices[3], vertices[4] */
			0, 1, 3,      /* � �. �. */
			0, 4, 2,

			5, 1, 2,
			5, 4, 3,
			5, 3, 1,
			5, 2, 4
		};

		bd.Usage = D3D11_USAGE_DEFAULT;            // ���������, ����������� ����������� �����
		bd.ByteWidth = 24 * sizeof(WORD); // ��� 6 ������������� ���������� 18 ������
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER; // ��� - ����� ��������
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = indices;         // ��������� �� ��� ������ ��������

		// ����� ������ g_pd3dDevice ������� ������ ������ ��������
		hr = device->CreateBuffer(&bd, &InitData, &g_pIndexBuffer);
		return hr;
	}
	HRESULT CreateVertexBuffer(ID3D11Device* device)
	{
		HRESULT hr = S_OK;
		D3D11_BUFFER_DESC bd;  // ���������, ����������� ����������� �����
		ZeroMemory(&bd, sizeof(bd));                    // ������� ��

		SimpleVertex vertices[] =
		{  /* ���������� X, Y, Z                          ���� R, G, B, A     */
				{ DirectX::XMFLOAT3(0.0f,  3.0f,  0.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
				{ DirectX::XMFLOAT3(-1.0f,  0.0f, -1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
				{ DirectX::XMFLOAT3(1.0f,  0.0f, -1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
				{ DirectX::XMFLOAT3(-1.0f,  0.0f,  1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
				{ DirectX::XMFLOAT3(1.0f,  0.0f,  1.0f), DirectX::XMFLOAT4(.0f, 1.0f, 0.0f, 1.0f) },
				{ DirectX::XMFLOAT3(0.0f,  -3.0f,  0.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		};

		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = 24 * sizeof(SimpleVertex); // ������ ������ = ������ ����� ������� * 3
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;          // ��� ������ - ����� 
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData; // ���������, ���������� ������ ������
		ZeroMemory(&InitData, sizeof(InitData)); // ������� 
		InitData.pSysMem = vertices;		// ��������� �� �������

		// ����� ������ g_pd3dDevice ������� ������ ������ ������ ID3D11Buffer
		hr = device->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
		return hr;
	}

	HRESULT CreateIndexBuffer(ID3D11Device* device, const int iSize, const WORD* inds)
	{
		HRESULT hr = S_OK;
		D3D11_BUFFER_DESC bd;  // ���������, ����������� ����������� �����
		ZeroMemory(&bd, sizeof(bd));
		D3D11_SUBRESOURCE_DATA InitData; // ���������, ���������� ������ ������
		ZeroMemory(&InitData, sizeof(InitData)); // ������� 

		WORD* indices = new WORD[iSize];
		memcpy(indices, inds, iSize * sizeof(WORD));

		bd.Usage = D3D11_USAGE_DEFAULT;            // ���������, ����������� ����������� �����
		bd.ByteWidth = iSize * sizeof(WORD); // ��� 6 ������������� ���������� 18 ������
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER; // ��� - ����� ��������
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = indices;         // ��������� �� ��� ������ ��������

		// ����� ������ g_pd3dDevice ������� ������ ������ ��������
		hr = device->CreateBuffer(&bd, &InitData, &g_pIndexBuffer);
		return hr;
	}
	HRESULT CreateVertexBuffer(ID3D11Device* device, const int vSize, const SimpleVertex* verts)
	{
		HRESULT hr = S_OK;
		D3D11_BUFFER_DESC bd;  // ���������, ����������� ����������� �����
		ZeroMemory(&bd, sizeof(bd));                    // ������� ��

		SimpleVertex* vertices = new SimpleVertex[vSize];
		memcpy(vertices, verts, vSize * sizeof(SimpleVertex));

		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = vSize * sizeof(SimpleVertex); // ������ ������ = ������ ����� ������� * 3
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;          // ��� ������ - ����� 
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData; // ���������, ���������� ������ ������
		ZeroMemory(&InitData, sizeof(InitData)); // ������� 
		InitData.pSysMem = vertices;		// ��������� �� �������

		// ����� ������ g_pd3dDevice ������� ������ ������ ������ ID3D11Buffer
		hr = device->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
		return hr;
	}
};

class GameObject
{
public:
	ID3D11Device* g_pd3dDevice = NULL;          // ���������� (��� �������� ��������)
	Mesh* mesh;
	int iSize, vSize;

	GameObject(ID3D11Device* device, HWND hWnd, const ShapeTemplate* templ)
	{
		g_pd3dDevice = device;
		if (templ == nullptr)
		{
			iSize = 24;
			vSize = 6;
		}
		else {
			iSize = templ->iSize;
			vSize = templ->vSize;
		}
		mesh = new Mesh(device, hWnd, templ);
	}
	~GameObject()
	{
		CleanupObject();
	}
	void CleanupObject();
};