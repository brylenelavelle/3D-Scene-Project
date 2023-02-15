#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>      // Image loading Utility functions

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

#include <learnOpengl/camera.h> // Camera class

static const float PI = 3.14159265358979323846f;
using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "Project"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // variable to handle ortho change
    bool perspective = false;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbo;         // Handle for the vertex buffer object
        GLuint nVertices;    // Number of indices of the mesh
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Triangle mesh data
    GLMesh gMesh;

    // Texture
    GLuint gTextureId1;
    GLuint gTextureId2;
    GLuint gTextureId3;
    GLuint gTextureId4;
    GLuint gTextureId5;
    GLuint gTextureId6;
    glm::vec2 gUVScale(1.0f, 1.0f);
    GLint gTexWrapMode = GL_REPEAT;

    // Shader program
    GLuint gProgramId;
    GLuint gLampProgramId;

    // camera
    Camera gCamera(glm::vec3(0.0f, -1.0f, 20.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

    // Subject position and scale
    glm::vec3 gCubePosition(1.0f, -2.0f, 0.0f);
    glm::vec3 gCubeScale(1.0f);

    // Cube and light color
    glm::vec3 gObjectColor(1.0f, 0.2f, 0.0f);
    glm::vec3 gLightColor(0.25f, 0.25f, 0.25f);

    // Light position and scale
    glm::vec3 gLightPosition(10.0f, -4.0f, 3.0f);
    glm::vec3 gLightScale(0.5f);

    // Light color, position and scale
    glm::vec3 gKeyLightColor(1.0f, 1.0f, 1.0f);
    glm::vec3 gKeyLightPosition(0.5f, 2.0f, -0.5f);
    glm::vec3 gKeyLightScale(0.2f);

    // Tube
    glm::vec3 gTubeColor(3.0f, -1.0f, 0.0f);
    glm::vec3 gTubePosition(5.0f, -5.3f, 0.0f);
    glm::vec3 gTubeScale(1.5f);

    // Octagon 
    glm::vec3 gOctColor(1.0f, 0.2f, 0.0f);
    glm::vec3 gOctPosition(0.0f, -6.1f, -1.0f);
    glm::vec3 gOctScale(0.8f);

    // Blue Top Cube1
    glm::vec3 gCube1Color(1.0f, 0.2f, 0.0f);
    glm::vec3 gCube1Position(-4.0f, -4.0f, -1.0f);
    glm::vec3 gCube1Scale(2.5f);

    // Blue Bottom Cube2
    glm::vec3 gCube2Color(1.0f, 0.2f, 0.0f);
    glm::vec3 gCube2Position(-4.0f, -5.5f, -1.0f);
    glm::vec3 gCube2Scale(2.9f);

    // Pink Sphere
    glm::vec3 gSphereColor(1.0f, 0.2f, 0.0f);
    glm::vec3 gSpherePosition(0.0f, -6.9f, 3.0f);
    glm::vec3 gSphereScale(1.3f);

    // Lamp animation
    bool gIsLampOrbiting = true;

}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UCreateMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
layout(location = 1) in vec3 normal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;


//Global variables for the transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates

    vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

    vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
    vertexTextureCoordinate = textureCoordinate;
}
);


/* Cube Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,

    in vec2 vertexTextureCoordinate;
in vec3 vertexNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position

out vec4 fragmentColor; // For outgoing cube color to the GPU

// Uniform / Global variables for object color, light color, light position, and camera/view position
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 keyLightColor;
uniform vec3 lightPos;
uniform vec3 keyLightPos;
uniform vec3 viewPosition;

uniform vec3 octColor;
uniform vec3 octPos;

uniform vec3 tubeColor;
uniform vec3 tubePos;

uniform vec3 cube1Color;
uniform vec3 cube1Pos;

uniform vec3 cube2Color;
uniform vec3 cube2Pos;

uniform vec3 sphereColor;
uniform vec3 sphere2Pos;

// Useful when working with multiple textures
uniform sampler2D uTexture; 
uniform sampler2D uTexture1;
uniform sampler2D uTexture2;
uniform sampler2D uTexture3;
uniform sampler2D uTexture4;
uniform sampler2D uTexture5;
uniform vec2 uvScale;
out vec4 textureColor;

void main()
{

    /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

    //Calculate Ambient lighting*/
    float ambientStrength = 0.05f; // Set ambient or global lighting strength
    vec3 ambient = ambientStrength * lightColor; // Generate ambient light color

    // Calculate Key Lighting
    float keyLightStrength = 0.8f;
    vec3 key = keyLightStrength * keyLightColor;

    // Octagon
    float octStrength = 0.8f;
    vec3 oct = octStrength * octColor;
    
    // Tube
    float tubeStrength = 1.0f;
    vec3 tube = tubeStrength * tubeColor;

    // Cube 1 and 2
    float cube1Strength = 0.0f;
    vec3 cube1 = cube1Strength * cube1Color;
    float cube2Strength = 0.0f;
    vec3 cube2 = cube2Strength * cube2Color;

    // Sphere
    float sphereStrength = 0.2f;
    vec3 sphere = sphereStrength * sphereColor;

    //Calculate Diffuse lighting*/
    vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
    vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
    float impact = max(dot(norm, lightDirection), 0.1);// Calculate diffuse impact by generating dot product of normal and light
    vec3 diffuse = impact * lightColor; // Generate diffuse light color
    vec3 keyLightDirection = normalize(keyLightPos - vertexFragmentPos);
    float keyLightImpact = max(dot(norm, keyLightDirection), 0.0);
    vec3 keyLightDiffuse = keyLightImpact * keyLightColor;

    //Calculate Specular lighting*/
    float specularIntensity = 3.0f; // Set specular light strength
    float highlightSize = 12.0f; // Set specular highlight size
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector

    //Calculate specular component
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
    vec3 specular = specularIntensity * specularComponent * lightColor;
    vec3 keyLightSpecular = specularIntensity * specularComponent * keyLightColor;

    // Octagon
    vec3 octSpecular = specularIntensity * specularComponent * octColor;

    // Tube
    vec3 tubeSpecular = specularIntensity * specularComponent * tubeColor;

    // Cube 1 and 2
    vec3 cube1Specular = specularIntensity * specularComponent * cube1Color;
    vec3 cube2Specular = specularIntensity * specularComponent * cube2Color;

    // Sphere
    vec3 sphereSpecular = specularIntensity * specularComponent * sphereColor;

    // Texture holds the color to be used for all three components
    vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);
    textureColor = texture(uTexture1, vertexTextureCoordinate * uvScale);
    textureColor = texture(uTexture2, vertexTextureCoordinate * uvScale);
    textureColor = texture(uTexture3, vertexTextureCoordinate * uvScale);
    textureColor = texture(uTexture4, vertexTextureCoordinate * uvScale);
    textureColor = texture(uTexture5, vertexTextureCoordinate * uvScale);

    // Calculate phong result
    vec3 phong = (ambient + key + diffuse + specular) * textureColor.xyz;

    fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU
}
);

