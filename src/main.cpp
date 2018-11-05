//Fabian Frontczak 210179 FTIMS

// dear imgui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <vector>

// About OpenGL function loaders: modern OpenGL doesn't have a standard header file and requires individual function pointers to be loaded manually.
// Helper libraries are often used for this purpose! Here we are supporting a few common ones: gl3w, glew, glad.
// You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>    // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>    // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>  // Initialize with gladLoadGL()
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

#include <stb_image.h>
#include <GLFW/glfw3.h> // Include glfw3.h after our OpenGL definitions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>


GLFWwindow* initializeWindow();
int buildVertexShader();
int buildFragmentShader();
int linkShaders(int vertexShader, int fragmentShader);
void fillVertexBuffer();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void calculateBox(float x, float y, float z, float width);
void menger(float xpos, float ypos, float zpos, float width, int depth);

// settings
const unsigned int SCR_WIDTH = 900;
const unsigned int SCR_HEIGHT = 900;
std::vector<float> vertices = {};
static int max_depth = 3;
ImVec4 clear_color = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
float radiusX = 0;
float radiusY = 0;

const char *vertexShaderSource ="#version 330 core\n"
                                "layout (location = 0) in vec3 aPos;\n"
                                "layout (location = 1) in vec3 aColor;\n"
                                "layout (location = 2) in vec2 aTexCoord;\n"
                                "out vec3 ourColor;\n"
                                "out vec2 TexCoord;\n"
                                "uniform mat4 model;\n"
                                "uniform mat4 view;\n"
                                "uniform mat4 projection;\n"
                                "void main()\n"
                                "{\n"
                                "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
                                "   ourColor = aColor;\n"
                                "   TexCoord = aTexCoord;\n"
                                "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
                                   "in vec3 ourColor;\n"
                                   "in vec2 TexCoord;\n"
                                   "out vec4 FragColor;\n"
                                   "uniform sampler2D ourTexture;\n"
                                   "void main()\n"
                                   "{\n"
                                   "   FragColor = texture(ourTexture, TexCoord) * vec4(ourColor, 1.0f);\n"
                                   "}\n\0";

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main()
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // Create window with graphics context
    // --------------------
    GLFWwindow* window = initializeWindow();
    if (window == NULL)
    {
        return -1;
    }

    // Initialize OpenGL loader
    #if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
        bool err = gl3wInit() != 0;
    #elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
        bool err = glewInit() != GLEW_OK;
    #elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
        bool err = !gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    #endif
        if (err)
        {
            fprintf(stderr, "Failed to initialize OpenGL loader!\n");
            return -1;
        }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    int vertexShader = buildVertexShader();
    // fragment shader
    int fragmentShader = buildFragmentShader();
    // link shaders
    int shaderProgram = linkShaders(vertexShader, fragmentShader);
    //cleaning up shader's objects
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    //calculateBox(-0.5f, -0.5f, 0.0f, 0.4f);
    //sierpinskiCarpet(-1.0f, -1.0f, 0.0f, 2.0f, 0);
    //menger(-1,-1,0, 2, 0, max_depth);
    menger(-0.8f,-0.8f, 0, 1.8f, max_depth);

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    fillVertexBuffer();


    // load and create a texture
    // -------------------------
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    unsigned char *data = stbi_load("res/textures/stone.jpg", &width, &height, &nrChannels, 0);
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

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    // glBindVertexArray(0);

    // as we only have a single shader, we could also just activate our shader once beforehand if we want to
    glUseProgram(shaderProgram);


    // pass projection matrix to shader (as projection matrix rarely changes there's no need to do this per frame)
    // -----------------------------------------------------------------------------------------------------------
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    // Setup style
    ImGui::StyleColorsDark();


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            bool isImGuiInit = true;
            static int localDepthLevel = max_depth;
            static bool autoRotate = false;
            static bool autoRotateX = false;
            static bool autoRotateY = false;
            static int localRadiusX = (int)radiusX;
            static int localRadiusY = (int)radiusY;

            ImGui::Begin("Glebokosc rekurencji / kolor", &isImGuiInit, ImGuiWindowFlags_NoTitleBar);           // Create a window called "sth" and append into it.
            ImGui::SliderInt("Glebokosc", &localDepthLevel, 1, 5);            // Edit 1 int using a slider from 0 to 7
            ImGui::ColorEdit3("Kolor", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Zatwierdz poziom i kolor"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            {
                vertices.clear();
                max_depth = localDepthLevel;
                menger(-0.8f,-0.8f, 0, 1.8f, max_depth);
                radiusX = (float)localRadiusX;
                radiusY = (float)localRadiusY;
                fillVertexBuffer();
            }
            ImGui::NewLine();

            ImGui::Checkbox("Auto obrót", &autoRotate);
            if (autoRotate)
            {
                radiusX = glm::radians(0.0f);
                radiusY = glm::radians(0.0f);

                ImGui::SameLine();
                ImGui::Checkbox("X", &autoRotateX);
                ImGui::SameLine();
                ImGui::Checkbox("Y", &autoRotateY);

                if (autoRotate && autoRotateX)
                {
                    radiusX = (float)glfwGetTime();
                }

                if (autoRotate && autoRotateY)
                {
                    radiusY = (float)glfwGetTime();
                }
            }
            else
            {
                ImGui::SliderInt("Obrot X", &localRadiusX, 0, 360);
                ImGui::SliderInt("Obrot Y", &localRadiusY, 0, 360);

                radiusX = glm::radians((float)localRadiusX);
                radiusY = glm::radians((float)localRadiusY);
            }

            ImGui::End();
        }


        // Rendering
        ImGui::Render();
        glfwMakeContextCurrent(window);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

        // bind Texture
        glBindTexture(GL_TEXTURE_2D, texture);


        // create transformations
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        //model = glm::rotate(model, glm::radians(30.0f), glm::vec3(0.2f, 0.3f, 0.0f));
        model = glm::rotate(model, radiusX, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, radiusY, glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::lookAt(glm::vec3(0.0f, 0.0f, -6.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        //model = glm::rotate(model, glm::radians((float)radiusX), glm::vec3(1.0f, 0.0f, 0.0f));
        //model = glm::rotate(model, glm::radians((float)radiusY), glm::vec3(0.0f, 1.0f, 0.0f));

        // pass transformation matrices to the shader
        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // render the triangle
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3*vertices.size()/4);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        //glfwSwapBuffers(window);
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    /*ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();*/

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

// glfw window creation
GLFWwindow* initializeWindow()
{
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Dywan Sierpińskiego", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    return window;
}

// building vertex shader
int buildVertexShader()
{
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    return vertexShader;
}

// building fragment shader
int buildFragmentShader()
{
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    return fragmentShader;
}

int linkShaders(int vertexShader, int fragmentShader)
{
    // link shaders
    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // check for linking errors
    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    return shaderProgram;
}

// fill Vertex Buffer
void fillVertexBuffer()
{
    // fill with vertex data
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void calculateBox(float x, float y, float z, float width)
{
//-----------------------------------------------
    // FRONT
    // left triangle
    vertices.push_back(x);
    vertices.push_back(y);
    vertices.push_back(z);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);

    vertices.push_back(x + width);
    vertices.push_back(y);
    vertices.push_back(z);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(0.5f);
    vertices.push_back(0.0f);

    vertices.push_back(x);
    vertices.push_back(y + width);
    vertices.push_back(z);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(0.0f);
    vertices.push_back(1.0f);

//-----------------------------------------------
    // FRONT
    // right triangle
    vertices.push_back(x);
    vertices.push_back(y + width);
    vertices.push_back(z);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(0.5f);
    vertices.push_back(1.0f);

    vertices.push_back(x + width);
    vertices.push_back(y + width);
    vertices.push_back(z);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(1.0f);
    vertices.push_back(1.0f);

    vertices.push_back(x + width);
    vertices.push_back(y);
    vertices.push_back(z);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(1.0f);
    vertices.push_back(0.0f);

//-----------------------------------------------
    // BACK
    // left triangle
    vertices.push_back(x);
    vertices.push_back(y);
    vertices.push_back(z + width);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);

    vertices.push_back(x + width);
    vertices.push_back(y);
    vertices.push_back(z + width);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(0.5f);
    vertices.push_back(0.0f);

    vertices.push_back(x);
    vertices.push_back(y + width);
    vertices.push_back(z + width);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(0.0f);
    vertices.push_back(1.0f);

//-----------------------------------------------
    // BACK
    // right triangle
    vertices.push_back(x);
    vertices.push_back(y + width);
    vertices.push_back(z + width);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(0.5f);
    vertices.push_back(1.0f);

    vertices.push_back(x + width);
    vertices.push_back(y + width);
    vertices.push_back(z + width);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(1.0f);
    vertices.push_back(1.0f);

    vertices.push_back(x + width);
    vertices.push_back(y);
    vertices.push_back(z + width);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(1.0f);
    vertices.push_back(0.0f);

//-----------------------------------------------
    // LEFT
    // left triangle
    vertices.push_back(x);
    vertices.push_back(y);
    vertices.push_back(z + width);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);

    vertices.push_back(x);
    vertices.push_back(y);
    vertices.push_back(z);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(0.5f);
    vertices.push_back(0.0f);

    vertices.push_back(x);
    vertices.push_back(y + width);
    vertices.push_back(z + width);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(0.0f);
    vertices.push_back(1.0f);

//-----------------------------------------------
    // LEFT
    // right triangle
    vertices.push_back(x);
    vertices.push_back(y + width);
    vertices.push_back(z + width);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(0.5f);
    vertices.push_back(1.0f);

    vertices.push_back(x);
    vertices.push_back(y + width);
    vertices.push_back(z);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(1.0f);
    vertices.push_back(1.0f);

    vertices.push_back(x);
    vertices.push_back(y);
    vertices.push_back(z);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(1.0f);
    vertices.push_back(0.0f);

//-----------------------------------------------
    // RIGHT
    // left triangle
    vertices.push_back(x + width);
    vertices.push_back(y);
    vertices.push_back(z + width);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);

    vertices.push_back(x + width);
    vertices.push_back(y);
    vertices.push_back(z);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(0.5f);
    vertices.push_back(0.0f);

    vertices.push_back(x + width);
    vertices.push_back(y + width);
    vertices.push_back(z + width);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(0.0f);
    vertices.push_back(1.0f);

