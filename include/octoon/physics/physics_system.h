#ifndef OCTOON_PHYSICS_SYSTEM_H_
#define OCTOON_PHYSICS_SYSTEM_H_

#include <octoon/runtime/platform.h>
#include <octoon/runtime/rtti_macros.h>
#include <octoon/runtime/rtti_interface.h>
#include <octoon/math/math.h>

#include <octoon/physics/physics_scene.h>
#include <octoon/physics/physics_rigidbody.h>

namespace octoon
{
    namespace physics
    {
        class OCTOON_EXPORT PhysicsSystem : public runtime::RttiInterface
        {
            OctoonDeclareSubInterface(PhysicsSystem, runtime::RttiInterface)
        public:
            PhysicsSystem() noexcept;
            virtual ~PhysicsSystem() noexcept;

            virtual PhysicsScene * createScene() except;
            virtual PhysicsRigidbody * createRigidbody() except;
        };
    }
}

#endif //OCTOON_PHYSICS_SYSTEM_H_