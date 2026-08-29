#pragma once

#include "Core/DataStructures/Array.h"

namespace Turbo
{
	class FCamera;
	struct FMesh;
	struct FTexture;

	struct FSpawnedByLevelTag {};

	struct World
	{
   	TArray<THandle<FMesh>> mLoadedMeshes;
   	TArray<THandle<FTexture>> mLoadedTextures;

	   /* Properties */
   	entt::registry mRegistry;

      /* Level loading */
      void Init(Engine* engine);
		void OpenLevel(Engine* engine, FName path);
		void UnloadLevel();
	};
} // Turbo
