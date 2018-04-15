#ifndef OCTOON_FIXED_JOINT2D_COMPONENT_H_
#define OCTOON_FIXED_JOINT2D_COMPONENT_H_

#include <memory>
#include <octoon/gameComponent.h>
#include <octoon/anchoredJoint2dComponent.h>
#include <octoon/joint2dComponent.h>
#include <octoon/math/math.h>


class b2Body;
class b2Fixture;
class b2RevoluteJoint;

namespace octoon
{
    class OCTOON_EXPORT FixedJoint2D : public AnchoredJoint2D
	{
        OctoonDeclareSubInterface(FixedJoint2D, AnchoredJoint2D)
        public:
            FixedJoint2D() noexcept;
            ~FixedJoint2D();
            virtual GameComponentPtr clone() const noexcept;
        
        protected:
            virtual void onJointChange() override;
            virtual void onJointEnter() override;
            virtual void onJointExit() override;
        private:
            b2RevoluteJoint *joint;

            friend class Rigidbody2D;
    };
}


#endif // OCTOON_FIXED_JOINT2D_COMPONENT_H_