#pragma once
#include "framework.h"

class BufferManager 
{
private:
	ID3D11Device* g_pd3dDevice = NULL;

	ID3D11Buffer* g_pIndexBuffer = NULL;        // Буфер индексов вершин
	ID3D11Buffer* g_pConstantBuffer = NULL;			// Константный буфер
	ID3D11Buffer* g_pVertexBuffer = NULL;         // Буфер вершин

public:
	BufferManager(ID3D11Device* pd3dDevice)
	{
		g_pd3dDevice = pd3dDevice;
	}
	~BufferManager()
	{
		CleanupManager();
	}

	HRESULT CreateBuffer(void* source, int size, UINT BindFlags, ID3D11Buffer** destination);
	HRESULT SetupIndexBuffer(void* source, int size);
	HRESULT SetupVertexBuffer(void* source, int size);
	HRESULT SetupConstantBuffer(void* source, int size);

	void CleanupManager();

	ID3D11Buffer* getConstantBuffer() { return g_pConstantBuffer; }
	ID3D11Buffer* getVertexBuffer() { return g_pVertexBuffer; }
	ID3D11Buffer* getIndexBuffer() { return g_pIndexBuffer; }
};