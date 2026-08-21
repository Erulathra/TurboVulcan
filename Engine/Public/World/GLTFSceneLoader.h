#pragma once

DECLARE_LOG_CATEGORY(LogGLTFSceneLoader, Display, Display)

namespace Turbo
{
	class World;

	struct FGLTFSceneLoader
	{
		static void LoadGLTFScene(World& world, FName path);
	};
} // Turbo
