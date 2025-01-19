#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <random>
#include <vector>
#include <string>
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

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

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

struct Food {
	glm::vec3 position;
	int type;
};

std::vector<Food> foods;

std::map<char, Character> Characters;
unsigned int txtVAO, txtVBO;

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

// lighting
glm::vec3 lightPos(0.2f, 0.10f, 0.01f);

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

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
glm::vec3 generateRandomPosition();
AABB createAABB(const glm::vec3& position);
bool checkCollision(const AABB& a, const AABB& b);
void renderText(Shader& s, std::string text, float x, float y, float scale, glm::vec3 color);
unsigned int TextureFromFile(const char* path, const std::string& directory);
int generateRandomObject();

// Struct for Vertex
struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

// Struct for Texture
struct Texture {
	unsigned int id;
	std::string type;
	std::string path;
};

// Mesh class
class Mesh {
public:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	unsigned int VAO;

	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) {
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;
		setupMesh();
	}

	void Draw(Shader& shader) {
		unsigned int diffuseNr = 1;
		unsigned int specularNr = 1;
		for (unsigned int i = 0; i < textures.size(); i++) {
			glActiveTexture(GL_TEXTURE0 + i);
			std::string number;
			std::string name = textures[i].type;
			if (name == "texture_diffuse")
				number = std::to_string(diffuseNr++);
			else if (name == "texture_specular")
				number = std::to_string(specularNr++);
			shader.setInt(("material." + name + number).c_str(), i);
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}
		glActiveTexture(GL_TEXTURE0);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

private:
	unsigned int VBO, EBO;

	void setupMesh() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

		glBindVertexArray(0);
	}
};

// Model class
class Model {
public:
	Model(const std::string& path) { loadModel(path); }

	void Draw(Shader& shader) {
		for (unsigned int i = 0; i < meshes.size(); i++)
			meshes[i].Draw(shader);
	}

private:
	std::vector<Mesh> meshes;
	std::string directory;

