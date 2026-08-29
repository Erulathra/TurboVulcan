#include "RuntimeTestLayer.h"
#include "Core/Engine.h"
#include "Rendering/GameViewportLayer.h"

i32 main(int argc, char* argv[])
{
    Turbo::InitEngine(argc, argv);

    Turbo::FLayersStack& layerStack = entt::locator<Turbo::FLayersStack>::value();
    layerStack.PushLayer<Turbo::FGameViewportLayer>();
    Turbo::gEngine->RegisterEngineLayers();

    layerStack.PushLayer<Turbo::FRuntimeTestLayer>();

    return Turbo::gEngine->Start();
}
