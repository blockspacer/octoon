#include <octoon/video/text_material.h>
#include <octoon/video/render_system.h>
#include <octoon/runtime/except.h>

namespace octoon
{
	namespace video
	{
		TextMaterial::TextMaterial() except
		{
			this->setup();
		}

		void
		TextMaterial::setup() except
		{
#if defined(OCTOON_BUILD_PLATFORM_EMSCRIPTEN) || defined(OCTOON_BUILD_PLATFORM_ANDROID)
			const char* vert = R"(
			precision mediump float;
			uniform mat4 proj;
			uniform mat4 model;
			uniform float lean;
			uniform vec3 frontColor;
			uniform vec3 sideColor;
			uniform vec3 translate;

			attribute vec4 POSITION0;
			attribute vec4 NORMAL0;

			varying vec3 oTexcoord0;

			void main()
			{
				vec4 P = POSITION0;
				P.x -= P.y * lean;
				if (P.z == 0.0)
					P.xyz += translate;

				if (abs(NORMAL0.z) > 0.5)
					oTexcoord0 = frontColor;
				else
					oTexcoord0 = sideColor;

				gl_Position = proj * model * P;
			})";

			const char* frag = R"(
			precision mediump float;
			varying vec3 oTexcoord0;
			void main()
			{
				gl_FragColor = vec4(oTexcoord0, 1.0);
			})";
#else
			const char* vert = R"(#version 330
			uniform mat4 proj;
			uniform mat4 model;
			uniform float lean;
			uniform vec3 frontColor;
			uniform vec3 sideColor;
			uniform vec3 translate;

			layout(location  = 0) in vec4 POSITION0;
			layout(location  = 1) in vec4 NORMAL0;

			out vec3 oTexcoord0;

			void main()
			{
				vec4 P = POSITION0;
				P.x -= P.y * lean;
				if (P.z == 0)
					P.xyz += translate;

				if (abs(NORMAL0.z) > 0.5)
					oTexcoord0 = frontColor;
				else
					oTexcoord0 = sideColor;

				gl_Position = proj * model * P;
			})";

			const char* frag = R"(#version 330
			layout(location  = 0) out vec4 fragColor;
			in vec3 oTexcoord0;
			void main()
			{
				fragColor = vec4(oTexcoord0, 1.0f);
			})";
