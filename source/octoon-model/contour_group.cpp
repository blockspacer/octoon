#include <octoon/model/contour_group.h>
#include <octoon/model/mesh.h>

#include <cstring>
#include <iostream>
#include <GL/glu.h>

namespace octoon
{
	namespace model
	{
		static std::vector<math::float3> g_tris;

		void APIENTRY beginCallback(GLenum which)
		{
			g_tris.clear();
		}

		void APIENTRY endCallback(void)
		{
		}

		void APIENTRY flagCallback(GLboolean)
		{
		}

		void APIENTRY errorCallback(GLenum errorCode)
		{
			std::cerr << "Tessellation Error:" << ::gluErrorString(errorCode) << std::endl;
		}

		void APIENTRY vertexCallback(GLvoid* vertex)
		{
			const GLdouble *d = (GLdouble *)vertex;
			g_tris.emplace_back((float)d[0], (float)d[1], (float)d[2]);
		}

		void APIENTRY combineCallback(GLdouble coords[3], GLdouble* points[4], GLfloat weight[4], GLdouble* dataOut[3])
		{
		}

		ContourGroup::ContourGroup() noexcept
		{
		}

		ContourGroup::ContourGroup(Contours&& contour) noexcept
		{
			this->setContours(std::move(contour));
		}

		ContourGroup::ContourGroup(const Contours& contour) noexcept
		{
			this->setContours(contour);
		}

		ContourGroup::~ContourGroup() noexcept
		{
		}

		void
		ContourGroup::setContours(Contours&& contour) noexcept
		{
			contours_ = std::move(contour);
		}

		void
		ContourGroup::setContours(const Contours& textContour) noexcept
		{
			Contours contours;

			for (auto& it : textContour)
			{
				auto contour = std::make_unique<Contour>();
				contour->points() = it->points();
				contour->isClockwise(it->isClockwise());

				contours.push_back(std::move(contour));
			}

			contours_ = std::move(contours);
		}

		Contours&
		ContourGroup::getContours() noexcept
		{
			return contours_;
		}

		const Contours&
		ContourGroup::getContours() const noexcept
		{
			return contours_;
		}

		std::size_t
		ContourGroup::count() const noexcept
		{
			return contours_.size();
		}

		Contour&
		ContourGroup::at(std::size_t index)  noexcept
		{
			assert(index < count());
			return *contours_[index];
		}

		const Contour&
		ContourGroup::at(std::size_t index) const noexcept
		{
			assert(index < count());
			return *contours_[index];
		}

		void
		ContourGroup::normalize(math::float3& center) noexcept
		{
			math::float3 min(std::numeric_limits<float>::max());
			math::float3 max(-std::numeric_limits<float>::max());

			for (auto& contour : contours_)
			{
				for (auto& it : contour->points())
				{
					min = math::min(it, min);
					max = math::max(it, max);
				}
			}

			center = min + (max - min) * 0.5f;

			*this -= center;
		}

		ContourGroupPtr
		ContourGroup::clone() const noexcept
		{
			auto instance = std::make_shared<ContourGroup>();
			instance->setContours(this->getContours());
			return instance;
		}

		Mesh makeMesh(const Contours& contours, float thickness) noexcept
		{
			Mesh mesh;
			math::float3s tris(max_count(contours) * 6);
			math::float3* trisData = tris.data();
			math::float3s& trisMesh = mesh.getVertexArray();

			std::vector<math::double3> vertices(sum(contours) * 2);

			float thicknessHalf = thickness * 0.5f;

			for (auto& contour : contours)
			{
				std::size_t written = 0;

				for (std::size_t n = 0; n < contour->count(); ++n)
				{
					auto& p1 = contour->at(n);
					auto& p2 = (n == contour->count() - 1) ? contour->at(0) : contour->at(n + 1);

					trisData[written++] = math::float3(p1.x, p1.y, -thicknessHalf);
					trisData[written++] = math::float3(p2.x, p2.y, thicknessHalf);
					trisData[written++] = math::float3(p1.x, p1.y, thicknessHalf);
					trisData[written++] = math::float3(p1.x, p1.y, -thicknessHalf);
					trisData[written++] = math::float3(p2.x, p2.y, -thicknessHalf);
					trisData[written++] = math::float3(p2.x, p2.y, thicknessHalf);
				}

				trisMesh.resize(trisMesh.size() + written);
				std::memcpy(trisMesh.data() + (trisMesh.size() - written), trisData, written * sizeof(math::float3));

				if (contour->isClockwise())
				{
					GLUtesselator* tobj = gluNewTess();

					gluTessCallback(tobj, GLU_TESS_BEGIN, (void(APIENTRY *) ()) &beginCallback);
					gluTessCallback(tobj, GLU_TESS_END, (void(APIENTRY *) ()) &endCallback);
					gluTessCallback(tobj, GLU_TESS_VERTEX, (void(APIENTRY *) ()) &vertexCallback);
					gluTessCallback(tobj, GLU_TESS_ERROR, (void(APIENTRY *) ()) &errorCallback);
					gluTessCallback(tobj, GLU_TESS_COMBINE, (void(APIENTRY *) ()) &combineCallback);
					gluTessCallback(tobj, GLU_TESS_EDGE_FLAG, (void(APIENTRY *) ()) &flagCallback);

					gluTessProperty(tobj, GLU_TESS_TOLERANCE, 0);
					gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);

					gluTessNormal(tobj, 0.0f, 0.0f, 1.0f);

					std::size_t index = 0;

					for (std::uint8_t face = 0; face < 2; face++)
					{
						gluTessBeginPolygon(tobj, nullptr);

						for (auto& contour : contours)
						{
							gluTessBeginContour(tobj);

							for (auto& it : contour->points())
							{
								auto& d = vertices[index++];
								d[0] = it.x;
								d[1] = it.y;
								d[2] = it.z + face ? -thicknessHalf : thicknessHalf;

								gluTessVertex(tobj, d.ptr(), d.ptr());
							}

							gluTessEndContour(tobj);
						}

						gluTessEndPolygon(tobj);

						trisMesh.resize(trisMesh.size() + g_tris.size());
						std::memcpy(trisMesh.data() + (trisMesh.size() - g_tris.size()), g_tris.data(), g_tris.size() * sizeof(math::float3));
					}

					gluDeleteTess(tobj);
				}
			}

