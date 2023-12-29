
#include "imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"


#include <string.h>
#include <stdio.h>
#include <cmath>


#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>



//window dimensions
const GLint WIDTH = 800, HEIGHT = 600;
const float toRadians = 3.14159265f / 180.0f;

GLuint VAO, VBO, IBO, shader, uniformModel, uniformProjection;

bool direction = true;
float triOffset = 0.0f;
float triMaxOffset = 0.7f;
float triIncrement = 0.0005f;

float curAngle = 0.0f;
float curAngleIncr = 0.01f;
bool autoRotate = false;

float sizeX = 1.0f;
float sizeY = 1.0f;

// Vertex Shader

static const char* vShader =
"#version 330														\n\
																	\n\
	layout (location = 0) in vec3 pos;								\n\
									\n\
	out vec4 vCol;							\n\
								\n\
								\n\
								\n\
								\n\
																	\n\
																	\n\
	uniform mat4 model;												\n\
	uniform mat4 projection;										\n\
																	\n\
	void main()														\n\
	{																\n\
		gl_Position = projection * model * vec4(pos, 1.0);			\n\
		vCol = vec4(clamp(pos,0.0f,1.0f), 1.0f);					\n\
																	\n\
																		\n\
	}																\n\
																	\n\
";
 
 
/*	""
	"layout (location = 0) in vec3 pos;"
	""
	"void main()"
	"{"
	"	gl_Position = vec4( 0.4 * pos.x,0.4 * pos.y, pos.z, 1.0);"
	"}"
	"";*/

// Fragment shader
static const char* fShader =
"#version 330								\n\
											\n\
out vec4 colour;							\n\
											\n\
in vec4 vCol;							\n\
							\n\
							\n\
void main()									\n\
{											\n\
	colour = vCol;		\n\
}											\n\
											\n\
											\n\
";

/*
"#version 330"
""
"out vec4 colour;"
""
"void main()"
"{"
"	colour = vec4(1.0, 0.0, 0.0, 1.0);"
"}"
"";*/

void CreateTriangle()
{
	unsigned int indices[]
	{
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	GLfloat vertecies[] = {
		-1.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 1.0f,
		1.0, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};



	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertecies), vertecies, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void AddShader(GLuint theProgram, const char* shaderCode, GLenum shaderType)
{
	GLuint theShader = glCreateShader(shaderType);

	const GLchar* theCode[1];
	theCode[0] = shaderCode;

	GLint codeLenght[1];
	codeLenght[0] = strlen(shaderCode);

	glShaderSource(theShader, 1, theCode, codeLenght);
	glCompileShader(theShader);

	GLint result = 0;
	GLchar eLog[1024] = { 0 };

	glGetShaderiv(theShader, GL_COMPILE_STATUS, &result);

	if (!result)
	{
		glGetShaderInfoLog(theShader, sizeof(eLog), NULL, eLog);
		printf("Error compiling the %d shader: '%s'\n", shaderType, eLog);
		return;
	}

	glAttachShader(theProgram, theShader);

}


void CompileShaders()
{
	shader = glCreateProgram();

	if(!shader)
	{
		printf("Error creating shader program!");
		return;
	}

	AddShader(shader, vShader, GL_VERTEX_SHADER);
	AddShader(shader, fShader, GL_FRAGMENT_SHADER);

	GLint result = 0;
	GLchar eLog[1024] = { 0 };

	glLinkProgram(shader);
	glGetProgramiv(shader, GL_LINK_STATUS, &result);

	if(!result)
	{
		glGetProgramInfoLog(shader, sizeof(eLog), NULL, eLog);
		printf("Error linking program: '%s'\n",eLog);
		return;
	}

	glValidateProgram(shader);
	glGetProgramiv(shader, GL_VALIDATE_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(shader, sizeof(eLog), NULL, eLog);
		printf("Error validating program: '%s'\n", eLog);
		return;
	}

	uniformModel = glGetUniformLocation(shader,"model");
	uniformProjection = glGetUniformLocation(shader, "projection");
}

int main()
{

	// Intiialise GLFW
	if(!glfwInit())
	{
		printf("GLFW Initialisation failed!");
		glfwTerminate();
		return 1;
	}



	// Setup GLFW window properties
	// OpenGL version

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Core profile = No backwards compatibility
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// Allow forward compatibility
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow* mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "Test Window", NULL, NULL);

	if(!mainWindow)
	{
		printf("GLFW window creation failed");
		glfwTerminate();
		return 1;
	}

	// Get Buffer size information
	int bufferWidth, bufferHeight;
	glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);

	// Set context for GLEW to use
	glfwMakeContextCurrent(mainWindow);



	// Allow modern extension features
	glewExperimental = GL_TRUE;

	if(glewInit() != GLEW_OK)
	{
		printf("GLEW intialisation failed!");
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
		return 1;
	}

	glEnable(GL_DEPTH_TEST);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(mainWindow, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();
	// Setup viewport size

	glViewport(0, 0, bufferWidth, bufferHeight);

	CreateTriangle();
	CompileShaders();

	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)bufferWidth / (GLfloat)bufferHeight, 0.1f, 100.0f);

	// Loop until window closed

	while(!glfwWindowShouldClose(mainWindow))
	{
		glfwPollEvents();

		//startimgui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		
		if(direction)
		{
			triOffset += triIncrement;
		} else
		{
			triOffset -= triIncrement;
		}

		if(abs(triOffset) >= triMaxOffset)
		{
			direction = !direction;
		}

		if(autoRotate)
		{
			curAngle += curAngleIncr;
		}
		

		if(curAngle >= 360)
		{
			curAngle -= 360;
		}

		// Clear window
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shader);

		glm::mat4 model(1.0f);
		model = glm::translate(model, glm::vec3(0, triOffset, -2.5f));
		model = glm::rotate(model, curAngle * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(sizeX, sizeY, 1.0f));



		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		ImGui::Begin("Options");

		ImGui::SliderFloat("Speed", &triIncrement, 0.00f, 0.1, "%.3f");

		ImGui::SliderFloat("Rotation speed", &curAngleIncr, 0.00f, 1.0f, "%.3f");
		ImGui::Checkbox("Auto rotation", &autoRotate);
		if (ImGui::Button("Reset Rotation", ImVec2(140, 30)))
		{
			curAngle = 0;
		}

		ImGui::SliderFloat("Size X", &sizeX, 0.01f, 2.0f, "%.3f");
		ImGui::SliderFloat("Size Y", &sizeY, 0.01f, 2.0f, "%.3f");
		if (ImGui::Button("Reset Size", ImVec2(140, 30)))
		{
			sizeX = 1.0f;
			sizeY = 1.0f;
		}

		


		ImGui::End();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


		glUseProgram(0);
		glfwSwapBuffers(mainWindow);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	return 0;
}