/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

        //Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
}
);


/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU
uniform vec2 uvScale;
in vec2 vertexTextureCoordinate;
uniform sampler2D uTexture; // Useful when working with multiple textures
uniform sampler2D uTexture1;
uniform sampler2D uTexture2;
uniform sampler2D uTexture3;
uniform sampler2D uTexture4;
uniform sampler2D uTexture5;

void main()
{
    fragmentColor = texture(uTexture, vertexTextureCoordinate * uvScale);
    fragmentColor = texture(uTexture1, vertexTextureCoordinate * uvScale);
    fragmentColor = texture(uTexture2, vertexTextureCoordinate * uvScale);
    fragmentColor = texture(uTexture3, vertexTextureCoordinate * uvScale);
    fragmentColor = texture(uTexture4, vertexTextureCoordinate * uvScale);
    fragmentColor = texture(uTexture5, vertexTextureCoordinate * uvScale);

    fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
}
);


// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}


int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the mesh
    UCreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object

    // Create the shader program
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampProgramId))
        return EXIT_FAILURE;

    // Load texture
    const char* texFilename = "../../3D Scene Interactivity/image/tubebody1.png";
    if (!UCreateTexture(texFilename, gTextureId1))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "../../3D Scene Interactivity/image/cardboard2.jpg";
    if (!UCreateTexture(texFilename, gTextureId2))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "../../3D Scene Interactivity/image/tubecap1.png";
    if (!UCreateTexture(texFilename, gTextureId3))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "../../3D Scene Interactivity/image/octagon.png";
    if (!UCreateTexture(texFilename, gTextureId4))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "../../3D Scene Interactivity/image/bluebox.png";
    if (!UCreateTexture(texFilename, gTextureId5))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "../../3D Scene Interactivity/image/pink1.png";
    if (!UCreateTexture(texFilename, gTextureId6))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);
    // We set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture1"), 1);
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture2"), 2);
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture3"), 3);
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture4"), 4);
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture5"), 5);

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        // per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        // -----
        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(gMesh);

    // Release texture
    UDestroyTexture(gTextureId1);
    UDestroyTexture(gTextureId2);
    UDestroyTexture(gTextureId3);
    UDestroyTexture(gTextureId4);
    UDestroyTexture(gTextureId5);
    UDestroyTexture(gTextureId6);

    // Release shader program
    UDestroyShaderProgram(gProgramId);
    UDestroyShaderProgram(gLampProgramId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.ProcessKeyboard(UP, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        perspective = false;
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        perspective = true;

    // Pause and resume lamp orbiting
    static bool isLKeyDown = false;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !gIsLampOrbiting)
        gIsLampOrbiting = true;
    else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && gIsLampOrbiting)
        gIsLampOrbiting = false;
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}

// glfw: handle mouse button events
// --------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}


// Functioned called to render a frame
void URender()
{
    // Lamp orbits around the origin
    const float angularVelocity = glm::radians(45.0f);
    if (gIsLampOrbiting)
    {
        glm::vec4 newPosition = glm::rotate(angularVelocity * gDeltaTime, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(gLightPosition, 1.0f);
        gLightPosition.x = newPosition.x;
        gLightPosition.y = newPosition.y;
        gLightPosition.z = newPosition.z;
    }

    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(1.2, 0.5f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Activate the cube VAO (used by pyramid and lamp)
    glBindVertexArray(gMesh.vao);

    // Set the shader to be used
    glUseProgram(gProgramId);

    // Model matrix: transformations are applied right-to-left order
    glm::mat4 model = glm::translate(gCubePosition) * glm::scale(gCubeScale);

    // camera/view transformation
    glm::mat4 view = gCamera.GetViewMatrix();

    glm::mat4 projection;
    if (!perspective)
    {
        // Enables perspective view (default) by pressing "P" key
        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
    }
    else
        // Enables ortho view when pressing "O" key
        projection = glm::ortho(-12.0f, 15.0f, -7.0f, 1.0f, 0.1f, 100.0f);

    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Cube Shader program for the cub color, light color, light position, and camera position
    GLint objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    GLint lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    GLint lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    GLint viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");
    GLint keyLightColorLoc = glGetUniformLocation(gProgramId, "keyLightColor");
    GLint keyLightPositionLoc = glGetUniformLocation(gProgramId, "keyLightPos");

    // Octagon
    GLint octColorLoc = glGetUniformLocation(gProgramId, "octColor");
    GLint octPositionLoc = glGetUniformLocation(gProgramId, "octPos");

    // Tube
    GLint tubeColorLoc = glGetUniformLocation(gProgramId, "tubeColor");
    GLint tubePositionLoc = glGetUniformLocation(gProgramId, "tubePos");

    // Cube 1 and 2
    GLint cube1ColorLoc = glGetUniformLocation(gProgramId, "cube1Color");
    GLint cube1PositionLoc = glGetUniformLocation(gProgramId, "cube1Pos");
    GLint cube2ColorLoc = glGetUniformLocation(gProgramId, "cube2Color");
    GLint cube2PositionLoc = glGetUniformLocation(gProgramId, "cube2Pos");

    // Sphere
    GLint sphereColorLoc = glGetUniformLocation(gProgramId, "sphereColor");
    GLint spherePositionLoc = glGetUniformLocation(gProgramId, "spherePos");

    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    glUniform3f(keyLightColorLoc, gKeyLightColor.r, gKeyLightColor.g, gKeyLightColor.b);
    glUniform3f(keyLightPositionLoc, gKeyLightPosition.x, gKeyLightPosition.y, gKeyLightPosition.z);
    glUniform3f(octColorLoc, gOctColor.r, gOctColor.g, gOctColor.b);
    glUniform3f(octPositionLoc, gOctPosition.x, gOctPosition.y, gOctPosition.z);
    glUniform3f(tubeColorLoc, gTubeColor.r, gTubeColor.g, gTubeColor.b);
    glUniform3f(tubePositionLoc, gTubePosition.x, gTubePosition.y, gTubePosition.z);
    glUniform3f(cube1ColorLoc, gCube1Color.r, gCube1Color.g, gCube1Color.b);
    glUniform3f(cube1PositionLoc, gCube1Position.x, gCube1Position.y, gCube1Position.z);
    glUniform3f(cube2ColorLoc, gCube2Color.r, gCube2Color.g, gCube2Color.b);
    glUniform3f(cube2PositionLoc, gCube2Position.x, gCube2Position.y, gCube2Position.z);
    glUniform3f(sphereColorLoc, gSphereColor.r, gSphereColor.g, gSphereColor.b);
    glUniform3f(spherePositionLoc, gSpherePosition.x, gSpherePosition.y, gSpherePosition.z);
    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));


    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, gTextureId2);
    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 36, 6);

    // LAMP: draw lamp
    //----------------
    glUseProgram(gLampProgramId);

    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gLightPosition) * glm::scale(gLightScale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Key light
    glUseProgram(gLampProgramId);

    model = glm::translate(gKeyLightPosition) * glm::scale(gKeyLightScale);

    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Tube
    //----------------
    glUseProgram(gProgramId);

    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gTubePosition) * glm::scale(gTubeScale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, gTextureId1);
    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 42, 30);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, gTextureId3);
    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 72, 66);

    // Octagon
    //----------------
    glUseProgram(gProgramId);

    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gOctPosition) * glm::scale(gOctScale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, gTextureId4);
    glDrawArrays(GL_TRIANGLES, 138, 84);

    // Cube 2
    //----------------
    glUseProgram(gProgramId);

    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gCube2Position) * glm::scale(gCube2Scale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, gTextureId5);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Cube 1
    //----------------
    glUseProgram(gProgramId);

    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gCube1Position) * glm::scale(gCube1Scale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, gTextureId5);
    glDrawArrays(GL_TRIANGLES, 222, 36);

    // Sphere
    //----------------
    glUseProgram(gProgramId);

    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gSpherePosition) * glm::scale(gSphereScale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, gTextureId6);
    glDrawArrays(GL_TRIANGLES, 258, 192);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
    glUseProgram(0);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}

