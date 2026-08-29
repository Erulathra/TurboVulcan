#include "World/World.h"

#include "Graphics/PostProcess.h"
#include "ProfilingMacros.h"
#include "World/GLTFSceneLoader.h"
#include "Assets/AssetManager.h"
#include "World/SceneGraph.h"

using namespace entt::literals;

namespace Turbo
{
   void World::Init(Engine* engine)
   {
      new(this) World();

   	entt::entity ppSettingsEntity = mRegistry.create();
   	mRegistry.emplace<FEntityLabel>(ppSettingsEntity, FName("PostProcessSettings"));
   	mRegistry.emplace<FWorldRoot>(ppSettingsEntity);
   	mRegistry.emplace<FPostProcessSettings>(ppSettingsEntity);
   	mRegistry.emplace<FWorldSettings>(ppSettingsEntity);
   }

	void World::OpenLevel(Engine* engine, FName path)
	{
      TRACE_ZONE_SCOPED_FORMAT(OpenLevel, "Open Level ({})", path.ToString())

		std::filesystem::path scenePath(path.ToString());
		if (scenePath.extension() == ".glb" || scenePath.extension() == ".gltf")
		{
			FGLTFSceneLoader::LoadGLTFScene(engine, path);
		}
	}

	void World::UnloadLevel()
	{
		const auto spawnedByLevelView = mRegistry.view<FSpawnedByLevelTag>();
		mRegistry.destroy(spawnedByLevelView.begin(), spawnedByLevelView.end());

		FAssetManager& assetManager = entt::locator<FAssetManager>::value();
		for (THandle<FMesh> mesh : mLoadedMeshes)
		{
			assetManager.UnloadMesh(mesh);
		}

		for (THandle<FTexture> texture : mLoadedTextures)
		{
			assetManager.UnloadTexture(texture);
		}
	}


} // Turbo
