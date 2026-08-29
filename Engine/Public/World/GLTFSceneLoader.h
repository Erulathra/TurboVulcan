#pragma once

DECLARE_LOG_CATEGORY(LogGLTFSceneLoader, Display, Display)

namespace Turbo
{
	class World;

	struct FGLTFSceneLoader
	{
		static void LoadGLTFScene(Engine* engine, FName path);
	};
} // Turbo
