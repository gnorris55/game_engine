#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    float aspect;
    float zNear = 0.1f;
    float zFar = 100.0f;
    // euler Angles
    float Yaw;
    float Pitch;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    float fovY = glm::radians(ZOOM);

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), float aspect = 16.0f/9.0f, glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        this->aspect = aspect;
        updateCameraVectors();
    }
    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    glm::mat4 GetProjection() {
        return glm::perspective(glm::radians(Zoom), aspect, zNear, zFar);
    }

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
        fovY = glm::radians(Zoom);
    }

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up = glm::normalize(glm::cross(Right, Front));
    }
};


class Frustum {


public:
    struct Plane {
        glm::vec3 normal;
        glm::vec3 position;
    };
    Camera *camera;

    struct VisibilityPlanes {
        Plane near;
        Plane far;
        Plane left;
        Plane right;
        Plane bottom;
        Plane top;
    };

    VisibilityPlanes visibility_planes;

    Frustum(Camera* camera) {
        this->camera = camera;
        update_visibility_planes();
    }

    void update_visibility_planes() {

        const float halfVSide = camera->zFar * tanf(camera->fovY * 2.0* .5f);
        const float halfHSide = halfVSide * camera->aspect;
        const glm::vec3 frontMultFar = camera->zFar * camera->Front;

        set_plane(&visibility_planes.near, camera->Position + camera->zNear * camera->Front, camera->Front);
        set_plane(&visibility_planes.far, camera->Position+frontMultFar, -camera->Front);
        set_plane(&visibility_planes.right, camera->Position, glm::cross(frontMultFar - camera->Right * halfHSide, camera->Up));
        set_plane(&visibility_planes.left, camera->Position, glm::cross(camera->Up, frontMultFar + camera->Right * halfHSide));
        set_plane(&visibility_planes.top, camera->Position, glm::cross(camera->Right, frontMultFar - camera->Up * halfVSide));
        set_plane(&visibility_planes.bottom, camera->Position, glm::cross(frontMultFar + camera->Up * halfVSide, camera->Right));
        //std::cout << "bottom position: " << glm::to_string(visibility_planes.bottom.position) << "\n";
        //std::cout << "bottom normal: " << glm::to_string(visibility_planes.bottom.normal) << "\n";
    }

    bool in_frustum(glm::vec3 extents, glm::vec3 center) {

        return (
            over_plane(&visibility_planes.right, extents, center)
            && over_plane(&visibility_planes.left, extents, center)
            && over_plane(&visibility_planes.top, extents, center)
            && over_plane(&visibility_planes.bottom, extents, center)
            //&& over_plane(&visibility_planes.near, extents, center)
            && over_plane(&visibility_planes.far, extents, center));

    }

    bool over_plane(Plane* plane, glm::vec3 extents, glm::vec3 center) {
        //glm::vec3 difference = point - plane->position;
        //return glm::dot(difference, plane->normal) >= 0.0f;
        
        const float r = extents.x * std::abs(plane->normal.x) +
            extents.y * std::abs(plane->normal.y) + extents.z * std::abs(plane->normal.z);

        //std::cout << "r: " << r << "\n";
        //std::cout << "dis to plane: " << getSignedDistanceToPlane(plane, center) << "\n";



        return -r <= getSignedDistanceToPlane(plane, center);
    }

    float getSignedDistanceToPlane(Plane *plane, glm::vec3 point) {
        glm::vec3 difference = point - plane->position;
        //return glm::dot(point, plane->normal) - glm::dot(plane->position,plane->normal);
        return glm::dot(difference, plane->normal);
    }

    //glm::vec3 difference = point - plane->position;
        //return glm::dot(difference, plane->normal) >= 0.0f;

    void set_plane(Plane* plane, glm::vec3 position, glm::vec3 normal) {
        plane->position = position;
        plane->normal = glm::normalize(normal);
    }




};
#endif