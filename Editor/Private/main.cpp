#include "EditorLayer.h"
#include "RenderingTestLayer.h"
#include "Core/Engine.h"
#include "Layers/Layer.h"

i32 main(int argc, char* argv[])
{
    Turbo::InitEngine(argc, argv);

    Turbo::FLayersStack& layerStack = entt::locator<Turbo::FLayersStack>::value();
    Turbo::gEngine->RegisterEngineLayers();

    layerStack.PushLayer<Turbo::FEditorLayer>();
    layerStack.PushLayer<Turbo::FRenderingTestLayer>();

    return Turbo::gEngine->Start();
}
