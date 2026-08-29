#include "Core/Engine.h"
#include "Core/Platform.h"
#include "Editor.h"
#include "Layers/Event.h"
#include "World/World.h"

namespace Turbo
{
	void Start(void* editor, Engine* engine);
	void Shutdown(void* editor, Engine* engine);
	void Tick(void* editor, Engine* engine, fp32 deltaTime);
	void CopyPresentTexture(
		void* editor,
		Engine* engine,
		RenderGraph* graphBuilder,
		FRGResourceHandle presentTexture
	);
	void HandleEvent(void* userData, Engine* engine, FEventBase& event);

	void LoadLevel(Engine* engine);
}

i32 main(int argc, char* argv[])
{
	using namespace Turbo;

	PlatformMemory platformMemory = {};
	FPlatform::AllocateGameMemory(&platformMemory);

	RuntimeModule* editorModule = (RuntimeModule*)platformMemory.mPersistentData.Allocate(sizeof(PlatformMemory));
	editorModule->mUserData = platformMemory.mPersistentData.Allocate(sizeof(Editor));
	editorModule->mfStart = Start;
	editorModule->mfShutdown = Shutdown;
	editorModule->mfTick = Tick;
	editorModule->mfCopyPresentTexture = CopyPresentTexture;
	editorModule->mfHandleEvent = HandleEvent;

	Engine* engine = InitEngine(&platformMemory, argc, argv);
	engine->mRuntimeModule = editorModule;
	engine->Start(&platformMemory);

	FPlatform::FreeGameMemory(&platformMemory);

	return 0;
}

namespace Turbo
{

	void Start(void* editor, Engine* engine)
	{
		((Editor*)editor)->Start(engine);
		LoadLevel(engine);
	}

	void LoadLevel(Engine* engine)
	{
		using namespace Turbo;

		World* world = engine->mWorld;
		world->OpenLevel(engine, FName("Content/External/main_sponza/compressed/NewSponza_Main_glTF_003.gltf"));
	}

	void Shutdown(void* editor, Engine* engine)
	{
      ((Editor*)editor)->Shutdown(engine);
	}

	void Tick(void* editor, Engine* engine, fp32 deltaTime)
	{
   	((Editor*)editor)->Tick(engine, deltaTime);
	}

	void CopyPresentTexture(
		void* editor,
		Engine* engine,
		RenderGraph* graphBuilder,
		FRGResourceHandle presentTexture
	)
	{
   	((Editor*)editor)->CopyPresentTexture(engine, graphBuilder, presentTexture);
	}

	void HandleEvent(void* editor, Engine* engine, FEventBase& event)
	{
   	((Editor*)editor)->HandleEvent(event);
	}
} // namespace Turbo
