#include "Boid.hpp"
#include "Paths.hpp"
#include "LayoutLocations.glsl"
#include <iostream>
#include <vector>
#include <atlas/utils/Mesh.hpp>
#include <atlas/core/STB.hpp>
#include <atlas/core/Float.hpp>
#include <atlas/utils/GUI.hpp>

#define border 50

namespace a2
{
    Boid::Boid() :
    
        mVertexBuffer(GL_ARRAY_BUFFER),
        mIndexBuffer(GL_ELEMENT_ARRAY_BUFFER)
    {

        using atlas::utils::Mesh;
        namespace gl = atlas::gl;
        namespace math = atlas::math;

        Mesh sphere;
        std::string path{ lab2::DataDirectory };
        path = path + "sphere.obj";
        Mesh::fromFile(path, sphere);

        mIndexCount = static_cast<GLsizei>(sphere.indices().size());

        std::vector<float> data;
        for (std::size_t i = 0; i < sphere.vertices().size(); ++i)
        {
            data.push_back(sphere.vertices()[i].x);
            data.push_back(sphere.vertices()[i].y);
            data.push_back(sphere.vertices()[i].z);

            data.push_back(sphere.normals()[i].x);
            data.push_back(sphere.normals()[i].y);
            data.push_back(sphere.normals()[i].z);

            data.push_back(sphere.texCoords()[i].x);
            data.push_back(sphere.texCoords()[i].y);
        }

        mVao.bindVertexArray();
        mVertexBuffer.bindBuffer();
        mVertexBuffer.bufferData(gl::size<float>(data.size()), data.data(),
            GL_STATIC_DRAW);
        mVertexBuffer.vertexAttribPointer(VERTICES_LAYOUT_LOCATION, 3, GL_FLOAT,
            GL_FALSE, gl::stride<float>(8), gl::bufferOffset<float>(0));
        mVertexBuffer.vertexAttribPointer(NORMALS_LAYOUT_LOCATION, 3, GL_FLOAT,
            GL_FALSE, gl::stride<float>(8), gl::bufferOffset<float>(3));
        mVertexBuffer.vertexAttribPointer(TEXTURES_LAYOUT_LOCATION, 2, GL_FLOAT,
            GL_FALSE, gl::stride<float>(8), gl::bufferOffset<float>(6));

        mVao.enableVertexAttribArray(VERTICES_LAYOUT_LOCATION);
        mVao.enableVertexAttribArray(NORMALS_LAYOUT_LOCATION);
        mVao.enableVertexAttribArray(TEXTURES_LAYOUT_LOCATION);

        mIndexBuffer.bindBuffer();
        mIndexBuffer.bufferData(gl::size<GLuint>(sphere.indices().size()),
            sphere.indices().data(), GL_STATIC_DRAW);

        mIndexBuffer.unBindBuffer();
        mVertexBuffer.unBindBuffer();
        mVao.unBindVertexArray();

        std::vector<gl::ShaderUnit> shaders
        {
            {std::string(lab2::ShaderDirectory) + "Golf.vs.glsl", GL_VERTEX_SHADER},
            {std::string(lab2::ShaderDirectory) + "Golf.fs.glsl", GL_FRAGMENT_SHADER}
        };

        mShaders.emplace_back(shaders);
        mShaders[0].setShaderIncludeDir(lab2::ShaderDirectory);
        mShaders[0].compileShaders();
        mShaders[0].linkShaders();

        auto var = mShaders[0].getUniformVariable("model");
        mUniforms.insert(UniformKey("model", var));
        var = mShaders[0].getUniformVariable("projection");
        mUniforms.insert(UniformKey("projection", var));
        var = mShaders[0].getUniformVariable("view");
        mUniforms.insert(UniformKey("view", var));
        var = mShaders[0].getUniformVariable("materialColour");
        mUniforms.insert(UniformKey("materialColour", var));

        mShaders[0].disableShaders();
        
        Position = std::vector<glm::vec3> {
            {0.0 , 2.0 , 0.0 },
            {0.5 , 2.0 , 5.0 },
            {1.0 , 0.0 , 3.0 },
            {-0.5 , 3.0 , -3.0 },
            {-2.0 , 0.0 , 4.0 },
            {1.0 , 0.8 , 7.0 },
            {-0.5 , 3.0 , 0.0 },
            {3.0 , 0.0 , 9.0 },
            {-0.5 , 5.0 , 5.0 },
            {0.0 , 6.0 , -2.0 }
        };
        
        Velocity = std::vector<glm::vec3>{
            {0.0 , 0.1 , 0.0 },
            {0.5 , 0.0 , 0.2 },
            {0.5 , 0.5 , 0.0 },
            {-0.5 , 0.0 , 0.0 },
            {-0.0 , 0.0 , 0.2 },
            {0.0 , 0.8 , 0.5 },
            {-0.5 , 1.0 , 1.0 },
            {0.05 , 0.0 , -0.5 },
            {-0.5 , 1.0 , 0.0 },
            {0.5 , 0.0 , 0.5 }
        };
        
        Accelerate = std::vector<glm::vec3>{
            {0.0 , 0.0 , 0.0 },
            {0.1 , 0.0 , 0.0 },
            {0.0 , 0.0 , 0.0 },
            {0.0 , 0.2 , 0.0 },
            {0.0 , 0.0 , 0.0 },
            {0.0 , 0.0 , 0.0 },
            {0.0 , 0.0 , -0.2 },
            {0.0 , 0.0 , 0.0 },
            {0.0 , 0.0 , 0.0 },
            {0.0 , 0.3 , 0.0 }
        };
    
        r = 1.0f;
        maxSpeed = 1.8f;
        maxForce = 0.05f;
        starColor = glm::vec3(0.71f, 0.44f, 0.19f);
        mSize = glm::vec3( 0.3f,0.3f,0.3f );
    }
    
