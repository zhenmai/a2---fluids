#pragma once
#include "Boid.hpp"
#include "TrackBall.hpp"
#include "QuaternionCamera.hpp"
#include "Spline.hpp"
#include <atlas/tools/ModellingScene.hpp>
#include <atlas/utils/FPSCounter.hpp>
#include <atlas/tools/MayaCamera.hpp>
#include <atlas/tools/Grid.hpp>


namespace a2
{
    class BoidScene : public atlas::tools::ModellingScene
    {
    public:
        BoidScene();
        void updateScene(double time) override;
        void renderScene() override;
        
        //Add
        void mousePressEvent(int button, int action, int modifiers,
                             double xPos, double yPos) override;
        void mouseMoveEvent(double xPos, double yPos) override;
        void mouseScrollEvent(double xOffset, double yOffset) override;

    private:
        bool mPlay;
        int mFPSOption;
        float mFPS;

        atlas::core::Time<float> mAnimTime;
        atlas::utils::FPSCounter mCounter;
        float cameraRotate;
        
        // Add
        Boid initial;
        int mCameraMode;
        QuaternionCamera mQuatCamera;
        Spline mSpline;
        atlas::math::Point railPosition;
        TrackBall Barrier;
        

    };
}