	void loadModel(const std::string& path) {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
			return;
		}
		directory = path.substr(0, path.find_last_of('/'));
		processNode(scene->mRootNode, scene);
	}

	void processNode(aiNode* node, const aiScene* scene) {
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}
		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			processNode(node->mChildren[i], scene);
		}
	}

	Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			Vertex vertex;
			glm::vec3 vector;
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;

			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.Normal = vector;

			if (mesh->mTextureCoords[0]) {
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}
			else
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);

			vertices.push_back(vertex);
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		if (mesh->mMaterialIndex >= 0) {
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		}

		return Mesh(vertices, indices, textures);
	}

	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName) {
		std::vector<Texture> textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
			aiString str;
			mat->GetTexture(type, i, &str);
			Texture texture;
			texture.id = TextureFromFile(str.C_Str(), directory);
			texture.type = typeName;
			texture.path = str.C_Str();
			textures.push_back(texture);
		}
		return textures;
	}
};

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

	Model croissantModel("C:/Users/franc/OneDrive/Documents/Coding/OpenGL/del3/del3/OpenGLApp/croissant.obj");
	Model plateModel("C:/Users/franc/OneDrive/Documents/Coding/OpenGL/del3/del3/OpenGLApp/sgorbio.obj");
	Model otherModel("C:/Users/franc/OneDrive/Documents/Coding/OpenGL/del3/del3/OpenGLApp/togocup.obj");
	Model muffinModel("C:/Users/franc/OneDrive/Documents/Coding/OpenGL/del3/del3/OpenGLApp/gus2.obj"); //con muffin.obj crasha

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

	std::vector<glm::vec3> objectsPositions; //OBSOLETE
	objectsPositions.push_back(generateRandomPosition());

	Food food;
	food.position = generateRandomPosition();
	food.type = generateRandomObject();
	foods.push_back(food);

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

	//----------- BEGIN lightning stuff
	Shader lightingShader("shader_light.vs", "shader_light.fs");

	float lightVertices[] = {
	-0.5f, -0.5f, -0.5f,  // Front-bottom-left
	 0.5f, -0.5f, -0.5f,  // Front-bottom-right
	 0.5f,  0.5f, -0.5f,  // Front-top-right
	 0.5f,  0.5f, -0.5f,  // Front-top-right
	-0.5f,  0.5f, -0.5f,  // Front-top-left
	-0.5f, -0.5f, -0.5f,  // Front-bottom-left

	-0.5f, -0.5f,  0.5f,  // Back-bottom-left
	 0.5f, -0.5f,  0.5f,  // Back-bottom-right
	 0.5f,  0.5f,  0.5f,  // Back-top-right
	 0.5f,  0.5f,  0.5f,  // Back-top-right
	-0.5f,  0.5f,  0.5f,  // Back-top-left
	-0.5f, -0.5f,  0.5f,  // Back-bottom-left

	-0.5f,  0.5f,  0.5f,  // Top-back-left
	-0.5f,  0.5f, -0.5f,  // Top-front-left
	 0.5f,  0.5f, -0.5f,  // Top-front-right
	 0.5f,  0.5f, -0.5f,  // Top-front-right
	 0.5f,  0.5f,  0.5f,  // Top-back-right
	-0.5f,  0.5f,  0.5f,  // Top-back-left

	-0.5f, -0.5f, -0.5f,  // Bottom-front-left
	-0.5f, -0.5f,  0.5f,  // Bottom-back-left
	 0.5f, -0.5f,  0.5f,  // Bottom-back-right
	 0.5f, -0.5f,  0.5f,  // Bottom-back-right
	 0.5f, -0.5f, -0.5f,  // Bottom-front-right
	-0.5f, -0.5f, -0.5f   // Bottom-front-left
	};

	unsigned int lightVAO, lightVBO;

	// Create the VAO and VBO
	glGenVertexArrays(1, &lightVAO);
	glGenBuffers(1, &lightVBO);

	// Bind the VAO
	glBindVertexArray(lightVAO);

	// Bind the VBO and load data
	glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lightVertices), lightVertices, GL_STATIC_DRAW);

	// Define the vertex position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Unbind the VAO and VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Set light properties
	lightingShader.use();
	lightingShader.setVec3("light.position", lightPos);
	lightingShader.setVec3("light.color", glm::vec3(1.0f, 0.0f, 0.0f)); // Red light
	lightingShader.setVec3("viewPos", camera.Position);

	// Model transformation matrix
	glm::mat4 lightModel = glm::mat4(1.0f);
	lightingShader.setMat4("model", lightModel);
	lightingShader.setMat4("view", camera.GetViewMatrix());
	lightingShader.setMat4("projection", projection);
	//----------- END lightning stuff

	//float activationTime[] = { 0.0f, 2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f, 14.0f, 16.0f, 18.0f, 20.0f, 22.0f, 24.0f, 26.0f, 28.0f, 30.0f, 32.0f, 34.0f, 36.0f, 38.0f, 40.0f }; // Base activation time

	int numberOfCollisions = 0;
	int numberOfObject = 1;

	float pastTime = 0.0f;
	float delay = 2.0f;
	float cubeSpeed = 0.007f;
	float increaseDifficulty = 6.0f;
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

		/*glBindVertexArray(lightCubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);*/

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
			Food food;
			food.position = generateRandomPosition();
			food.type = generateRandomObject();	
			foods.push_back(food);

			//objectsPositions.push_back(generateRandomPosition());
			std::cout << "Spawned at " << currentTime << " with speed " << cubeSpeed << " with delay " << delay << std::endl;
			numberOfObject++;
			// update pastTime for delay
			pastTime = currentTime;

			objectMessage = "Object dropped: " + std::to_string(numberOfObject);
		}

		// RENDER CUBES
		ourShader.use();
		for (unsigned int i = 0; i < foods.size(); i++) {

			if (foods[i].position.y <= -1.10f) {
				// Reset or remove objects off-screen
				continue;
			}

			// Update position
			foods[i].position.y -= cubeSpeed;

			// Create AABB for the current object after position update
			AABB objectAABB = createAABB(foods[i].position);

			// Create AABB for the plate (static position)
			AABB plateAABB = createAABB(platePosition);

			// Check for collision
			if (checkCollision(objectAABB, plateAABB)) {
				numberOfCollisions++;
				// Optional: handle collision, e.g., remove object or reset position
				foods[i].position.y = -10.0f; // Move off-screen after collision

				collisionMessage = "Object collected: " + std::to_string(numberOfCollisions);
				soundEngine->play2D("C:/Users/franc/OneDrive/Documents/Coding/OpenGL/del3/del3/OpenGLApp/pickup_sound.wav", false);
			}

			// Render cube
			glm::mat4 objModel = glm::mat4(1.0f);
			if (foods[i].type == 0) {
				float angle = glfwGetTime(); // Use the current time as the angle in radians
				objModel = glm::translate(objModel, foods[i].position);
				objModel = glm::scale(objModel, glm::vec3(0.3f, 0.3f, 0.3f));
				objModel = glm::rotate(objModel, angle, glm::vec3(0.0f, 1.0f, 0.0f));

				//ourShader.setInt("textureID", 1);	// still show the croissant white
				ourShader.setMat4("model", objModel);
				croissantModel.Draw(ourShader);
			} else if (foods[i].type == 1) {
				float angle = glfwGetTime(); // Use the current time as the angle in radians
				objModel = glm::translate(objModel, foods[i].position);
				objModel = glm::scale(objModel, glm::vec3(0.1f, 0.1f, 0.1f));
				objModel = glm::rotate(objModel, angle, glm::vec3(0.0f, 1.0f, 0.0f));

				//ourShader.setInt("textureID", 1);	// still show the croissant white
				ourShader.setMat4("model", objModel);
				otherModel.Draw(ourShader);
			}
			else if (foods[i].type == 2) {
				float angle = glfwGetTime(); // Use the current time as the angle in radians
				objModel = glm::translate(objModel, foods[i].position);
				objModel = glm::scale(objModel, glm::vec3(0.1f, 0.1f, 0.1f));
				objModel = glm::rotate(objModel, angle, glm::vec3(0.0f, 1.0f, 0.0f));

				//ourShader.setInt("textureID", 1);	// still show the croissant white
				ourShader.setMat4("model", objModel);
				muffinModel.Draw(ourShader);
			}
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

		// be sure to activate shader when setting uniforms/drawing objects
		//lightingShader.use();
		ourShader.setVec3("light.position", lightPos);
		ourShader.setVec3("viewPos", camera.Position);

		// light properties
		glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
		
		//pulsing light (?)
		/*lightColor.x = static_cast<float>(sin(glfwGetTime() * 2.0));
		lightColor.y = static_cast<float>(sin(glfwGetTime() * 0.7));
		lightColor.z = static_cast<float>(sin(glfwGetTime() * 1.3));*/

		glm::vec3 diffuseColor = lightColor * glm::vec3(0.8f); // decrease the influence
		glm::vec3 ambientColor = diffuseColor * glm::vec3(0.5f); // low influence
		ourShader.setVec3("light.ambient", ambientColor);
		ourShader.setVec3("light.diffuse", diffuseColor);
		ourShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

		// material properties
		ourShader.setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
		ourShader.setVec3("material.diffuse", 1.0f, 0.5f, 0.31f);
		ourShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f); // specular lighting doesn't have full effect on this object's material
		ourShader.setFloat("material.shininess", 32.0f);

		// Render plate
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glBindVertexArray(plateVAO);
		model = glm::translate(glm::mat4(1.0f), platePosition);
		model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));
		ourShader.setMat4("model", model);
		ourShader.setInt("textureID", 1);
		plateModel.Draw(ourShader);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Render cube
		/*
		glm::mat4 objModel = glm::mat4(1.0f);
		ourShader.setMat4("model", objModel);
		*/

		// render cubes
		/*
		glBindVertexArray(plateVAO);
		glBindVertexArray(conveyorVAO);
		glBindVertexArray(VAO);
		*/

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

// Helper function for loading textures
unsigned int TextureFromFile(const char* path, const std::string& directory) {
	std::string filename = directory + "/" + std::string(path);
	std::cout << "Loading texture: " << filename << std::endl;
	unsigned int textureID;
	glGenTextures(1, &textureID);
	int width, height, nrComponents;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data) {
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else {
		std::cerr << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}
	return textureID;
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

int generateRandomObject() {
	static std::random_device rd; // Seed
	static std::mt19937 gen(rd()); // Random number generator
	static std::uniform_int_distribution<int> dist(0, 2); // Distribution for x

	return dist(gen); 
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