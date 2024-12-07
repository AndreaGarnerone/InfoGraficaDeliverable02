#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <random>
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_s.h"
#include "camera.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
glm::vec3 generateRandomPosition();

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

float plateVerteces[] = {
    // first triangle
    0.15f, 0.10f, 0.01f,    1.0f, 1.0f,  // top right
    0.15f, -0.10f, 0.01f,   1.0f, 0.0f,  // bottom right
    -0.15f, 0.10f, 0.01f,   0.0f, 1.0f,  // top left

    // second triangle 
    0.15f, -0.10f, 0.01f,   1.0f, 0.0f,  // bottom right
    -0.15f, -0.10f, 0.01f,  0.0f, 0.0f,  // bottom left
    -0.15f, 0.10f, 0.01f,   0.0f, 1.0f   // top left
};
glm::vec3 platePosition = { 0.0f, -1.10f, 0.0f };

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Example19", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // glfwSetCursorPosCallback(window, mouse_callback); // for moving the view with the mouse
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

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("shader.vs", "shader.fs");

    float conveyorBeltVertices[] = {
        // first triangle
        0.60f, 1.20f, -0.01f,    1.0f, 1.0f,  // top right
        0.60f, -1.20f, -0.01f,   1.0f, 0.0f,  // bottom right
        -0.60f, 1.20f, -0.01f,   0.0f, 1.0f,  // top left
        
        // second triangle 
        0.60f, -1.20f, -0.01f,   1.0f, 0.0f,  // bottom right
        -0.60f, -1.20f, -0.01f,  0.0f, 0.0f,  // bottom left
        -0.60f, 1.20f, -0.01f,   0.0f, 1.0f   // top left
    };
    glm::vec3 conveyorBeltPosition = {0.0f, 0.0f, 0.0f};

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float objectsVertices[] = {
        // first (upper) triangle
        0.10f, 0.10f, 0.0f,    1.0f, 1.0f,  // top right
        0.10f, -0.10f, 0.0f,   1.0f, 0.0f,  // bottom right
        -0.10f, 0.10f, 0.0f,   0.0f, 1.0f,  // top left

        // second triangle 
        0.10f, -0.10f, 0.0f,   1.0f, 0.0f,  // bottom right
        -0.10f, -0.10f, 0.0f,  0.0f, 0.0f,  // bottom left
        -0.10f, 0.10f, 0.0f,   0.0f, 1.0f   // top left
    };
    // world space positions of the objects
    
    glm::vec3 objectsPositions[] = { // 20
        glm::vec3(-0.45f, 1.20f,  -0.0f),
        glm::vec3(0.45f,  1.20f,  -0.0f),
        glm::vec3(0.0f,   1.20f,  -0.0f),
        glm::vec3(0.5f,   1.20f,  -0.0f),
        glm::vec3(-0.5f,  1.20f,  -0.0f),
        glm::vec3(-0.15f, 1.20f,  -0.0f),
        glm::vec3(0.3f,   1.20f,  -0.0f),
        glm::vec3(0.2f,   1.20f,  -0.0f),
        glm::vec3(-0.5f,  1.20f,  -0.0f),
        glm::vec3(0.1f,   1.20f,  -0.0f),
        glm::vec3(-0.0f,  1.20f,  -0.0f),
        glm::vec3(0.25f,  1.20f,  -0.0f),
        glm::vec3(-0.1f,  1.20f,  -0.0f),
        glm::vec3(0.5f,   1.20f,  -0.0f),
        glm::vec3(-0.35f, 1.20f,  -0.0f),
        glm::vec3(0.15f,  1.20f,  -0.0f),
        glm::vec3(-0.45f, 1.20f,  -0.0f),
        glm::vec3(0.0f,   1.20f,  -0.0f),
        glm::vec3(-0.2f,  1.20f,  -0.0f),
        glm::vec3(0.3f,   1.20f,  -0.0f),
    };
    /*
    std::vector<glm::vec3> objectsPositions;
    for (unsigned int i = 0; i < 7; ++i) { // Adjust 7 to the number of positions needed
        objectsPositions.push_back(generateRandomPosition());
    }
    */

    // Declare VAO and VBO for the conveyor belt
    unsigned int conveyorVAO, conveyorVBO;
    glGenVertexArrays(1, &conveyorVAO);
    glGenBuffers(1, &conveyorVBO);

    // Bind and set the conveyor belt vertices
    glBindVertexArray(conveyorVAO);

    glBindBuffer(GL_ARRAY_BUFFER, conveyorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(conveyorBeltVertices), conveyorBeltVertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0); // Unbind VAO


    // PLATE
    // --------------------------------------------------------------------------------------------------
    // Declare VAO and VBO for the conveyor belt
    unsigned int plateVAO, plateVBO;
    glGenVertexArrays(1, &plateVAO);
    glGenBuffers(1, &plateVBO);

    glBindVertexArray(plateVAO);

    glBindBuffer(GL_ARRAY_BUFFER, plateVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(plateVerteces), plateVerteces, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0); // Unbind VAO



    // CUBES
    // --------------------------------------------------------------------------------------------------
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(objectsVertices), objectsVertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // load and create a texture 
    // -------------------------
    unsigned int texture1, texture2, texture3;

    // texture 1
    // ---------
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char* data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    // texture 2
    // ---------
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    data = stbi_load("awesomeface.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        // note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    // texture 3
    // ---------
    glGenTextures(1, &texture3);
    glBindTexture(GL_TEXTURE_2D, texture3);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    data = stbi_load("conveyorBelt.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------
    ourShader.use();
    ourShader.setInt("texture1", 0);
    ourShader.setInt("texture2", 1);
    ourShader.setInt("texture3", 2);

    float activationTime[] = { 0.0f, 2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f, 14.0f, 16.0f, 18.0f, 20.0f, 22.0f, 24.0f, 26.0f, 28.0f, 30.0f, 32.0f, 34.0f, 36.0f, 38.0f, 40.0f }; // Base activation time

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // Activate shader
        ourShader.use();

        // Bind textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        ourShader.setInt("texture1", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);
        ourShader.setInt("texture2", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, texture3);
        ourShader.setInt("texture3", 2);

        // Set projection and view matrices
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("view", view);

        // Render conveyor belt
        ourShader.use();
        glBindVertexArray(conveyorVAO);
        // Apply transformations if needed
        glm::mat4 model = glm::translate(glm::mat4(1.0f), conveyorBeltPosition);
        ourShader.setMat4("model", model);
        ourShader.setInt("textureID", 3);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Render plate
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glBindVertexArray(plateVAO);
        model = glm::translate(glm::mat4(1.0f), platePosition);
        ourShader.setMat4("model", model);
        ourShader.setInt("textureID", 1);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // render cubes
        glBindVertexArray(plateVAO);
        glBindVertexArray(conveyorVAO);
        ourShader.setInt("textureID", 2);
        glBindVertexArray(VAO);

        for (unsigned int i = 0; i < 20; i++) { // Loop for the cubes
            float currentTime = static_cast<float>(glfwGetTime());
            if (currentTime < activationTime[i]) {
                continue;
            }

            if (objectsPositions[i].y <= -1.10f) {
                continue;
            }

            // Update position
            objectsPositions[i].y -= 0.0005f;

            model = glm::mat4(1.0f);
            model = glm::translate(model, objectsPositions[i]);
            ourShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        
        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &conveyorVAO);
    glDeleteBuffers(1, &conveyorVBO);

    glDeleteVertexArrays(1, &plateVAO);
    glDeleteBuffers(1, &plateVBO);

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

glm::vec3 generateRandomPosition() {
    static std::random_device rd; // Seed
    static std::mt19937 gen(rd()); // Random number generator
    static std::uniform_real_distribution<float> distX(-0.50f, 0.50f); // Distribution for x

    float randomX = distX(gen);
    return glm::vec3(randomX, 1.20f, 0.0f); // Fixed y and z
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        if (platePosition.x <= 0.45f)   // if plate in border
            platePosition.x += 0.001f;  // Move plate right
        else platePosition.x = 0.45f;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        if (platePosition.x >= -0.45f)  // if plate in border
            platePosition.x -= 0.001f;  // Move plate left
        else platePosition.x = -0.45f;
    }

    /*
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    */
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