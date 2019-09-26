#include "pch.h"
#include "BufferManager.h"

HRESULT BufferManager::CreateBuffer(void* source, int size, UINT BindFlags, ID3D11Buffer** destination)
{
	D3D11_BUFFER_DESC bd;	// Структура, описывающая создаваемый буфер
	ZeroMemory(&bd, sizeof(bd));				// очищаем ее
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = size;	// размер буфера
	bd.BindFlags = BindFlags;	// тип буфера 
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;	// Структура, содержащая данные буфера
	ZeroMemory(&InitData, sizeof(InitData));	// очищаем ее
	InitData.pSysMem = source;				// указатель на наши данные

// Вызов метода CreateBuffer объекта Device создаст объект буфера
	return g_pd3dDevice->CreateBuffer(&bd, &InitData, destination);
}

HRESULT BufferManager::SetupIndexBuffer(void* source, int size)
{
	return CreateBuffer(source, size, D3D11_BIND_INDEX_BUFFER, &g_pIndexBuffer);
}

HRESULT BufferManager::SetupVertexBuffer(void* source, int size)
{
	return CreateBuffer(source, size, D3D11_BIND_VERTEX_BUFFER, &g_pVertexBuffer);
}

HRESULT BufferManager::SetupConstantBuffer(void* source, int size)
{
	return CreateBuffer(source, size, D3D11_BIND_CONSTANT_BUFFER, &g_pConstantBuffer);
}

void BufferManager::CleanupManager()
{
	if (g_pConstantBuffer) g_pConstantBuffer->Release();
	if (g_pVertexBuffer) g_pVertexBuffer->Release();
	if (g_pIndexBuffer) g_pIndexBuffer->Release();
}
