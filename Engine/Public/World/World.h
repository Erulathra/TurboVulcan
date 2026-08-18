#pragma once

#include "Core/DataStructures/Array.h"

namespace Turbo
{
	class FCamera;
	struct FMesh;
	struct FTexture;

	struct FSpawnedByLevelTag {};

	struct FRuntimeLevel
	{
		TArray<THandle<FMesh>> mLoadedMeshes;
		TArray<THandle<FTexture>> mLoadedTextures;
	};

	class FWorld
	{
	public:
		void OpenLevel(FName path);
		void UnloadLevel();

	public:
		entt::registry mRegistry;
		FRuntimeLevel mRuntimeLevel;
	};
} // Turbo
