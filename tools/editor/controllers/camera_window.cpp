#include "camera_window.h"

#include <octoon/octoon.h>
#include <octoon/ui/imgui.h>

namespace octoon
{
	namespace editor
	{
		OctoonImplementSubClass(CameraWindow, GameComponent, "CameraWindow")

		CameraWindow::CameraWindow() noexcept
			: framebufferSizeW_(0)
			, framebufferSizeH_(0)
			, isShowedCameraWindow_(true)
		{
		}

		CameraWindow::~CameraWindow() noexcept
		{
		}

		void
		CameraWindow::onActivate() noexcept
		{
			this->addComponentDispatch(GameDispatchType::Gui);

			camera_ = GameObject::create("MainCamera");
			camera_->getComponent<TransformComponent>()->setTranslate(math::float3(0, 0, -200));
			camera_->addComponent<FirstPersonCameraComponent>();

			auto camera = camera_->addComponent<PerspectiveCameraComponent>(45.0f);
			camera->setCameraType(video::CameraType::Custom);
			camera->setClearColor(octoon::math::float4(0.1f, 0.2f, 0.3f, 1.0f));
		}

		void
		CameraWindow::onDeactivate() noexcept
		{
			this->removeComponentDispatchs();

			camera_.reset();
		}

		void 
		CameraWindow::onGui() noexcept
		{
			auto cameraComponent = camera_->getComponent<CameraComponent>();

			if (imgui::begin_dock("Camera", &isShowedCameraWindow_, imgui::GuiWindowFlagBits::AlwaysUseWindowPaddingBit | imgui::GuiWindowFlagBits::NoScrollWithMouseBit))
			{
				imgui::set_scroll_y(imgui::get_style_default().WindowPadding.y);

				auto size = imgui::get_window_size();

				if (framebufferSizeW_ != size.x || framebufferSizeH_ != size.y)
				{
#if defined(OCTOON_BUILD_PLATFORM_EMSCRIPTEN)
					camera_->getComponent<CameraComponent>()->setupFramebuffers((std::uint32_t)size.x, (std::uint32_t)size.y);
#else
					camera_->getComponent<CameraComponent>()->setupFramebuffers((std::uint32_t)size.x, (std::uint32_t)size.y, 4);
					camera_->getComponent<CameraComponent>()->setupSwapFramebuffers((std::uint32_t)size.x, (std::uint32_t)size.y);
#endif
					framebufferSizeW_ = (std::uint32_t)size.x;
					framebufferSizeH_ = (std::uint32_t)size.y;
				}

#if defined(OCTOON_BUILD_PLATFORM_EMSCRIPTEN)
				auto framebuffer = cameraComponent->getFramebuffer();
#else
				auto framebuffer = cameraComponent->getSwapFramebuffer();
#endif
				if (framebuffer)
				{
					auto texture = framebuffer->getGraphicsFramebufferDesc().getColorAttachment().getBindingTexture();
					if (texture)
						imgui::image(texture.get(), size, math::float2::UnitY, math::float2::UnitX);
				}

				imgui::end_dock();
			}
		}

		GameComponentPtr 
		CameraWindow::clone() const noexcept
		{
			return std::make_shared<CameraWindow>();
		}
	}
}