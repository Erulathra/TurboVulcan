#pragma once

[vk_binding(0, 0)]
public __DynamicResource<__DynamicResourceKind::General> texturePool[];

[vk_binding(1, 0)]
public __DynamicResource<__DynamicResourceKind::General> rwTexturePool[];

[vk_binding(2, 0)]
public SamplerState samplerPool[];

[vk_binding(3, 0)]
public RaytracingAccelerationStructure tlasPool[];

struct FBufferBindingTable
{
   Ptr<void> mData[1024];
};

struct FTextureBindingTable
{
   uint mData[1024];
};

[vk_binding(0, 1)]
ConstantBuffer<FBufferBindingTable> rgBufferAddressTable;

[vk_binding(1, 1)]
ConstantBuffer<FTextureBindingTable> rgTextureBindingTable;

public Texture2D<T> GetRGTexture2D<T>(uint RGTextureId) where T : ITexelElement
{
   return texturePool[rgTextureBindingTable.mData[RGTextureId]];
}

public RWTexture2D<T> GetRWRGTexture2D<T>(uint RGTextureId) where T : ITexelElement
{
   return rwTexturePool[rgTextureBindingTable.mData[RGTextureId]];
}

public Ptr<T> GetRGBufferAddress<T>(uint RGBufferId)
{
   return Ptr<T>(rgBufferAddressTable.mData[RGBufferId]);
}