    //     Wraparound
    void Boid::borders(int index) {
        if (Position[index].x > border || Position[index].y > border || Position[index].z > border || Position[index].x < -border || Position[index].y < -border || Position[index].z < -border) {
            Velocity[index] *= -0.5;
            Position[index] += Velocity[index];
        }
    }
    
    // Check collision avoid
    void Boid::collisionVoid(int index, TrackBall &barrier) {
        float d = glm::distance(Position[index],barrier.mPosition);
        glm::vec3 coeeff(0.08f,0.08f,0.08f);
        if (d <= r + barrier.radius) {
            Velocity[index] = Velocity[index] - coeeff * barrier.mPosition;
            // Limit speed
            float magnitudeVelocity = sqrt(dot(Velocity[index], Velocity[index]));
            if(magnitudeVelocity > maxSpeed) {
                Velocity[index] = glm::normalize(Velocity[index]) * maxSpeed;
            }
            Position[index] += Velocity[index];
        }
    }
    
    // We accumulate a new acceleration each time based on three rules
    void Boid::flock(TrackBall &barrier) {
        for(int i = 0; i < Position.size(); i++) {
            glm::vec3 sep = separate(i);   // Separation
            glm::vec3 ali = align(i);      // Alignment
            glm::vec3 coh = cohesion(i);   // Cohesion
            
            // Arbitrarily weight these forces
            sep = 1.0f * sep;
            ali = 1.0f * ali;
            coh = 1.0f * coh;
            
            // Add the force vectors to acceleration
            Accelerate[i] += sep;
            Accelerate[i] += ali;
            Accelerate[i] += coh;
            
            // Update velocity
            Velocity[i] += Accelerate[i];
            
            // Limit speed
            float magnitudeVelocity = sqrt(dot(Velocity[i], Velocity[i]));
            if(magnitudeVelocity > maxSpeed) {
                Velocity[i] = glm::normalize(Velocity[i]) * maxSpeed;
            }
            Position[i] += Velocity[i];
            
//            std::cout << "Before check borders: " << '\n';
//            std::cout << "Position =  " << "(" << Position[i].x << "," << Position[i].y << "," << Position[i].z << ")"<< '\n';
//            std::cout << "Velocity =  " << "(" << Velocity[i].x << "," << Velocity[i].y << "," << Velocity[i].z << ")"<< '\n';
//             std::cout << "Accelerate =  " << "(" << Accelerate[i].x << "," << Accelerate[i].y << "," << Accelerate[i].z << ")"<< '\n';
            
            // Check if it beyonds the border
            borders(i);
            
            // Reset accelertion to 0 each cycle
            Accelerate[i] *= glm::vec3(0,0,0);
        }
    }
    
    
    // A method that calculates a steering vector towards a target
    // Takes a second argument, if true, it slows down as it approaches the target
    glm::vec3 Boid::seek(glm::vec3 target, int i){
        // A vector pointing from the position to the target
        glm::vec3 desired = target - Position[i];
        // Normalize desired and scale to maximum speed
        desired = glm::normalize(desired) * maxSpeed;
        // Steering = Desired minus Velocity
        glm::vec3 steer = desired - Velocity[i];
        // Limit to maximum steering force
        float magnitudeSteer = sqrt(dot(steer, steer));
        if(magnitudeSteer > maxForce) {
            steer = glm::normalize(steer) * maxForce;
        }
        return steer;
    }

    
    // Separation
    // Method checks for nearby boids and steers away
    glm::vec3 Boid::separate(int index) {
//        float desiredseparation = 5.0f;
        float desiredseparation = 10.0f;
        glm::vec3 sum(0,0,0);
        int count = 0;
        for(int j = 0; j < Position.size(); j++){
            if(index != j){
                float distance =  sqrt(dot(Position[index], Position[j]));
                // If the distance is greater than 0 and less than an arbitrary amount (0 when you are yourself)
                if ((distance > 0) && (distance < desiredseparation)) {
                    // Calculate vector pointing away from neighbor
                    glm::vec3 diff = Position[index] - Position[j];
                    diff = glm::normalize(diff);
                    diff /= distance;        // Weight by distance
                    sum += diff;
                    count++;
                }
            }
            // Average -- divide by how many
            if (count > 0) {
                sum /= (float)count;
            }
            if (glm::length(sum) > 0) {
                sum = glm::normalize(sum);
                sum *= maxSpeed;
                sum -= Velocity[index];
                // Limit to maximum sum force
                float magnitudeSum = sqrt(dot(sum, sum));
                if(magnitudeSum > maxForce) {
                    sum = glm::normalize(sum) * maxForce;
                }
            }
        }
        return sum;
    }
    
    
    // Alignment
    // For every nearby boid in the system, calculate the average velocity
    glm::vec3 Boid::align (int index) {
        float neighbordist = 20.0f;
//        float neighbordist = 10.0f;
        glm::vec3 sum(0,0,0);
        int count = 0;
        for(int j = 0; j < Position.size(); j++){
            if(index != j){
                float distance =  sqrt(dot(Position[index], Position[j]));
                // If the distance is greater than 0 and less than an arbitrary amount (0 when you are yourself)
                if ((distance > 0) && (distance < neighbordist)) {
                    sum += Velocity[j];
                    count++;
                }
            }
            if (count > 0) {
                sum /= (float)count;
            }
            if (glm::length(sum) > 0) {
                sum = glm::normalize(sum);
                sum *= maxSpeed;
                sum -= Velocity[index];
                // Limit to maximum sum force
                float magnitudeSum = sqrt(dot(sum, sum));
                if(magnitudeSum > maxForce) {
                    sum = glm::normalize(sum) * maxForce;
                }
            }
        }
        return sum;
    }
    
    
    
