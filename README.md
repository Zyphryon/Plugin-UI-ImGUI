# ImGui Plugin for Zyphryon Engine

This plugin integrates [Dear ImGui](https://github.com/ocornut/imgui) with [Zyphryon](https://github.com/Zyphryon/Engine).  

---

## Features

- Full Dear ImGui integration (UI context, styling, docking, navigation).
- Plug-and-play: minimal boilerplate required to start using ImGui in your project.
- ImGui speaks the engine's types (`ImGuiExtensions.hpp`), with no standard library involved.

---

## Usage

```cpp
#include <ImGuiSystem.hpp>
#include <ImGuiExtensions.hpp>

ZyPlugin::ImGuiSystem mImGui;

// During initialization
mImGui.Initialize(Host);

// In your frame loop
mImGui.Begin(Time);

// Use ImGui normally, with the engine's types where they fit
ImGui::Begin("Demo");
ImGui::Text<"Hello from Zyphryon + ImGui, frame {0}">(Frame);
ImGui::InputText("Name", mName);
ImGui::Combo("Mode", mMode);
ImGui::End();

// Render UI
mImGui.End();
```

## 📄 License

This plugin is licensed under the MIT License – see the LICENSE file for details.

## 🤝 Contributing

We welcome contributions! Please feel free to submit Pull Requests, open Issues, or discuss new features in Discussions.