// Implements the UCreateMesh function
void UCreateMesh(GLMesh& mesh)
{
    // Vertex Data
    GLfloat verts[] = {

        // Floating cube lamp
        //Back Face          //Negative Z Normal  Texture Coords.
       -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
       -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
       -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

       //Front Face         //Positive Z Normal
      -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
       0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
       0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
       0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
      -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
      -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

      //Left Face          //Negative X Normal
     -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

     //Right Face         //Positive X Normal
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

     //Bottom Face        //Negative Y Normal
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

    //Top Face           //Positive Y Normal
   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
    0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
   -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,

   // White Table
  -15.0f, -5.0f, -12.10f,   0.0f, -1.0f, 0.0f,   0.5f, 1.0f,
   15.0f, -5.0f, -12.10f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f,
   15.0f, -5.0f,  12.10f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f,
   15.0f, -5.0f,  12.10f,   0.0f, -1.0f, 0.0f,   0.5f, 1.0f,
  -15.0f, -5.0f,  12.10f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f,
  -15.0f, -5.0f, -12.10f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f,

  //Body of the tube
  //Triangle 0
  1.0f, 3.0f, 0.0f,                      0.0f,  0.0f, 1.0f,  0.0f, 0.0f, // Coordinate 1
 -1.0f, 3.0f, 0.0f,                      0.0f,  0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 0
  cos(5 * PI / 8), 0, -sin(5 * PI / 8),  0.0f,  0.0f, 1.0f,  0.5f, 1.0f, // Coordinate 6
  //Triangle 1
  1.0f, 3.0f, 0.0f,                        0.0f,  0.0f, 1.0f,  0.0f, 0.0f, // Coordinate 1
 -1.0f, 3.0f, 0.0f,                        0.0f,  0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 0
  cos(13 * PI / 8), 0, -sin(13 * PI / 8),  0.0f,  0.0f, 1.0f,  0.5f, 1.0f, // Coordinate 2
  // Triangle 2
  1.0f, 3.0f, 0.0f,                      0.0f,  0.0f, 1.0f,  0.5f, 1.0f, // Coordinate 1
  cos(3 * PI / 8), 0, -sin(3 * PI / 8),  0.0f,  0.0f, 1.0f,  0.0f, 0.0f, // Coordinate 5
  cos(5 * PI / 8), 0, -sin(5 * PI / 8),  0.0f,  0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 6
  // Triangle 3
  1.0f, 3.0f, 0.0f,                      0.0f,  0.0f, 1.0f,  0.5f, 1.0f, // Coordinate 1
  cos(3 * PI / 8), 0, -sin(3 * PI / 8),  0.0f,  0.0f, 1.0f,  0.0f, 0.0f, // Coordinate 5
  cos(1 * PI / 8), 0, -sin(1 * PI / 8),  0.0f,  0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 4
  //Triangle 4
  1.0f, 3.0f, 0.0f,                        0.0f, 0.0f,  1.0f,  0.5f, 1.0f, // Coordinate 1
  cos(15 * PI / 8), 0, -sin(15 * PI / 8),  0.0f, 0.0f,  1.0f,  0.0f, 0.0f, // Coordinate 3
  cos(1 * PI / 8), 0, -sin(1 * PI / 8),    0.0f, 0.0f,  1.0f,  1.0f, 0.0f, // Coordinate 4
  //Triangle 5
  1.0f, 3.0f, 0.0f,                        0.0f,  0.0f,  1.0f,  0.5f, 1.0f, // Coordinate 1
  cos(15 * PI / 8), 0, -sin(15 * PI / 8),  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, // Coordinate 3
  cos(13 * PI / 8), 0, -sin(13 * PI / 8),  0.0f,  0.0f,  1.0f,  1.0f, 0.0f, // Coordinate 2
  //Triangle 6
  -1.0f, 3.0f, 0.0f,                        0.0f,  0.0f, 1.0f,  0.5f, 1.0f, // Coordinate 0
  cos(13 * PI / 8), 0, -sin(13 * PI / 8),   0.0f,  0.0f, 1.0f,  0.0f, 0.0f, // Coordinate 2
  cos(11 * PI / 8), 0, -sin(11 * PI / 8),   0.0f,  0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 9
  //Triangle 7
  -1.0f, 3.0f, 0.0f,                       0.0f,  0.0f, 1.0f,  0.5f, 1.0f, // Coordinate 0
  cos(9 * PI / 8), 0, -sin(9 * PI / 8),    0.0f,  0.0f, 1.0f,  0.0f, 0.0f, // Coordinate 8
  cos(11 * PI / 8), 0, -sin(11 * PI / 8),  0.0f,  0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 9
  //Triangle 8
  -1.0f, 3.0f, 0.0f,                     0.0f,  0.0f, 1.0f,  0.5f, 1.0f, // Coordinate 0
  cos(9 * PI / 8), 0, -sin(9 * PI / 8),  0.0f,  0.0f, 1.0f,  0.0f, 0.0f, // Coordinate 8
  cos(7 * PI / 8), 0, -sin(7 * PI / 8),  0.0f,  0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 7
  //Triangle 9
  -1.0f, 3.0f, 0.0f,                     0.0f,  0.0f, 1.0f,  0.5f, 1.0f, // Coordinate 0
  cos(7 * PI / 8), 0, -sin(7 * PI / 8),  0.0f,  0.0f, 1.0f,  0.0f, 0.0f, // Coordinate 7
  cos(5 * PI / 8), 0, -sin(5 * PI / 8),  0.0f,  0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 6

  // Cap of the tube
  //Triangle 0
  cos(13 * PI / 8), 0, -sin(13 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 0
  cos(15 * PI / 8), 0, -sin(15 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 1
  cos(13 * PI / 8), -1, -sin(13 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 8
  // Triangle 1
  cos(15 * PI / 8), 0, -sin(15 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 1
  cos(13 * PI / 8), -1, -sin(13 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 8
  cos(15 * PI / 8), -1, -sin(15 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 9
  //Triangle 2
  cos(15 * PI / 8), 0, -sin(15 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 1
  cos(15 * PI / 8), -1, -sin(15 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 9
  cos(1 * PI / 8), 0, -sin(1 * PI / 8),     0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 2
  //Triangle 3
  cos(15 * PI / 8), -1, -sin(15 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 9
  cos(1 * PI / 8), 0, -sin(1 * PI / 8),     0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 2
  cos(1 * PI / 8), -1, -sin(1 * PI / 8),    0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 10
  //Triangle 4
  cos(1 * PI / 8), 0, -sin(1 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 2
  cos(1 * PI / 8), -1, -sin(1 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 10
  cos(3 * PI / 8), 0, -sin(3 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 3
  //Triangle 5
  cos(1 * PI / 8), -1, -sin(1 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 10
  cos(3 * PI / 8), 0, -sin(3 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 3
  cos(3 * PI / 8), -1, -sin(3 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,// Coordinate 11
  //Triangle 6
  cos(3 * PI / 8), 0, -sin(3 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 3
  cos(3 * PI / 8), -1, -sin(3 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 11
  cos(5 * PI / 8), 0, -sin(5 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 4
  //Triangle 7
  cos(3 * PI / 8), -1, -sin(3 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 11
  cos(5 * PI / 8), 0, -sin(5 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 4
  cos(5 * PI / 8), -1, -sin(5 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 12
  //Triangle 8 
  cos(5 * PI / 8), 0, -sin(5 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 4
  cos(5 * PI / 8), -1, -sin(5 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 12
  cos(7 * PI / 8), 0, -sin(7 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 5
  //Triangle 9
  cos(5 * PI / 8), -1, -sin(5 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 12
  cos(7 * PI / 8), 0, -sin(7 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 5
  cos(7 * PI / 8), -1, -sin(7 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 13
  //Triangle 10
  cos(7 * PI / 8), 0, -sin(7 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 5
  cos(7 * PI / 8), -1, -sin(7 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 13
  cos(9 * PI / 8), 0, -sin(9 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 6
  //Triangle 11
  cos(7 * PI / 8), -1, -sin(7 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 13
  cos(9 * PI / 8), 0, -sin(9 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 6
  cos(9 * PI / 8), -1, -sin(9 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 14
  //Triangle 12
  cos(9 * PI / 8), 0, -sin(9 * PI / 8),    0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 6
  cos(9 * PI / 8), -1, -sin(9 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 14
  cos(11 * PI / 8), 0, -sin(11 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 7
  //Triangle 13
  cos(9 * PI / 8), -1, -sin(9 * PI / 8),    0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 14
  cos(11 * PI / 8), 0, -sin(11 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 7
  cos(11 * PI / 8), -1, -sin(11 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 15
  //Triangle 14
  cos(11 * PI / 8), 0, -sin(11 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 7
  cos(11 * PI / 8), -1, -sin(11 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 15
  cos(13 * PI / 8), 0, -sin(13 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 0
  //Triangle 15
  cos(11 * PI / 8), -1, -sin(11 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 15
  cos(13 * PI / 8), 0, -sin(13 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 0
  cos(13 * PI / 8), -1, -sin(13 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 8
  //Triangle 16
  cos(5 * PI / 8), -1, -sin(5 * PI / 8),  0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // Coordinate 12
  cos(7 * PI / 8), -1, -sin(7 * PI / 8),  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // Coordinate 13
  cos(9 * PI / 8), -1, -sin(9 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 14
  //Triangle 17
  cos(9 * PI / 8), -1, -sin(9 * PI / 8),    0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // Coordinate 14
  cos(5 * PI / 8), -1, -sin(5 * PI / 8),    0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // Coordinate 12
  cos(11 * PI / 8), -1, -sin(11 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 15
  //Triangle 18
  cos(5 * PI / 8), -1, -sin(5 * PI / 8),    0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // Coordinate 12
  cos(11 * PI / 8), -1, -sin(11 * PI / 8),  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // Coordinate 15
  cos(13 * PI / 8), -1, -sin(13 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 8
  //Triangle 19
  cos(13 * PI / 8), -1, -sin(13 * PI / 8),  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // Coordinate 8
  cos(5 * PI / 8), -1, -sin(5 * PI / 8),    0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // Coordinate 12
  cos(15 * PI / 8), -1, -sin(15 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 9
  //Triangle 20
  cos(5 * PI / 8), -1, -sin(5 * PI / 8),    0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // Coordinate 12
  cos(15 * PI / 8), -1, -sin(15 * PI / 8),  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // Coordinate 9
  cos(1 * PI / 8), -1, -sin(1 * PI / 8),    0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 10
  //Triangle 21
  cos(1 * PI / 8), -1, -sin(1 * PI / 8),  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // Coordinate 10
  cos(5 * PI / 8), -1, -sin(5 * PI / 8),  0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // Coordinate 12
  cos(3 * PI / 8), -1, -sin(3 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 11

  // Octagon
  //Triangle 0
  2*cos(13 * PI / 8), 0, -2*sin(13 * PI / 8),   0.0f, 0.0f, 1.0f,   1.0f, 0.0f, // Coordinate 0
  2*cos(15 * PI / 8), 0, -2*sin(15 * PI / 8),   0.0f, 0.0f, 1.0f,   1.0f, 0.0f, // Coordinate 1
  2*cos(13 * PI / 8), -1, -2*sin(13 * PI / 8),  0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // Coordinate 8
  // Triangle 1
  2*cos(15 * PI / 8), 0, -2*sin(15 * PI / 8),    0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 1
  2*cos(13 * PI / 8), -1, -2*sin(13 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 8
  2*cos(15 * PI / 8), -1, -2*sin(15 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 9
  //Triangle 2
  2*cos(15 * PI / 8), 0, -2*sin(15 * PI / 8),   0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // Coordinate 1
  2*cos(15 * PI / 8), -1, -2*sin(15 * PI / 8),  0.0f, 0.0f, 1.0f,   1.0f, 0.0f, // Coordinate 9
  2*cos(1 * PI / 8), 0, -2*sin(1 * PI / 8),     0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // Coordinate 2
  //Triangle 3
  2*cos(15 * PI / 8), -1, -2*sin(15 * PI / 8),  0.0f, 0.0f, 1.0f,   1.0f, 0.0f, // Coordinate 9
  2*cos(1 * PI / 8), 0, -2*sin(1 * PI / 8),     0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // Coordinate 2
  2*cos(1 * PI / 8), -1, -2*sin(1 * PI / 8),    0.0f, 0.0f, 1.0f,   1.0f, 0.0f, // Coordinate 10
  //Triangle 4
  2*cos(1 * PI / 8), 0, -2*sin(1 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 2
  2*cos(1 * PI / 8), -1, -2*sin(1 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 10
  2*cos(3 * PI / 8), 0, -2*sin(3 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 3
  //Triangle 5
  2*cos(1 * PI / 8), -1, -2*sin(1 * PI / 8),  0.0f, 0.0f, 1.0f,   1.0f, 0.0f, // Coordinate 10
  2*cos(3 * PI / 8), 0, -2*sin(3 * PI / 8),   0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // Coordinate 3
  2*cos(3 * PI / 8), -1, -2*sin(3 * PI / 8),  0.0f, 0.0f, 1.0f,   1.0f, 0.0f,// Coordinate 11
  //Triangle 6
  2*cos(3 * PI / 8), 0, -2*sin(3 * PI / 8),   0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // Coordinate 3
  2*cos(3 * PI / 8), -1, -2*sin(3 * PI / 8),  0.0f, 0.0f, 1.0f,   1.0f, 0.0f, // Coordinate 11
  2*cos(5 * PI / 8), 0, -2*sin(5 * PI / 8),   0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // Coordinate 4
  //Triangle 7
  2*cos(3 * PI / 8), -1, -2*sin(3 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 11
  2*cos(5 * PI / 8), 0, -2*sin(5 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 4
  2*cos(5 * PI / 8), -1, -2*sin(5 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 12
  //Triangle 8 
  2*cos(5 * PI / 8), 0, -2*sin(5 * PI / 8),   0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // Coordinate 4
  2*cos(5 * PI / 8), -1, -2*sin(5 * PI / 8),  0.0f, 0.0f, 1.0f,   1.0f, 0.0f, // Coordinate 12
  2*cos(7 * PI / 8), 0, -2*sin(7 * PI / 8),   0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // Coordinate 5
  //Triangle 9
  2*cos(5 * PI / 8), -1, -2*sin(5 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 12
  2*cos(7 * PI / 8), 0, -2*sin(7 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 5
  2*cos(7 * PI / 8), -1, -2*sin(7 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 13
  //Triangle 10
  2*cos(7 * PI / 8), 0, -2*sin(7 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 5
  2*cos(7 * PI / 8), -1, -2*sin(7 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 13
  2*cos(9 * PI / 8), 0, -2*sin(9 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 6
  //Triangle 11
  2*cos(7 * PI / 8), -1, -2*sin(7 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 13
  2*cos(9 * PI / 8), 0, -2*sin(9 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 6
  2*cos(9 * PI / 8), -1, -2*sin(9 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 14
  //Triangle 12
  2*cos(9 * PI / 8), 0, -2*sin(9 * PI / 8),    0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 6
  2*cos(9 * PI / 8), -1, -2*sin(9 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 14
  2*cos(11 * PI / 8), 0, -2*sin(11 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 7
  //Triangle 13
  2*cos(9 * PI / 8), -1, -2*sin(9 * PI / 8),    0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 14
  2*cos(11 * PI / 8), 0, -2*sin(11 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 7
  2*cos(11 * PI / 8), -1, -2*sin(11 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 15
  //Triangle 14
  2*cos(11 * PI / 8), 0, -2*sin(11 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 7
  2*cos(11 * PI / 8), -1, -2*sin(11 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 15
  2*cos(13 * PI / 8), 0, -2*sin(13 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 0
  //Triangle 15
  2*cos(11 * PI / 8), -1, -2*sin(11 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 15
  2*cos(13 * PI / 8), 0, -2*sin(13 * PI / 8),   0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // Coordinate 0
  2*cos(13 * PI / 8), -1, -2*sin(13 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 8
  // Triangle 16
  2*cos(5 * PI / 8), 0, -2*sin(5 * PI / 8),  0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // Coordinate 4
  2*cos(7 * PI / 8), 0, -2*sin(7 * PI / 8),  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // Coordinate 5
  2*cos(9 * PI / 8), 0, -2*sin(9 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 6
  // Triangle 17
  2*cos(5 * PI / 8), 0, -2*sin(5 * PI / 8),   0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // Coordinate 4
  2*cos(9 * PI / 8), 0, -2*sin(9 * PI / 8),   0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // Coordinate 6
  2*cos(11 * PI / 8), 0, -2*sin(11 * PI / 8), 0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 7
  // Triangle 18
  2*cos(5 * PI / 8), 0, -2*sin(5 * PI / 8),    0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // Coordinate 4
  2*cos(11 * PI / 8), 0, -2*sin(11 * PI / 8),  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // Coordinate 7
  2*cos(13 * PI / 8), 0, -2*sin(13 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 0
  // Triangle 19
  2*cos(5 * PI / 8), 0, -2*sin(5 * PI / 8),    0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // Coordinate 4
  2*cos(13 * PI / 8), 0, -2*sin(13 * PI / 8),  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // Coordinate 0
  2*cos(15 * PI / 8), 0, -2*sin(15 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 1
  // Triangle 20
  2*cos(5 * PI / 8), 0, -2*sin(5 * PI / 8),    0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // Coordinate 4
  2*cos(15 * PI / 8), 0, -2*sin(15 * PI / 8),  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // Coordinate 1
  2*cos(1 * PI / 8), 0, -2*sin(1 * PI / 8),    0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 2
  // Triangle 21
  2*cos(5 * PI / 8), 0, -2*sin(5 * PI / 8),  0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // Coordinate 4
  2*cos(1 * PI / 8), 0, -2*sin(1 * PI / 8),  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // Coordinate 2
  2*cos(3 * PI / 8), 0, -2*sin(3 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 3
  //Triangle 22
  2*cos(5 * PI / 8), -1, -2*sin(5 * PI / 8),  0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // Coordinate 12
  2*cos(7 * PI / 8), -1, -2*sin(7 * PI / 8),  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // Coordinate 13
  2*cos(9 * PI / 8), -1, -2*sin(9 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 14
  //Triangle 23
  2*cos(9 * PI / 8), -1, -2*sin(9 * PI / 8),    0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // Coordinate 14
  2*cos(5 * PI / 8), -1, -2*sin(5 * PI / 8),    0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // Coordinate 12
  2*cos(11 * PI / 8), -1, -2*sin(11 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 15
  //Triangle 24
  2*cos(5 * PI / 8), -1, -2*sin(5 * PI / 8),    0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // Coordinate 12
  2*cos(11 * PI / 8), -1, -2*sin(11 * PI / 8),  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // Coordinate 15
  2*cos(13 * PI / 8), -1, -2*sin(13 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 8
  //Triangle 25
  2*cos(13 * PI / 8), -1, -2*sin(13 * PI / 8),  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // Coordinate 8
  2*cos(5 * PI / 8), -1, -2*sin(5 * PI / 8),    0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // Coordinate 12
  2*cos(15 * PI / 8), -1, -2*sin(15 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 9
  //Triangle 26
  2*cos(5 * PI / 8), -1, -2*sin(5 * PI / 8),    0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // Coordinate 12
  2*cos(15 * PI / 8), -1, -2*sin(15 * PI / 8),  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // Coordinate 9
  2*cos(1 * PI / 8), -1, -2*sin(1 * PI / 8),    0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 10
  //Triangle 27
  2*cos(1 * PI / 8), -1, -2*sin(1 * PI / 8),  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // Coordinate 10
  2*cos(5 * PI / 8), -1, -2*sin(5 * PI / 8),  0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // Coordinate 12
  2*cos(3 * PI / 8), -1, -2*sin(3 * PI / 8),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Coordinate 11

  // Top Blue Cube
  //Back Face          //Negative Z Normal  Texture Coords.
 -0.6f, -0.1f, -0.6f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
  0.6f, -0.1f, -0.6f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
  0.6f,  0.1f, -0.6f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
  0.6f,  0.1f, -0.6f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
 -0.6f,  0.1f, -0.6f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
 -0.6f, -0.1f, -0.6f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

  //Front Face         //Positive Z Normal
 -0.6f, -0.1f,  0.6f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
  0.6f, -0.1f,  0.6f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
  0.6f,  0.1f,  0.6f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
  0.6f,  0.1f,  0.6f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
 -0.6f,  0.1f,  0.6f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
 -0.6f, -0.1f,  0.6f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

 //Left Face          //Negative X Normal
 -0.6f,  0.1f,  0.6f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
 -0.6f,  0.1f, -0.6f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
 -0.6f, -0.1f, -0.6f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
 -0.6f, -0.1f, -0.6f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
 -0.6f, -0.1f,  0.6f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
 -0.6f,  0.1f,  0.6f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

 //Right Face         //Positive X Normal
  0.6f,  0.1f,  0.6f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
  0.6f,  0.1f, -0.6f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
  0.6f, -0.1f, -0.6f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
  0.6f, -0.1f, -0.6f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
  0.6f, -0.1f,  0.6f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
  0.6f,  0.1f,  0.6f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

  //Bottom Face        //Negative Y Normal
 -0.6f, -0.1f, -0.6f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
  0.6f, -0.1f, -0.6f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
  0.6f, -0.1f,  0.6f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
  0.6f, -0.1f,  0.6f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
 -0.6f, -0.1f,  0.6f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
 -0.6f, -0.1f, -0.6f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

  //Top Face           //Positive Y Normal
 -0.6f,  0.1f, -0.6f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
  0.6f,  0.1f, -0.6f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
  0.6f,  0.1f,  0.6f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
  0.6f,  0.1f,  0.6f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
 -0.6f,  0.1f,  0.6f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
 -0.6f,  0.1f, -0.6f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,

  // Pink Sphere
  // Sphere Top
  // 1
  1 * cos(0.5 * (PI / 4)), 0, 1 * sin(0.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // C
  0, 0, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A
  1 * cos(1 * (PI / 4)), 0, 1 * sin(1 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // D
  // 2
  1 * cos(1 * (PI / 4)), 0, 1 * sin(1 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // D
  0, 0, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A
  1 * cos(1.5 * (PI / 4)), 0, 1 * sin(1.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // E
  // 3
  1 * cos(1.5 * (PI / 4)), 0, 1 * sin(1.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // E
  0, 0, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A
  1 * cos(2 * (PI / 4)), 0, 1 * sin(2 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // F
  // 4
  1 * cos(2 * (PI / 4)), 0, 1 * sin(2 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // F
  0, 0, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A
  1 * cos(2.5 * (PI / 4)), 0, 1 * sin(2.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // G
  // 5
  1 * cos(2.5 * (PI / 4)), 0, 1 * sin(2.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // G
  0, 0, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A
  1 * cos(3 * (PI / 4)), 0, 1 * sin(3 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // H
  // 6
  1 * cos(3 * (PI / 4)), 0, 1 * sin(3 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // H
  0, 0, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A
  1 * cos(3.5 * (PI / 4)), 0, 1 * sin(3.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // I
  // 7
  1 * cos(3.5 * (PI / 4)), 0, 1 * sin(3.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // I
  0, 0, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A
  1 * cos(4 * (PI / 4)), 0, 1 * sin(4 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // J
  // 8
  1 * cos(4 * (PI / 4)), 0, 1 * sin(4 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // J
  0, 0, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A
  1 * cos(4.5 * (PI / 4)), 0, 1 * sin(4.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // K
  // 9
  1 * cos(4.5 * (PI / 4)), 0, 1 * sin(4.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // K
  0, 0, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A
  1 * cos(5 * (PI / 4)), 0, 1 * sin(5 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // L
  // 10
  1 * cos(5 * (PI / 4)), 0, 1 * sin(5 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // L
  0, 0, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A
  1 * cos(5.5 * (PI / 4)), 0, 1 * sin(5.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // M
  // 11
  1 * cos(5.5 * (PI / 4)), 0, 1 * sin(5.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // M
  0, 0, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A
  1 * cos(6 * (PI / 4)), 0, 1 * sin(6 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // N
  // 12
  1 * cos(6 * (PI / 4)), 0, 1 * sin(6 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // N
  0, 0, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A
  1 * cos(6.5 * (PI / 4)), 0, 1 * sin(6.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // O
  // 13
  1 * cos(6.5 * (PI / 4)), 0, 1 * sin(6.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // O
  0, 0, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A
  1 * cos(7 * (PI / 4)), 0, 1 * sin(7 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // P
  // 14
  1 * cos(7 * (PI / 4)), 0, 1 * sin(7 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // P
  0, 0, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A
  1 * cos(7.5 * (PI / 4)), 0, 1 * sin(7.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Q
  // 15
  1 * cos(7.5 * (PI / 4)), 0, 1 * sin(7.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Q
  0, 0, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A
  1, 0, 0,                                              0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // B
  // 16
  1, 0, 0,                                              0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // B
  0, 0, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A
  1 * cos(0.5 * (PI / 4)), 0, 1 * sin(0.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // C
  // Sphere Bottom
  // 1
  1 * cos(0.5 * (PI / 4)), 0.5, 1 * sin(0.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // C1
  0, 0.5, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A1
  1 * cos(1 * (PI / 4)), 0.5, 1 * sin(1 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // D1
  // 2
  1 * cos(1 * (PI / 4)), 0.5, 1 * sin(1 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // D1
  0, 0.5, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A1
  1 * cos(1.5 * (PI / 4)), 0.5, 1 * sin(1.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // E1
  // 3
  1 * cos(1.5 * (PI / 4)), 0.5, 1 * sin(1.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // E1
  0, 0.5, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A1
  1 * cos(2 * (PI / 4)), 0.5, 1 * sin(2 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // F1
  // 4
  1 * cos(2 * (PI / 4)), 0.5, 1 * sin(2 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // F1
  0, 0.5, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A1
  1 * cos(2.5 * (PI / 4)), 0.5, 1 * sin(2.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // G1
  // 5
  1 * cos(2.5 * (PI / 4)), 0.5, 1 * sin(2.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // G1
  0, 0.5, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A1
  1 * cos(3 * (PI / 4)), 0.5, 1 * sin(3 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // H1
  // 6
  1 * cos(3 * (PI / 4)), 0.5, 1 * sin(3 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // H1
  0, 0.5, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A1
  1 * cos(3.5 * (PI / 4)), 0.5, 1 * sin(3.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // I1
  // 7
  1 * cos(3.5 * (PI / 4)), 0.5, 1 * sin(3.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // I1
  0, 0.5, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A1
  1 * cos(4 * (PI / 4)), 0.5, 1 * sin(4 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // J1
  // 8
  1 * cos(4 * (PI / 4)), 0.5, 1 * sin(4 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // J1
  0, 0.5, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A1
  1 * cos(4.5 * (PI / 4)), 0.5, 1 * sin(4.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // K1
  // 9
  1 * cos(4.5 * (PI / 4)), 0.5, 1 * sin(4.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // K1
  0, 0.5, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A1
  1 * cos(5 * (PI / 4)), 0.5, 1 * sin(5 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // L1
  // 10
  1 * cos(5 * (PI / 4)), 0.5, 1 * sin(5 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // L1
  0, 0.5, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A1
  1 * cos(5.5 * (PI / 4)), 0.5, 1 * sin(5.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // M1
  // 11
  1 * cos(5.5 * (PI / 4)), 0.5, 1 * sin(5.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // M1
  0, 0.5, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A1
  1 * cos(6 * (PI / 4)), 0.5, 1 * sin(6 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // N1
  // 12
  1 * cos(6 * (PI / 4)), 0.5, 1 * sin(6 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // N1
  0, 0.5, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A1
  1 * cos(6.5 * (PI / 4)), 0.5, 1 * sin(6.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // O1
  // 13
  1 * cos(6.5 * (PI / 4)), 0.5, 1 * sin(6.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // O1
  0, 0.5, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A1
  1 * cos(7 * (PI / 4)), 0.5, 1 * sin(7 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // P1
  // 14
  1 * cos(7 * (PI / 4)), 0.5, 1 * sin(7 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // P1
  0, 0.5, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A1
  1 * cos(7.5 * (PI / 4)), 0.5, 1 * sin(7.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Q1
  // 15
  1 * cos(7.5 * (PI / 4)), 0.5, 1 * sin(7.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Q1
  0, 0.5, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A1
  1, 0.5, 0,                                              0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // B1
  // 16
  1, 0.5, 0,                                              0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // B1
  0, 0.5, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // A1
  1 * cos(0.5 * (PI / 4)), 0.5, 1 * sin(0.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // C1
  // Sphere side 1
  // 1
  1, 0, 0,                                              0.0f, 0.0f, 1.0f,  1.0f, 0.0f,// B
  1, 0.5, 0,                                            0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // B1
  1 * cos(0.5 * (PI / 4)), 0, 1 * sin(0.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  0.5f, 1.0f,  // C
  // 2 
  1 * cos(0.5 * (PI / 4)), 0, 1 * sin(0.5 * (PI / 4)),    0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // C
  1 * cos(0.5 * (PI / 4)), 0.5, 1 * sin(0.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // C1
  1 * cos(1 * (PI / 4)), 0, 1 * sin(1 * (PI / 4)),        0.0f, 0.0f, 1.0f,  0.5f, 1.0f,  // D
  // 3
  1 * cos(1 * (PI / 4)), 0, 1 * sin(1 * (PI / 4)),      0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // D
  1 * cos(1 * (PI / 4)), 0.5, 1 * sin(1 * (PI / 4)),    0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // D1
  1 * cos(1.5 * (PI / 4)), 0, 1 * sin(1.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  0.5f, 1.0f,  // E
  // 4
  1 * cos(1.5 * (PI / 4)), 0, 1 * sin(1.5 * (PI / 4)),    0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // E
  1 * cos(1.5 * (PI / 4)), 0.5, 1 * sin(1.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // E1
  1 * cos(2 * (PI / 4)), 0, 1 * sin(2 * (PI / 4)),        0.0f, 0.0f, 1.0f,  0.5f, 1.0f,  // F
  // 5
  1 * cos(2 * (PI / 4)), 0, 1 * sin(2 * (PI / 4)),      0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // F
  1 * cos(2 * (PI / 4)), 0.5, 1 * sin(2 * (PI / 4)),    0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // F1
  1 * cos(2.5 * (PI / 4)), 0, 1 * sin(2.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  0.5f, 1.0f,  // G
  // 6
  1 * cos(2.5 * (PI / 4)), 0, 1 * sin(2.5 * (PI / 4)),    0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // G
  1 * cos(2.5 * (PI / 4)), 0.5, 1 * sin(2.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // G1
  1 * cos(3 * (PI / 4)), 0, 1 * sin(3 * (PI / 4)),        0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // H
  // 7
  1 * cos(3 * (PI / 4)), 0, 1 * sin(3 * (PI / 4)),      0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // H
  1 * cos(3 * (PI / 4)), 0.5, 1 * sin(3 * (PI / 4)),    0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // H1
  1 * cos(3.5 * (PI / 4)), 0, 1 * sin(3.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  0.5f, 1.0f,  // I
  // 8
  1 * cos(3.5 * (PI / 4)), 0, 1 * sin(3.5 * (PI / 4)),    0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // I
  1 * cos(3.5 * (PI / 4)), 0.5, 1 * sin(3.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // I1
  1 * cos(4 * (PI / 4)), 0, 1 * sin(4 * (PI / 4)),        0.0f, 0.0f, 1.0f,  0.5f, 1.0f,  // J
  // 9
  1 * cos(4 * (PI / 4)), 0, 1 * sin(4 * (PI / 4)),      0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // J
  1 * cos(4 * (PI / 4)), 0.5, 1 * sin(4 * (PI / 4)),    0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // J1
  1 * cos(4.5 * (PI / 4)), 0, 1 * sin(4.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  0.5f, 1.0f,  // K
  // 10
  1 * cos(4.5 * (PI / 4)), 0, 1 * sin(4.5 * (PI / 4)),    0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // K
  1 * cos(4.5 * (PI / 4)), 0.5, 1 * sin(4.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // K1
  1 * cos(5 * (PI / 4)), 0, 1 * sin(5 * (PI / 4)),        0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // L
  // 11
  1 * cos(5 * (PI / 4)), 0, 1 * sin(5 * (PI / 4)),      0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // L
  1 * cos(5 * (PI / 4)), 0.5, 1 * sin(5 * (PI / 4)),    0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // L1 
  1 * cos(5.5 * (PI / 4)), 0, 1 * sin(5.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // M
  // 12
  1 * cos(5.5 * (PI / 4)), 0, 1 * sin(5.5 * (PI / 4)),    0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // M
  1 * cos(5.5 * (PI / 4)), 0.5, 1 * sin(5.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // M1
  1 * cos(6 * (PI / 4)), 0, 1 * sin(6 * (PI / 4)),        0.0f, 0.0f, 1.0f,  0.5f, 1.0f,  // N
  // 13
  1 * cos(6 * (PI / 4)), 0, 1 * sin(6 * (PI / 4)),      0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // N
  1 * cos(6 * (PI / 4)), 0.5, 1 * sin(6 * (PI / 4)),    0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // N1
  1 * cos(6.5 * (PI / 4)), 0, 1 * sin(6.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // O
  // 14
  1 * cos(6.5 * (PI / 4)), 0, 1 * sin(6.5 * (PI / 4)),    0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // O
  1 * cos(6.5 * (PI / 4)), 0.5, 1 * sin(6.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // O1
  1 * cos(6.5 * (PI / 4)), 0, 1 * sin(6.5 * (PI / 4)),    0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // P
  // 15
  1 * cos(6.5 * (PI / 4)), 0, 1 * sin(6.5 * (PI / 4)),    0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // P
  1 * cos(6.5 * (PI / 4)), 0.5, 1 * sin(6.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // P1
  1 * cos(7.5 * (PI / 4)), 0, 1 * sin(7.5 * (PI / 4)),    0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // Q
  // 16
  1 * cos(7.5 * (PI / 4)), 0, 1 * sin(7.5 * (PI / 4)),    0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Q
  1 * cos(7.5 * (PI / 4)), 0.5, 1 * sin(7.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // Q1
  1, 0, 0,                                                0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // B
  // Sphere Side 2
  // 1
  1, 0.5, 0,                                              0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // B1
  1 * cos(0.5 * (PI / 4)), 0, 1 * sin(0.5 * (PI / 4)),    0.0f, 0.0f, 1.0f,  0.0f, 1.0f,  // C
  1 * cos(0.5 * (PI / 4)), 0.5, 1 * sin(0.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // C1
  // 2
  1 * cos(0.5 * (PI / 4)), 0.5, 1 * sin(0.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // C1
  1 * cos(1 * (PI / 4)), 0, 1 * sin(1 * (PI / 4)),        0.0f, 0.0f, 1.0f,  0.0f, 1.0f,  // D
  1 * cos(1 * (PI / 4)), 0.5, 1 * sin(1 * (PI / 4)),      0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // D1
  // 3
  1 * cos(1 * (PI / 4)), 0.5, 1 * sin(1 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // D1
  1 * cos(1.5 * (PI / 4)), 0, 1 * sin(1.5 * (PI / 4)),    0.0f, 0.0f, 1.0f,  0.0f, 1.0f,  // E
  1 * cos(1.5 * (PI / 4)), 0.5, 1 * sin(1.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // E1
  // 4
  1 * cos(1.5 * (PI / 4)), 0.5, 1 * sin(1.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // E1
  1 * cos(2 * (PI / 4)), 0, 1 * sin(2 * (PI / 4)),        0.0f, 0.0f, 1.0f,  0.0f, 1.0f,  // F
  1 * cos(2 * (PI / 4)), 0.5, 1 * sin(2 * (PI / 4)),      0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // F1
  // 5
  1 * cos(2 * (PI / 4)), 0.5, 1 * sin(2 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // F1
  1 * cos(2.5 * (PI / 4)), 0, 1 * sin(2.5 * (PI / 4)),    0.0f, 0.0f, 1.0f,  0.0f, 1.0f,  // G
  1 * cos(2.5 * (PI / 4)), 0.5, 1 * sin(2.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // G1
  // 6
  1 * cos(2.5 * (PI / 4)), 0.5, 1 * sin(2.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // G1
  1 * cos(3 * (PI / 4)), 0, 1 * sin(3 * (PI / 4)),        0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // H
  1 * cos(3 * (PI / 4)), 0.5, 1 * sin(3 * (PI / 4)),      0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // H1
  // 7
  1 * cos(3 * (PI / 4)), 0.5, 1 * sin(3 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // H1
  1 * cos(3.5 * (PI / 4)), 0, 1 * sin(3.5 * (PI / 4)),    0.0f, 0.0f, 1.0f,  0.0f, 1.0f,  // I
  1 * cos(3.5 * (PI / 4)), 0.5, 1 * sin(3.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // I1
  // 8
  1 * cos(3.5 * (PI / 4)), 0.5, 1 * sin(3.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // I1
  1 * cos(4 * (PI / 4)), 0, 1 * sin(4 * (PI / 4)),        0.0f, 0.0f, 1.0f,  0.0f, 1.0f,  // J
  1 * cos(4 * (PI / 4)), 0.5, 1 * sin(4 * (PI / 4)),      0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // J1
  // 9
  1 * cos(4 * (PI / 4)), 0.5, 1 * sin(4 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // J1
  1 * cos(4.5 * (PI / 4)), 0, 1 * sin(4.5 * (PI / 4)),    0.0f, 0.0f, 1.0f,  0.0f, 1.0f,  // K
  1 * cos(4.5 * (PI / 4)), 0.5, 1 * sin(4.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // K1
  // 10
  1 * cos(4.5 * (PI / 4)), 0.5, 1 * sin(4.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // K1
  1 * cos(5 * (PI / 4)), 0, 1 * sin(5 * (PI / 4)),        0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // L
  1 * cos(5 * (PI / 4)), 0.5, 1 * sin(5 * (PI / 4)),      0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // L1
  // 11
  1 * cos(5 * (PI / 4)), 0.5, 1 * sin(5 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // L1 
  1 * cos(5.5 * (PI / 4)), 0, 1 * sin(5.5 * (PI / 4)),    0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // M
  1 * cos(5.5 * (PI / 4)), 0.5, 1 * sin(5.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // M1
  // 12
  1 * cos(5.5 * (PI / 4)), 0.5, 1 * sin(5.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // M1
  1 * cos(6 * (PI / 4)), 0, 1 * sin(6 * (PI / 4)),        0.0f, 0.0f, 1.0f,  0.0f, 1.0f,  // N
  1 * cos(6 * (PI / 4)), 0.5, 1 * sin(6 * (PI / 4)),      0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // N1
  // 13
  1 * cos(6 * (PI / 4)), 0.5, 1 * sin(6 * (PI / 4)),      0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // N1
  1 * cos(6.5 * (PI / 4)), 0, 1 * sin(6.5 * (PI / 4)),    0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // O
  1 * cos(6.5 * (PI / 4)), 0.5, 1 * sin(6.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // O1
  // 14
  1 * cos(6.5 * (PI / 4)), 0.5, 1 * sin(6.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // O1
  1 * cos(6.5 * (PI / 4)), 0, 1 * sin(6.5 * (PI / 4)),    0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // P
  1 * cos(6.5 * (PI / 4)), 0.5, 1 * sin(6.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // P1
  // 15
  1 * cos(6.5 * (PI / 4)), 0.5, 1 * sin(6.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // P1
  1 * cos(7.5 * (PI / 4)), 0, 1 * sin(7.5 * (PI / 4)),    0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // Q
  1 * cos(7.5 * (PI / 4)), 0.5, 1 * sin(7.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Q1
  // 16
  1 * cos(7.5 * (PI / 4)), 0.5, 1 * sin(7.5 * (PI / 4)),  0.0f, 0.0f, 1.0f,  0.5f, 1.0f, // Q1
  1, 0, 0,                                                0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // B
  1, 0.5, 0,                                              0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // B1

  };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}


void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
}


/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}


void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}


// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}