    // Cohesion
    // For the average location (i.e. center) of all nearby boids, calculate steering vector towards that location
    glm::vec3 Boid::cohesion(int index){
//        float neighbordist = 10.0f;
        float neighbordist = 20.0f;
        glm::vec3 sum(0,0,0);   // Start with empty vector to accumulate all locations
        int count = 0;
        for(int j = 0; j < Position.size(); j++){
            if(index != j){
                float distance =  sqrt(dot(Position[index], Position[j]));
                // If the distance is greater than 0 and less than an arbitrary amount (0 when you are yourself)
                if ((distance > 0) && (distance < neighbordist)) {
                    sum += Velocity[j];
                    count++;
                }
            }
            if (count > 0) {
                sum /= (float)count;
                return seek(sum, index);
            }
        }
        return sum;
    }
    
    glm::vec3 Boid::getPosition(int index){
        return Position[index];
    }
    glm::vec3 Boid::getVelocity(int index){
        return Velocity[index];
    }
    
    void Boid::updateGeometry(atlas::core::Time<> const& t, bool &mPlay, TrackBall &barrier)
    {
        flock(barrier);
        for(int i = 0; i < Position.size(); i++) {
            // Check if it accross the barrier
            collisionVoid(i, barrier);
        }
    }

    
    void Boid::renderGeometry(atlas::math::Matrix4 const& projection, atlas::math::Matrix4 const& view)
    {
       
        namespace math = atlas::math;

        mShaders[0].hotReloadShaders();
        if (!mShaders[0].shaderProgramValid())
        {
            return;
        }

        mShaders[0].enableShaders();

        mVao.bindVertexArray();
        mIndexBuffer.bindBuffer();

        glUniformMatrix4fv(mUniforms["projection"], 1, GL_FALSE,
            &projection[0][0]);
        glUniformMatrix4fv(mUniforms["view"], 1, GL_FALSE, &view[0][0]);

        for(int i = 0; i < Position.size(); i++) {
            auto model = glm::translate(math::Matrix4(1.0f), Position[i]);
            model = glm::scale(model, mSize);
            glUniformMatrix4fv(mUniforms["model"], 1, GL_FALSE, &model[0][0]);
            glUniform3fv(mUniforms["materialColour"], 1, &starColor[0]);
            glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_INT, 0);
        }

