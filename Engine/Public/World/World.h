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

	struct World
	{
	   /* Properties */
   	entt::registry mRegistry;
   	FRuntimeLevel mRuntimeLevel;

      /* Level loading */
		void OpenLevel(FName path);
		void UnloadLevel();
	};
} // Turbo
