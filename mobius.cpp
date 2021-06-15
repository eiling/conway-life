#define GLFW_INCLUDE_NONE

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <streambuf>
#include <random>
#include <cmath>

const int WIDTH = 800;
const int HEIGHT = 800;

const int STEPS = 60;
const float RADIUS = .6f;
const float THICKNESS = .8f;

const float PI = 3.1415;

int compileShader(const unsigned int shader, const char *file) {
    std::ifstream t(file);
    std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    const char *c = str.c_str();

    glShaderSource(shader, 1, &c, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    return success;
}

void printErrorInfo(const unsigned int shader) {
    char infoLog[512];
    glGetShaderInfoLog(shader, 512, nullptr, infoLog);
    std::cout << infoLog << std::endl;
}

void errorCallback(int error, const char *description) {
    std::cout << "Code: " << error << std::endl;
    std::cout << description << std::endl;
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

int main() {
    glfwSetErrorCallback(errorCallback);

    if (!glfwInit()) {
        std::cout << "GLFW initialization failed" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "conway_life", nullptr, nullptr);
    if (!window) {
        std::cout << "GLFW window creation failed" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    std::cout << "GLVersion: " << GLVersion.major << "." << GLVersion.minor << std::endl;

    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    if (!compileShader(vertexShader, "mobiusVertex.glsl")) {
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED" << std::endl;
        printErrorInfo(vertexShader);
        return -1;
    }

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    if (!compileShader(fragmentShader, "mobiusFragment.glsl")) {
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED" << std::endl;
        printErrorInfo(fragmentShader);
        return -1;
    }

    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED" << std::endl;
        std::cout << infoLog << std::endl;
        return -1;
    }

    glDetachShader(shaderProgram, vertexShader);
    glDetachShader(shaderProgram, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glClearColor(.0f, .1f, .0f, .0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    float vertices[STEPS * 6 * 5];
    for (int i = 0; i < STEPS; ++i) {
        float a0 = (float) i * PI * 2.f / (float) STEPS;
        float b0 = (float) i * PI / (float) STEPS;

        float a1 = (float) (i + 1) * PI * 2.f / (float) STEPS;
        float b1 = (float) (i + 1) * PI / (float) STEPS;

        float x0 = -std::cos(a0) * std::sin(b0) * THICKNESS / 2.f;
        float y0 = std::sin(a0) * std::sin(b0) * THICKNESS / 2.f;
        float z0 = -std::cos(b0) * THICKNESS / 2.f;

        float x2 = -std::cos(a1) * std::sin(b1) * THICKNESS / 2.f;
        float y2 = std::sin(a1) * std::sin(b1) * THICKNESS / 2.f;
        float z2 = -std::cos(b1) * THICKNESS / 2.f;

        float x1 = -x0, y1 = -y0, z1 = -z0;
        float x3 = -x2, y3 = -y2, z3 = -z2;

        x0 += std::cos(a0) * RADIUS;
        y0 += -std::sin(a0) * RADIUS;
        x1 += std::cos(a0) * RADIUS;
        y1 += -std::sin(a0) * RADIUS;
        x2 += std::cos(a1) * RADIUS;
        y2 += -std::sin(a1) * RADIUS;
        x3 += std::cos(a1) * RADIUS;
        y3 += -std::sin(a1) * RADIUS;

        float v0 = (float) i / (float) STEPS;
        float v1 = (float) (i + 1) / (float) STEPS;

        vertices[6 * 5 * i + 0 * 5 + 0] = x0;
        vertices[6 * 5 * i + 0 * 5 + 1] = y0;
        vertices[6 * 5 * i + 0 * 5 + 2] = z0;
        vertices[6 * 5 * i + 0 * 5 + 3] = 1.f;
        vertices[6 * 5 * i + 0 * 5 + 4] = v0;

        vertices[6 * 5 * i + 1 * 5 + 0] = x2;
        vertices[6 * 5 * i + 1 * 5 + 1] = y2;
        vertices[6 * 5 * i + 1 * 5 + 2] = z2;
        vertices[6 * 5 * i + 1 * 5 + 3] = 1.f;
        vertices[6 * 5 * i + 1 * 5 + 4] = v1;

        vertices[6 * 5 * i + 2 * 5 + 0] = x1;
        vertices[6 * 5 * i + 2 * 5 + 1] = y1;
        vertices[6 * 5 * i + 2 * 5 + 2] = z1;
        vertices[6 * 5 * i + 2 * 5 + 3] = .0f;
        vertices[6 * 5 * i + 2 * 5 + 4] = v0;

        vertices[6 * 5 * i + 3 * 5 + 0] = x2;
        vertices[6 * 5 * i + 3 * 5 + 1] = y2;
        vertices[6 * 5 * i + 3 * 5 + 2] = z2;
        vertices[6 * 5 * i + 3 * 5 + 3] = 1.f;
        vertices[6 * 5 * i + 3 * 5 + 4] = v1;

        vertices[6 * 5 * i + 4 * 5 + 0] = x3;
        vertices[6 * 5 * i + 4 * 5 + 1] = y3;
        vertices[6 * 5 * i + 4 * 5 + 2] = z3;
        vertices[6 * 5 * i + 4 * 5 + 3] = .0f;
        vertices[6 * 5 * i + 4 * 5 + 4] = v1;

        vertices[6 * 5 * i + 5 * 5 + 0] = x1;
        vertices[6 * 5 * i + 5 * 5 + 1] = y1;
        vertices[6 * 5 * i + 5 * 5 + 2] = z1;
        vertices[6 * 5 * i + 5 * 5 + 3] = .0f;
        vertices[6 * 5 * i + 5 * 5 + 4] = v0;
    }

    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));

    float rotx = .0f;
    float roty = .0f;
    float rotz = .0f;

    // double lastx, lasty;

    double last = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        // glfwGetCursorPos(window, &lastx, &lasty);

        // int mouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        // if (mouseState == GLFW_PRESS) {
        //     double x, y, dx, dy;
        //     glfwGetCursorPos(window, &x, &y);
        //     dx = (x - lastx) / WIDTH;
        //     dy = (y - lasty) / HEIGHT;
        //     rotx += (float) (20.0 * dy);
        //     roty -= (float) (20.0 * dx);
        // }

        double now = glfwGetTime();
        while (now > last) {
            // rotx += PI / 180.f;
            roty += PI / 180.f;
            // rotz += PI / 270.f;
            last += 1.0 / 60.0;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(vao);

        glUniform3f(2, rotx, roty, rotz);

        glDrawArrays(GL_TRIANGLES, 0, STEPS * 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shaderProgram);

    glfwTerminate();

    return 0;
}
