#pragma once

#include <atlas/tools/MayaCamera.hpp>

namespace a2
{
    class QuaternionCamera : public atlas::tools::MayaCamera
    {
    public:
        QuaternionCamera();

        void setMovement(atlas::tools::MayaMovements movement);

        void mouseDown(atlas::math::Point2 const& point) override;
        void mouseMove(atlas::math::Point2 const& point) override;
        void mouseUp() override;
        void resetCamera() override;


        atlas::math::Matrix4 getCameraMatrix() const override;
        atlas::math::Matrix4 getCameraMatrix2(glm::vec3 position, glm::vec3 target) const;

    private:
        atlas::math::Point mPosition;
        atlas::math::Point mTarget;
        atlas::math::Vector mUp;
        atlas::math::Point2 mLastPos;
        atlas::math::Quaternion mQuat;

        float mFov;
        atlas::tools::MayaMovements mMovement;
    };

}
