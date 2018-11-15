#pragma once

#include <atlas/utils/Geometry.hpp>
#include <atlas/gl/Buffer.hpp>
#include <atlas/gl/VertexArrayObject.hpp>
#include <atlas/gl/Texture.hpp>
#include "TrackBall.hpp"

namespace a2
{
    class Boid : public atlas::utils::Geometry
    {
    public:
        Boid();

        void updateGeometry(atlas::core::Time<> const& t, bool &mPlay, TrackBall &barrier);
        void renderGeometry(atlas::math::Matrix4 const& projection,
            atlas::math::Matrix4 const& view) override;
        void resetGeometry() override;

        
        // Self-defined functions
        // index from 0 to 9
        void borders(int index);
        void collisionVoid(int index, TrackBall &barrier);
        void flock(TrackBall &barrier);
        glm::vec3 seek(glm::vec3 target, int i);
        glm::vec3 separate(int index);
        glm::vec3 align (int index);
        glm::vec3 cohesion(int index);
        glm::vec3 getPosition(int index);
        glm::vec3 getVelocity(int index);

    private:
        void eulerIntegrator(atlas::core::Time<> const& t);
        void implicitEulerIntegrator(atlas::core::Time<> const& t);
        void verletIntegrator(atlas::core::Time<> const& t);

        atlas::gl::Buffer mVertexBuffer;
        atlas::gl::Buffer mIndexBuffer;
        atlas::gl::VertexArrayObject mVao;

        
        float r, maxForce, maxSpeed;
        std::vector<glm::vec3> Position;
        std::vector<glm::vec3> Velocity;
        std::vector<glm::vec3> Accelerate;
        atlas::math::Vector starColor;
        atlas::math::Vector mSize;


        GLsizei mIndexCount;
    };
}
