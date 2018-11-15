#include "FlockScene.hpp"
#include <atlas/utils/GUI.hpp>
#include <atlas/gl/GL.hpp>
#include <atlas/core/Log.hpp>
#include "glm/ext.hpp"
#include <iostream>
#include <atlas/utils/Application.hpp>

namespace a2
{
    BoidScene::BoidScene() :
        mPlay(false),
        mFPSOption(0),
        mFPS(25.0f), // Speed
        mCounter(mFPS),
        cameraRotate(45.0f),
        mCameraMode(0),
        mSpline(int(10.0f * mFPS)),
        Barrier(glm::vec3(5,5,5),2)
    { }

    void BoidScene::updateScene(double time)
    {
        using atlas::core::Time;
        
        ModellingScene::updateScene(time);
        if (mPlay && mCounter.isFPS(mTime))
        {
            const float delta = 1.0f / mFPS;
            mAnimTime.currentTime += delta;
            mAnimTime.deltaTime = delta;
            mAnimTime.totalTime = mAnimTime.currentTime;
            //Add
            initial.updateGeometry(mAnimTime, mPlay, Barrier);
            mSpline.updateGeometry(mAnimTime);
            railPosition = mSpline.getPosition();
        }
    }

    void BoidScene::mousePressEvent(int button, int action, int modifiers,
        double xPos, double yPos)
    {
        using atlas::tools::MayaMovements;
        atlas::utils::Gui::getInstance().mousePressed(button, action, modifiers);

        if (action == GLFW_PRESS)
        {
            atlas::math::Point2 point(xPos, yPos);

            if (button == GLFW_MOUSE_BUTTON_LEFT &&
                modifiers == GLFW_MOD_ALT)
            {
                if (mCameraMode == 0)
                {
                    mCamera.setMovement(MayaMovements::Tumble);
                    mCamera.mouseDown(point);
                }
                else
                {
                    mQuatCamera.setMovement(MayaMovements::Tumble);
                    mQuatCamera.mouseDown(point);
                }
            }
            else if (button == GLFW_MOUSE_BUTTON_MIDDLE &&
                modifiers == GLFW_MOD_ALT)
            {
                if (mCameraMode == 0)
                {
                    mCamera.setMovement(MayaMovements::Track);
                    mCamera.mouseDown(point);
                }
                else
                {
                    mQuatCamera.setMovement(MayaMovements::Track);
                    mQuatCamera.mouseDown(point);
                }
            }
            else if (button == GLFW_MOUSE_BUTTON_RIGHT &&
                modifiers == GLFW_MOD_ALT)
            {
                if (mCameraMode == 0)
                {
                    mCamera.setMovement(MayaMovements::Dolly);
                    mCamera.mouseDown(point);
                }
                else
                {
                    mQuatCamera.setMovement(MayaMovements::Dolly);
                    mQuatCamera.mouseDown(point);
                }
            }
        }
        else
        {
            if (mCameraMode == 0)
            {
                mCamera.mouseUp();
            }
            else
            {
                mQuatCamera.mouseUp();
            }
        }
    }

    void BoidScene::mouseMoveEvent(double xPos, double yPos)
    {
        atlas::utils::Gui::getInstance().mouseMoved(xPos, yPos);
        if (mCameraMode == 0)
        {
            mCamera.mouseMove(atlas::math::Point2(xPos, yPos));
        }
        else if (mCameraMode == 1)
        {
            mQuatCamera.mouseMove(atlas::math::Point2(xPos, yPos));
        }
    }

    void BoidScene::mouseScrollEvent(double xOffset, double yOffset)
    {
        atlas::utils::Gui::getInstance().mouseScroll(xOffset, yOffset);

        if (mCameraMode == 0)
        {
            mCamera.mouseScroll(atlas::math::Point2(xOffset, yOffset));
        }
        else if (mCameraMode == 1)
        {
            mQuatCamera.mouseScroll(atlas::math::Point2(xOffset, yOffset));
        }
    }