        mIndexBuffer.unBindBuffer();
        mVao.unBindVertexArray();
        mShaders[0].disableShaders();
    }

    void Boid::resetGeometry()
    {
        Position = std::vector<glm::vec3> {
            {0.0 , 2.0 , 0.0 },
            {0.5 , 2.0 , 5.0 },
            {1.0 , 0.0 , 3.0 },
            {-0.5 , 3.0 , -3.0 },
            {-2.0 , 0.0 , 4.0 },
            {1.0 , 0.8 , 7.0 },
            {-0.5 , 3.0 , 0.0 },
            {3.0 , 0.0 , 9.0 },
            {-0.5 , 5.0 , 5.0 },
            {0.0 , 6.0 , -2.0 }
        };
        
        Velocity = std::vector<glm::vec3>{
            {0.0 , 0.1 , 0.0 },
            {0.5 , 0.0 , 0.2 },
            {0.5 , 0.5 , 0.0 },
            {-0.5 , 0.0 , 0.0 },
            {-0.0 , 0.0 , 0.2 },
            {0.0 , 0.8 , 0.5 },
            {-0.5 , 1.0 , 1.0 },
            {0.05 , 0.0 , -0.5 },
            {-0.5 , 1.0 , 0.0 },
            {0.5 , 0.0 , 0.5 }
        };
        
        Accelerate = std::vector<glm::vec3>{
            {0.0 , 0.0 , 0.0 },
            {0.1 , 0.0 , 0.0 },
            {0.0 , 0.0 , 0.0 },
            {0.0 , 0.2 , 0.0 },
            {0.0 , 0.0 , 0.0 },
            {0.0 , 0.0 , 0.0 },
            {0.0 , 0.0 , -0.2 },
            {0.0 , 0.0 , 0.0 },
            {0.0 , 0.0 , 0.0 },
            {0.0 , 0.3 , 0.0 }
        };
    }

}
