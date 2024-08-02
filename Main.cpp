#define STB_IMAGE_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZW

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <vector>
#include <iostream>
#include <math.h> 


#include <shaders/Shader.h>
#include <learnopengl/Camera.h>
#include <learnopengl/Raw_Model.h>
#include <RigidBodies/PhysicsHandler.h>
#include <RigidBodies/Plane.h>
#include <RigidBodies/sphere.h>
#include <shaders/shader_c.h>
#include <ecosystem/Terrain.h>
#include <ecosystem/skybox.h>
#include <learnopengl/model.h>



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void simulate_physics(float time, int& nbFrames, PhysicsManager *physics_handler, float freq, float accumulator);
void get_fps(int& frameCount, float currentTime, float& prev_time);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(10.0f, 0.0f, 25.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastTime = 0.0f;


int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // -----------------------------

    // build and compile shaders
    // -------------------------
    //ComputeShader compute_shader("Shaders/computeShader.cs");

    Frustum scene_frustum = Frustum(&camera);

    Shader main_shader("Shaders/default_shader.vs", "Shaders/default_fragment_shader.fs");
    Shader body_shader("Shaders/rigidBodyShader.vs", "Shaders/rigidBodyShader.fs");
    Shader grass_shader("Shaders/grassShader.vs", "Shaders/grassShader.fs", "Shaders/grass_shader.gs");
    ComputeShader clipping_shader("Shaders/terrainComputeShader.cs");

    main_shader.use();
    main_shader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
    main_shader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
    main_shader.setVec3("lightPos", -30.0f, 50.0f, 20.0f);

    body_shader.use();
    body_shader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
    body_shader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
    body_shader.setVec3("lightPos", -30.0f, 50.0f, 20.0f);

    Sphere sphere = Sphere(&body_shader, 1.0, glm::vec4(5.0, 3.0, 5.0, 1.0), glm::vec4(0.0, 0.0, 0.0, 0.0), glm::vec4(0.0f, 0.0f, 0.0f, 0), glm::vec3(0.53, 0.81, 0.94), 1);
    Sphere sphere2 = Sphere(&body_shader, 0.5, glm::vec4(8.0, 3.0, 9.0, 1.0), glm::vec4(0.0, -15.0, 2.0, 0.0), glm::vec4(0.0f, 0.0f, 5.0f, 0), glm::vec3(0.7, 0.6, 0.3), 1);
    Plane curr_plane = Plane(glm::vec4(0, -5.0f, 0, 1), glm::vec4(0.0, 1.0, 0.0, 0), &body_shader);
    Plane curr_plane2 = Plane(glm::vec4(0, -3.0f, 0, 1), glm::vec4(1.0, 1.0, 0.0, 0), &body_shader);
   
    Terrain* terrain = new Terrain(&main_shader, &grass_shader, &clipping_shader, glm::vec4(0.0f, -5.0f, 0.0f, 1.0), "Textures/clumping2.jpg", "Textures/height_map1.jpg", true );
    //Terrain* terrain2 = new Terrain(&main_shader, &grass_shader, &clipping_shader, glm::vec4(-128.0f, -5.0f, 0.0f, 1.0), "Textures/clumping2.jpg", "Textures/height_map1.jpg", true);
    //Terrain* terrain3 = new Terrain(&main_shader, &grass_shader, &clipping_shader, glm::vec4(0.0f, -5.0f, 128.0f, 1.0), "Textures/clumping2.jpg", "Textures/height_map1.jpg", true);
    //Terrain* terrain4 = new Terrain(&main_shader, &grass_shader, &clipping_shader, glm::vec4(-128.0f, -5.0f, 128.0f, 1.0), "Textures/clumping2.jpg", "Textures/height_map1.jpg", true);
    
    PhysicsManager physics_handler;
    physics_handler.add_sphere(&sphere);
    physics_handler.add_sphere(&sphere2);
    physics_handler.add_plane(&curr_plane);
    //physics_handler.add_plane(&curr_plane2);
    //physics_handler.test_bisection();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // render loop
    // -----------
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    int nbFrames = 0;
    lastTime = glfwGetTime();
    float prev_time = glfwGetTime();
    int frameCount = 0;
    float accumulator = 0;
    while (!glfwWindowShouldClose(window))
    {
        
        float currentTime = static_cast<float>(glfwGetTime());
        deltaTime = currentTime - lastTime;


        get_fps(frameCount, currentTime, prev_time);

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            std::cout << "pressed space\n";
            glm::vec4 input_force = glm::vec4(0.0, 50.0, 0.0, 0.0);
            sphere.input_acc = input_force;
        }

        scene_frustum.update_visibility_planes();
        simulate_physics(currentTime, nbFrames, &physics_handler, 0.01, accumulator);

        // per-frame time logic
        // --------------------
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.52f, 0.8f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        
        //glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 projection = camera.GetProjection(); 
        glm::mat4 view = camera.GetViewMatrix();
        sphere.draw(projection*view, camera.Position);
        sphere2.draw(projection*view, camera.Position);
        terrain->draw(currentTime, projection, view, camera.Position, &scene_frustum);
        //terrain2->draw(currentTime, projection, view, camera.Position, &scene_frustum);
        //terrain3->draw(currentTime, projection, view, camera.Position, & scene_frustum);
        //terrain4->draw(currentTime, projection, view, camera.Position, &scene_frustum);
        //curr_plane.draw(projection, view, camera.Position);
      //  curr_plane2.draw(projection, view, camera.Position);
        lastTime = currentTime;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void get_fps(int &frameCount, float currentTime, float &prev_time) {
    float frameDeltaTime = currentTime - prev_time;
    frameCount++;
    if (frameDeltaTime >= 1.0) {
       // Calculate FPS
       double fps = (double)frameCount / frameDeltaTime;

       // Display FPS
       std::cout << "fps: " << fps << "\n";
       // Reset for next calculation
            
       prev_time = currentTime;
       frameCount = 0;
    }
}

void simulate_physics(float time, int& nbFrames, PhysicsManager *physics_handler, float freq, float accumulator) {

        nbFrames++;
        float delta_time = time - lastTime;
        lastTime = time;
        accumulator += delta_time;
        while (accumulator >= freq) {
            //std::cout << time << "\n";
            // Print the average FPS over the last second
            physics_handler->physics_step(freq);
            
            nbFrames = 0;
            accumulator -= freq;
        }
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{

    float movement_speed = deltaTime * 5.0;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, movement_speed);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, movement_speed);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, movement_speed);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, movement_speed);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}


