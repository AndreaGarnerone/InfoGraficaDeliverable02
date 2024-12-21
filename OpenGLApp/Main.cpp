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
#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <irrKlang.h>
using namespace irrklang;

// Collision handling
	// AABB (Axis-Aligned Bounding Box) structure
struct AABB {
	glm::vec3 min;  // Minimum corner (x, y, z)
	glm::vec3 max;  // Maximum corner (x, y, z)
};

struct Character {
	unsigned int TextureID; // ID handle of the glyph texture
	glm::ivec2 Size; // Size of glyph
	glm::ivec2 Bearing; // Offset from baseline to left/top of glyph
	unsigned int Advance; // Horizontal offset to advance to next glyph
};

std::map<char, Character> Characters;
unsigned int txtVAO, txtVBO;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
glm::vec3 generateRandomPosition();
AABB createAABB(const glm::vec3& position);
bool checkCollision(const AABB& a, const AABB& b);
void renderText(Shader& s, std::string text, float x, float y, float scale, glm::vec3 color);
void initFreetype(const char* fontPath);


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

ISoundEngine* soundEngine = createIrrKlangDevice();

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
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Collect it!", NULL, NULL);
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

	// Text handling
	// --------------------------------------

	// compile and setup the shader
	// ----------------------------
	Shader shader("text.vs", "text.fs");
	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
	shader.use();
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	shader.use();
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
		return -1;
	}
	std::string font_name = "resources/fonts/Antonio/static/Antonio-Bold.ttf";
	FT_Face face;
	if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
		return -1;
	}

	if (!soundEngine) {
		std::cerr << "Could not initialize irrKlang sound engine" << std::endl;
		return -1;
	}

	FT_Set_Pixel_Sizes(face, 0, 48);

	if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
	{
		std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
		return -1;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

	for (unsigned char c = 0; c < 128; c++)
	{
		// load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}
		// generate texture
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// now store character for later use
		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		Characters.insert(std::pair<char, Character>(c, character));
	}

	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	// configure VAO/VBO for texture quads
// -----------------------------------
	glGenVertexArrays(1, &txtVAO);
	glGenBuffers(1, &txtVBO);
	glBindVertexArray(txtVAO);
	glBindBuffer(GL_ARRAY_BUFFER, txtVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//----------- END text handling

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
	glm::vec3 conveyorBeltPositions[] = {
	 glm::vec3(0.0f, 0.0f, 0.0f),
	 glm::vec3(0.0f, 2.4f, 0.0f)
	};

	float conveyorSpeed = 0.002f; // Speed of scrolling

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

	std::vector<glm::vec3> objectsPositions;
	objectsPositions.push_back(generateRandomPosition());

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
	data = stbi_load("cb4.jpg", &width, &height, &nrChannels, 0);
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

	//float activationTime[] = { 0.0f, 2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f, 14.0f, 16.0f, 18.0f, 20.0f, 22.0f, 24.0f, 26.0f, 28.0f, 30.0f, 32.0f, 34.0f, 36.0f, 38.0f, 40.0f }; // Base activation time

	int numberOfCollisions = 0;
	int numberOfObject = 1;

	float pastTime = 0.0f;
	float delay = 2.0f;
	float cubeSpeed = 0.007f;
	float increaseDifficulty = 5.0f;
	float pastDifficulty = 0.0f;
	int level = 1;

	std::string collisionMessage = "Object collected: " + std::to_string(numberOfCollisions);
	std::string objectMessage = "Object dropped: " + std::to_string(numberOfObject);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Handle continuous cube appearance
		float currentTime = static_cast<float>(glfwGetTime());

		if (currentTime >= pastDifficulty + increaseDifficulty) {
			if (level > 1) cubeSpeed += 0.003f / level;
			delay -= 0.5f / level;
			pastDifficulty = currentTime;
			level++;
		}

		if (currentTime >= pastTime + delay) {
			// keep adding cubes
			objectsPositions.push_back(generateRandomPosition());
			std::cout << "Spawned at " << currentTime << " with speed " << cubeSpeed << " with delay " << delay << std::endl;
			numberOfObject++;
			// update pastTime for delay
			pastTime = currentTime;

			objectMessage = "Object dropped: " + std::to_string(numberOfObject);
		}

		// RENDER CUBES
		ourShader.use();
		for (unsigned int i = 0; i < objectsPositions.size(); i++) {

			if (objectsPositions[i].y <= -1.10f) {
				// Reset or remove objects off-screen
				continue;
			}

			// Update position
			objectsPositions[i].y -= cubeSpeed;

			// Create AABB for the current object after position update
			AABB objectAABB = createAABB(objectsPositions[i]);

			// Create AABB for the plate (static position)
			AABB plateAABB = createAABB(platePosition);

			// Check for collision
			if (checkCollision(objectAABB, plateAABB)) {
				numberOfCollisions++;
				// Optional: handle collision, e.g., remove object or reset position
				objectsPositions[i].y = -10.0f; // Move off-screen after collision

				collisionMessage = "Object collected: " + std::to_string(numberOfCollisions);
				soundEngine->play2D("C:/Users/aagar/Documents/InfoGrafica/OpenGLApp  - demos/OpenGLApp/OpenGLApp/pickup_sound.wav", false);
			}


			// Render cube
			glm::mat4 objModel = glm::mat4(1.0f);
			objModel = glm::translate(objModel, objectsPositions[i]);
			ourShader.setMat4("model", objModel);
			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

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
		/*glm::mat4 model = glm::translate(glm::mat4(1.0f), conveyorBeltPosition);
		ourShader.setMat4("model", model);
		ourShader.setInt("textureID", 3);
		glDrawArrays(GL_TRIANGLES, 0, 6);*/
		glm::mat4 model = glm::mat4(1.0f);
		for (int i = 0; i < 2; i++) {
			conveyorBeltPositions[i].y -= conveyorSpeed;
			if (conveyorBeltPositions[i].y <= -2.4f) {
				// Reset position when off-screen
				conveyorBeltPositions[i].y = 2.4f;
			}

			// Render the conveyor belt
			glm::mat4 conveyorModel = glm::mat4(1.0f);
			conveyorModel = glm::translate(conveyorModel, conveyorBeltPositions[i]);
			ourShader.setMat4("model", conveyorModel);
			ourShader.setInt("textureID", 3); // Set the conveyor belt texture
			glBindVertexArray(conveyorVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

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

		// Render text
		glUseProgram(shader.ID); // Use text shader
		renderText(shader, objectMessage, 10.0f, 550.0f, 0.6f, glm::vec3(1.0f, 1.0f, 1.0f));
		renderText(shader, collisionMessage, 10.0f, 480.0f, 0.6f, glm::vec3(1.0f, 1.0f, 1.0f));

		// Restore OpenGL state for 3D rendering
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_BLEND);

		// Swap buffers and poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	std::cout << "Oggetti: " << numberOfObject << std::endl;
	std::cout << "Collisioni: " << numberOfCollisions << std::endl;

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &conveyorVAO);
	glDeleteBuffers(1, &conveyorVBO);

	glDeleteVertexArrays(1, &plateVAO);
	glDeleteBuffers(1, &plateVBO);

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	soundEngine->drop();

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

glm::vec3 generateRandomPosition() {
	static std::random_device rd; // Seed
	static std::mt19937 gen(rd()); // Random number generator
	static std::uniform_int_distribution<int> distX(0, 2); // Distribution for x

	// Fixed x positions
	float xPositions[3] = { -0.50f, 0.0f, 0.50f };

	int randomIndex = distX(gen); // Pick a random index
	float randomX = xPositions[randomIndex];

	/*float randomX = distX(gen);*/
	return glm::vec3(randomX, 1.20f, 0.0f); // Fixed y and z
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		if (platePosition.x <= 0.45f) // if plate in border
			platePosition.x += 0.03f;  // Move plate right
		else {
			platePosition.x = 0.45f;
			// soundEngine->play2D("C:/Users/aagar/Documents/InfoGrafica/OpenGLApp  - demos/OpenGLApp/OpenGLApp/hitting_wall.wav", false);
		}
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		if (platePosition.x >= -0.45f) // if plate in border
			platePosition.x -= 0.03f;  // Move plate left
		else {
			platePosition.x = -0.45f;
			// soundEngine->play2D("C:/Users/aagar/Documents/InfoGrafica/OpenGLApp  - demos/OpenGLApp/OpenGLApp/hitting_wall.wav", false);
		}
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

// Function to create an AABB from position
AABB createAABB(const glm::vec3& position) {
	// Assuming the object has a fixed size of 0.2f x 0.2f 
	float halfWidth = 0.1f;  // Half of the object's width
	float halfHeight = 0.1f; // Half of the object's height

	return AABB{
		glm::vec3(position.x - halfWidth, position.y - halfHeight, position.z - 0.01f),  // min
		glm::vec3(position.x + halfWidth, position.y + halfHeight, position.z + 0.01f)   // max
	};
}

// Function to check for collision between two AABBs
bool checkCollision(const AABB& a, const AABB& b) {
	return (a.max.x >= b.min.x && a.min.x <= b.max.x) &&
		(a.max.y >= b.min.y && a.min.y <= b.max.y) &&
		(a.max.z >= b.min.z && a.min.z <= b.max.z);
}

void renderText(Shader& s, std::string text, float x, float y, float scale, glm::vec3 color) {
	// activate corresponding render state	
	s.use();
	glUniform3f(glGetUniformLocation(s.ID, "textColor"), color.x, color.y, color.z);

	// Enable blending to handle glyph transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(txtVAO);

	// iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = Characters[*c];

		float xpos = x + ch.Bearing.x * scale;
		float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;
		// update VBO for each character
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};
		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, txtVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