//-----------------------------------------------
    // RIGHT
    // right triangle
    vertices.push_back(x + width);
    vertices.push_back(y + width);
    vertices.push_back(z + width);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(0.5f);
    vertices.push_back(1.0f);

    vertices.push_back(x + width);
    vertices.push_back(y + width);
    vertices.push_back(z);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(1.0f);
    vertices.push_back(1.0f);

    vertices.push_back(x + width);
    vertices.push_back(y);
    vertices.push_back(z);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(1.0f);
    vertices.push_back(0.0f);

//-----------------------------------------------
    // BOTTOM
    // left triangle
    vertices.push_back(x);
    vertices.push_back(y);
    vertices.push_back(z);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);

    vertices.push_back(x + width);
    vertices.push_back(y);
    vertices.push_back(z);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(0.5f);
    vertices.push_back(0.0f);

    vertices.push_back(x);
    vertices.push_back(y);
    vertices.push_back(z + width);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(0.0f);
    vertices.push_back(1.0f);

    //-----------------------------------------------
    // BOTTOM
    // right triangle
    vertices.push_back(x);
    vertices.push_back(y);
    vertices.push_back(z + width);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(0.5f);
    vertices.push_back(1.0f);

    vertices.push_back(x + width);
    vertices.push_back(y);
    vertices.push_back(z + width);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(1.0f);
    vertices.push_back(1.0f);

    vertices.push_back(x + width);
    vertices.push_back(y);
    vertices.push_back(z);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(1.0f);
    vertices.push_back(0.0f);

    //-----------------------------------------------
    // TOP
    // left triangle
    vertices.push_back(x);
    vertices.push_back(y + width);
    vertices.push_back(z);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);

    vertices.push_back(x + width);
    vertices.push_back(y + width);
    vertices.push_back(z);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(0.5f);
    vertices.push_back(0.0f);

    vertices.push_back(x);
    vertices.push_back(y + width);
    vertices.push_back(z + width);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(0.0f);
    vertices.push_back(1.0f);

    //-----------------------------------------------
    // TOP
    // right triangle
    vertices.push_back(x);
    vertices.push_back(y + width);
    vertices.push_back(z + width);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(0.5f);
    vertices.push_back(1.0f);

    vertices.push_back(x + width);
    vertices.push_back(y + width);
    vertices.push_back(z + width);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(1.0f);
    vertices.push_back(1.0f);

    vertices.push_back(x + width);
    vertices.push_back(y + width);
    vertices.push_back(z);
    vertices.push_back(clear_color.x);
    vertices.push_back(clear_color.y);
    vertices.push_back(clear_color.z);
    vertices.push_back(1.0f);
    vertices.push_back(0.0f);
}

void menger(float xpos, float ypos, float zpos, float width, int depth)
{
    // See if this is depth 1.
    if (depth == 1)
    {
        // Just make a cube.
        calculateBox(xpos, ypos, zpos, width);
    }
    else
    {
        // Divide the cube.
        double newWidth = width / 3.0;

        for (int ix = 0; ix < 3; ix++)
        {
            for (int iy = 0; iy < 3; iy++)
            {
                if ((ix == 1) && (iy == 1)) continue;
                for (int iz = 0; iz < 3; iz++)
                {
                    if ((iz == 1) && ((ix == 1) || (iy == 1))) continue;
                    menger(xpos + newWidth * ix, ypos + newWidth * iy, zpos + newWidth * iz, newWidth, depth - 1);
                }
            }
        }
    }
}

