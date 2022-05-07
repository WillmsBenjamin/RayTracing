#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"

using namespace Walnut;

namespace RayTracingConstants
{
	static constexpr char* kResName4k = "3840x2160";
	static constexpr uint32_t kWidth4k = 3840;
	static constexpr uint32_t kHeight4k = 2160;
	static constexpr std::tuple<uint32_t, uint32_t> kRes4k(kWidth4k, kHeight4k);

	static constexpr char* kResName2k = "2560x1440";
	static constexpr uint32_t kWidth2k = 2560;
	static constexpr uint32_t kHeight2k = 1440;
	static constexpr std::tuple<uint32_t, uint32_t> kRes2k(kWidth2k, kHeight2k);

	static constexpr char* kResNameFullHD = "1920x1080";
	static constexpr uint32_t kWidthFullHD = 1920;
	static constexpr uint32_t kHeightFullHD = 1080;
	static constexpr std::tuple<uint32_t, uint32_t> kResFullHD(kWidthFullHD, kHeightFullHD);

	static constexpr char* kResNameHD = "1280x720";
	static constexpr uint32_t kWidthHD = 1280;
	static constexpr uint32_t kHeightHD = 720;
	static constexpr std::tuple<uint32_t, uint32_t> kResHD(kWidthHD, kHeightHD);

	static constexpr char* kResNameLow = "640x480";
	static constexpr uint32_t kWidthLow = 640;
	static constexpr uint32_t kHeightLow = 480;
	static constexpr std::tuple<uint32_t, uint32_t> kResLow(kWidthLow, kHeightLow);

	static constexpr char* kAvailableResolutions[] = { kResNameLow, kResNameHD, kResNameFullHD, kResName2k, kResName4k };

	static const std::unordered_map<const char*, std::tuple<uint32_t, uint32_t>> kResolutionMap({ { kResName4k, kRes4k }, { kResName2k, kRes2k }, { kResNameFullHD, kResFullHD }, { kResNameHD, kResHD }, { kResNameLow, kResLow } });

	static const bool TryChooseResolution(const char* ResolutionName, uint32_t& OutWidth, uint32_t& OutHeight)
	{
		auto ResolutionIt = kResolutionMap.find(ResolutionName);
		
		if (ResolutionIt == kResolutionMap.end())
		{
			return false;
		}

		std::tie(OutWidth, OutHeight) = ResolutionIt->second;
		return true;
	}
}
using namespace RayTracingConstants;

class RayTracingLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		HandleSettingsPanel();
		HandleViewport();
	}

private:
	void HandleViewport()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");

		//ViewportWidth = (uint32_t)ImGui::GetContentRegionAvail().x;
		//ViewportHeight = (uint32_t)ImGui::GetContentRegionAvail().y;

		if (RenderedImage != nullptr)
		{
			const ImVec2 ImageSize((float)RenderedImage->GetWidth(), (float)RenderedImage->GetHeight());
			ImGui::Image(RenderedImage->GetDescriptorSet(), ImageSize);
		}

		ImGui::End();
		ImGui::PopStyleVar();

		if (ShouldRender)
		{
			Render();
		}
	}

	void HandleSettingsPanel()
	{
		ImGui::Begin("Settings");

		ImGui::Text("Last render: %.3fms", LastRenderTime);

		HandleRenderingSettings();

		ImGui::End();
	}

	void HandleRenderingSettings()
	{
		ImGui::BeginVertical("Rendering");
		
		ImGui::Checkbox("Realtime", &IsRealtime);

		if (ImGui::BeginCombo("Resolution", CurrentResolution))
		{
			for (int I = 0; I < IM_ARRAYSIZE(kAvailableResolutions); I++)
			{
				const bool IsSelected = (CurrentResolution == kAvailableResolutions[I]);
				if (!ImGui::Selectable(kAvailableResolutions[I], IsSelected))
				{
					continue;
				}
				
				CurrentResolution = kAvailableResolutions[I];
				if (IsSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();
		}

		ImGui::EndVertical();

		TryChooseResolution(CurrentResolution, RenderingWidth, RenderingHeight);

		ImGui::BeginHorizontal("RenderButtons");

		const bool ShouldStart = ImGui::Button("Render");
		bool ShouldStop = false;

		if (ShouldRender && IsRealtime)
		{
			ShouldStop = ImGui::Button("Stop");
		}

		ImGui::EndHorizontal();

		ShouldRender = !ShouldStop &&
			(ShouldStart || (ShouldRender && IsRealtime));
	}

	void Render()
	{
		Timer RenderTimer;

		if (!IsRenderedImageValid())
		{
			RenderedImage = std::make_shared<Image>(RenderingWidth, RenderingHeight, ImageFormat::RGBA);
			delete[] ImageData;
			ImageData = new uint32_t[RenderingWidth * RenderingHeight];
		}

		for (uint32_t I = 0; I < RenderingWidth * RenderingHeight; I++)
		{
			ImageData[I] = Random::UInt();
			//Sets alpha to 255
			ImageData[I] |= 0xff000000;
		}

		RenderedImage->SetData(ImageData);

		LastRenderTime = RenderTimer.ElapsedMillis();
	}

	bool IsRenderedImageValid() const
	{
		return RenderedImage != nullptr
			&& RenderedImage->GetWidth() == RenderingWidth
			&& RenderedImage->GetHeight() == RenderingHeight;
	}

	//Rendering
	std::shared_ptr<Image> RenderedImage;
	uint32_t* ImageData = nullptr;
	uint32_t RenderingWidth = 0;
	uint32_t RenderingHeight = 0;


	//UI
	bool IsRealtime = false;
	bool ShouldRender = false;
	float LastRenderTime = 0.0f;
	const char* CurrentResolution = kResNameLow;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Ray Tracing";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<RayTracingLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}