    void BoidScene::renderScene()
    {
        using atlas::utils::Gui;
        
        Gui::getInstance().newFrame();
        const float grey = 92.0f / 255.0f;
        glClearColor(grey, grey, grey, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        mProjection = glm::perspective(
            glm::radians(mCamera.getCameraFOV()*2),
            (float)mWidth / mHeight, 1.0f, 100000000.0f);
        
        // Add: to switch the camera mode.
        switch (mCameraMode){
            case 0:
            {
                // Global view
                float radius = sqrt(1800);
                float camX = sin(cameraRotate) * radius;
                float camZ = cos(cameraRotate) * radius;
                GLFWwindow* window = atlas::utils::Application::getInstance().getCurrentWindow();
                if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
                    cameraRotate -= 0.01f;
                    mCamera.setCameraPosition(glm::vec3(camX, 30.0f, camZ));
                }
                if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
                    cameraRotate += 0.01f;
                    mCamera.setCameraPosition(glm::vec3(camX, 30.0f, camZ));
                }
                mView = mCamera.getCameraMatrix();
                break;
            }
            case 1:
            {
                // Rail-way view
                auto railTarget = glm::vec3(0,0,0);
                mView = mQuatCamera.getCameraMatrix2(railPosition,railTarget);
                break;
            }
            case 2:
            {
                // First-view of the second boid by getPosition(1) and getVelocity(1).
                auto cameraPosition = initial.getPosition(1);
                auto cameraTarget = cameraPosition + initial.getVelocity(1);
                mView = mQuatCamera.getCameraMatrix2(cameraPosition,cameraTarget);
                break;
            }
            default:
                break;
        }

//        Add
        initial.renderGeometry(mProjection, mView);
        Barrier.renderGeometry(mProjection, mView);
        mGrid.renderGeometry(mProjection, mView);
        mSpline.renderGeometry(mProjection, mView);

        // Global HUD
        ImGui::SetNextWindowSize(ImVec2(350, 140), ImGuiSetCond_FirstUseEver);
        ImGui::Begin("Camera options");
        ImGui::Text("Use 'A' and 'D' to control the Camera!");
        if (ImGui::Button("Reset Camera"))
        {
            mCamera.resetCamera();
            cameraRotate = 45.0f;
        }

        if (mPlay)
        {
            if (ImGui::Button("Pause"))
            {
                mPlay = !mPlay;
            }
        }
        else
        {
            if (ImGui::Button("Play"))
            {
                mPlay = !mPlay;
            }
        }

        if (ImGui::Button("Reset"))
        {
            initial.resetGeometry();
            mAnimTime.currentTime = 0.0f;
            mAnimTime.totalTime = 0.0f;
            mPlay = false;
        }

//        ImGui::Text("Application average %.3f ms/frame (%.1FPS)",
//            1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

//        std::vector<const char*> options = { "60 FPS", "30 FPS", "20 FPS",
//        "10 FPS", "5 FPS", "1 FPS" };
//        ImGui::Combo("Choose FPS: ", &mFPSOption, options.data(),
//            ((int)options.size()));

        // Add
        std::vector<const char*> options = { "Global Camera", "Rail-Way View", "First-person View"};

        ImGui::Combo("Camera mode: ", &mCameraMode, options.data(),
            ((int)options.size()));

        ImGui::End();

//        switch (mFPSOption)
//        {
//        case 0:
//            mFPS = 60.0f;
//            break;
//
//        case 1:
//            mFPS = 30.0f;
//            break;
//
//        case 2:
//            mFPS = 20.0f;
//            break;
//
//        case 3:
//            mFPS = 10.0f;
//            break;
//
//        case 4:
//            mFPS = 5.0f;
//            break;
//
//        case 5:
//            mFPS = 1.0f;
//            break;
//
//        default:
//            break;
//        }
//        mCounter.setFPS(mFPS);

        ImGui::Render();
    }
}