#endif
			graphics::GraphicsProgramDesc programDesc;
			programDesc.addShader(RenderSystem::instance()->createShader(graphics::GraphicsShaderDesc(graphics::GraphicsShaderStageFlagBits::VertexBit, vert, "main", graphics::GraphicsShaderLang::GLSL)));
			programDesc.addShader(RenderSystem::instance()->createShader(graphics::GraphicsShaderDesc(graphics::GraphicsShaderStageFlagBits::FragmentBit, frag, "main", graphics::GraphicsShaderLang::GLSL)));
			auto program = RenderSystem::instance()->createProgram(programDesc);

			graphics::GraphicsInputLayoutDesc layoutDesc;
			layoutDesc.addVertexLayout(graphics::GraphicsVertexLayout(0, "POSITION", 0, graphics::GraphicsFormat::R32G32B32SFloat));
			layoutDesc.addVertexLayout(graphics::GraphicsVertexLayout(0, "NORMAL", 0, graphics::GraphicsFormat::R32G32B32SFloat));
			layoutDesc.addVertexBinding(graphics::GraphicsVertexBinding(0, layoutDesc.getVertexSize()));

			graphics::GraphicsDescriptorSetLayoutDesc descriptor_set_layout;
			descriptor_set_layout.setUniformComponents(program->getActiveParams());

			graphics::GraphicsStateDesc stateDesc;
			stateDesc.setPrimitiveType(graphics::GraphicsVertexType::TriangleList);
			stateDesc.setCullMode(graphics::GraphicsCullMode::None);
			stateDesc.setDepthEnable(true);

			graphics::GraphicsPipelineDesc pipeline;
			pipeline.setGraphicsInputLayout(RenderSystem::instance()->createInputLayout(layoutDesc));
			pipeline.setGraphicsState(RenderSystem::instance()->createRenderState(stateDesc));
			pipeline.setGraphicsProgram(std::move(program));
			pipeline.setGraphicsDescriptorSetLayout(RenderSystem::instance()->createDescriptorSetLayout(descriptor_set_layout));

			pipeline_ = RenderSystem::instance()->createRenderPipeline(pipeline);
			if (!pipeline_)
				return;

			graphics::GraphicsDescriptorSetDesc descriptorSet;
			descriptorSet.setGraphicsDescriptorSetLayout(pipeline.getDescriptorSetLayout());
			descriptorSet_ = RenderSystem::instance()->createDescriptorSet(descriptorSet);
			if (!descriptorSet_)
				return;

			auto begin = descriptorSet_->getUniformSets().begin();
			auto end = descriptorSet_->getUniformSets().end();

			translate_ = *std::find_if(begin, end, [](const graphics::GraphicsUniformSetPtr& set) { return set->getName() == "translate"; });
			proj_ = *std::find_if(begin, end, [](const graphics::GraphicsUniformSetPtr& set) { return set->getName() == "proj"; });
			model_ = *std::find_if(begin, end, [](const graphics::GraphicsUniformSetPtr& set) { return set->getName() == "model"; });
			lean_ = *std::find_if(begin, end, [](const graphics::GraphicsUniformSetPtr& set) { return set->getName() == "lean"; });
			frontColor_ = *std::find_if(begin, end, [](const graphics::GraphicsUniformSetPtr& set) { return set->getName() == "frontColor"; });
			sideColor_ = *std::find_if(begin, end, [](const graphics::GraphicsUniformSetPtr& set) { return set->getName() == "sideColor"; });
		}

		TextMaterial::~TextMaterial() noexcept
		{
		}

		void
		TextMaterial::setTransform(const math::float4x4& m) noexcept
		{
			model_->uniform4fmat(m);
		}

		void
		TextMaterial::setViewProjection(const math::float4x4& vp) noexcept
		{
			proj_->uniform4fmat(vp);
		}

		const graphics::GraphicsPipelinePtr&
		TextMaterial::getPipeline() const noexcept
		{
			return pipeline_;
		}

		const graphics::GraphicsDescriptorSetPtr&
		TextMaterial::getDescriptorSet() const noexcept
		{
			return descriptorSet_;
		}

		void
		TextMaterial::setLean(float lean) noexcept
		{
			lean_->uniform1f(lean);
		}

		void
		TextMaterial::setTextColor(TextColor::Type which, const math::float3& colors) except
		{
			switch (which)
			{
			case octoon::video::TextColor::FrontColor:
				frontColor_->uniform3f(colors);
				break;
			case octoon::video::TextColor::SideColor:
				sideColor_->uniform3f(colors);
				break;
			default:
				throw runtime::out_of_range::create("Unknown enum type of text color");
			}
		}

		void
		TextMaterial::setTranslate(const math::float3& translate) noexcept
		{
			translate_->uniform3f(translate);
		}

		float
		TextMaterial::getLean() const noexcept
		{
			return lean_->getFloat();
		}

		const math::float3&
		TextMaterial::getTranslate() const noexcept
		{
			return translate_->getFloat3();
		}

		const math::float3&
		TextMaterial::getTextColor(TextColor::Type which) const except
		{
			switch (which)
			{
			case octoon::video::TextColor::FrontColor:
				return frontColor_->getFloat3();
			case octoon::video::TextColor::SideColor:
				return sideColor_->getFloat3();
			default:
				throw runtime::out_of_range::create("Unknown enum type of text color");
			}
		}

		MaterialPtr
		TextMaterial::clone() const noexcept
		{
			auto instance = std::make_shared<TextMaterial>();
			instance->setTranslate(this->getTranslate());
			instance->setLean(this->getLean());
			instance->setTextColor(TextColor::FrontColor, this->getTextColor(TextColor::FrontColor));
			instance->setTextColor(TextColor::SideColor, this->getTextColor(TextColor::SideColor));

			return instance;
		}
	}
}