			mesh.computeVertexNormals();

			return mesh;
		}

		Mesh makeMesh(const ContourGroup& group, float thickness) noexcept
		{
			return makeMesh(group.getContours(), thickness);
		}

		Mesh makeMesh(const ContourGroups& groups, float thickness) noexcept
		{
			Mesh mesh;

			for (auto& group : groups)
				mesh.combineMeshes(makeMesh(*group, thickness), true);

			return mesh;
		}

		Mesh makeMeshWireframe(const Contours& contours, float thickness) noexcept
		{
			Mesh mesh;

			math::float3s& tris = mesh.getVertexArray();
			math::uint1s& indices = mesh.getIndicesArray();

			float thicknessHalf = thickness * 0.5f;

			for (auto& contour : contours)
			{
				for (std::size_t n = 0; n < contour->count(); ++n)
				{
					auto& p1 = contour->at(n);
					auto& p2 = (n == contour->count() - 1) ? contour->at(0) : contour->at(n + 1);

					math::float3 a = math::float3(p1.x, p1.y, p1.z + thicknessHalf);
					math::float3 b = math::float3(p1.x, p1.y, p1.z + -thicknessHalf);
					math::float3 c = math::float3(p2.x, p2.y, p2.z + -thicknessHalf);
					math::float3 d = math::float3(p2.x, p2.y, p2.z + thicknessHalf);

					std::uint32_t index = tris.size();
					indices.push_back(index);
					indices.push_back(index + 1);
					indices.push_back(index + 1);
					indices.push_back(index + 2);
					indices.push_back(index + 2);
					indices.push_back(index + 3);
					indices.push_back(index + 3);
					indices.push_back(index);

					tris.push_back(a);
					tris.push_back(b);
					tris.push_back(c);
					tris.push_back(d);
				}
			}

			return mesh;
		}

		Mesh makeMeshWireframe(const ContourGroup& group, float thickness) noexcept
		{
			return makeMeshWireframe(group.getContours(), thickness);
		}

		Mesh makeMeshWireframe(const ContourGroups& groups, float thickness) noexcept
		{
			Mesh mesh;

			math::float3s& tris = mesh.getVertexArray();
			math::uint1s& indices = mesh.getIndicesArray();

			float thicknessHalf = thickness * 0.5f;

			for (auto& group : groups)
			{
				for (auto& contour : group->getContours())
				{
					for (std::size_t n = 0; n < contour->count(); ++n)
					{
						auto& p1 = contour->at(n);
						auto& p2 = (n == contour->count() - 1) ? contour->at(0) : contour->at(n + 1);

						math::float3 a = math::float3(p1.x, p1.y, thicknessHalf);
						math::float3 b = math::float3(p1.x, p1.y, -thicknessHalf);
						math::float3 c = math::float3(p2.x, p2.y, -thicknessHalf);
						math::float3 d = math::float3(p2.x, p2.y, thicknessHalf);

						auto index = tris.size();
						indices.push_back(index);
						indices.push_back(index + 1);
						indices.push_back(index + 1);
						indices.push_back(index + 2);
						indices.push_back(index + 2);
						indices.push_back(index + 3);
						indices.push_back(index + 3);
						indices.push_back(index);

						tris.push_back(a);
						tris.push_back(b);
						tris.push_back(c);
						tris.push_back(d);
					}
				}
			}

			return mesh;
		}
	}
}