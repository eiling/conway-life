#define GLFW_INCLUDE_NONE

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <streambuf>
#include <random>
#include <cmath>

struct st_shaderInfo {
    unsigned int type;
    const char *file;
} allShaders[] = {{GL_VERTEX_SHADER,   "mobiusVertex.glsl"},
                  {GL_FRAGMENT_SHADER, "fragment.glsl"},
                  {GL_COMPUTE_SHADER,  "mobiusEdgesCompute.glsl"},
                  {GL_COMPUTE_SHADER,  "lifeCompute.glsl"},
                  {GL_COMPUTE_SHADER,  "texCompute.glsl"}};

const int WIDTH = 800;
const int HEIGHT = 800;

const int BOARD_HEIGHT = 100;
const int BOARD_WIDTH = BOARD_HEIGHT * 5;

const int TEX_SCALE = 32;  // matches local group size of texture compute shader

const int STEPS = 600;
const float RADIUS = .6f;
const float THICKNESS = .8f;

const float PI = 3.1415f;

const float PERIOD = 1.f / 30.f;
// const float PERIOD = .5f;

float getRandom() {
    static std::random_device rd;
    static std::default_random_engine e(rd());
    static std::uniform_int_distribution<> d(0, 1);
    return (float) d(e);
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void errorCallback(int error, const char *description) {
    std::cout << "Code: " << error << std::endl;
    std::cout << description << std::endl;
}

int createShader(unsigned int *shader, unsigned int type, const char *file) {
    *shader = glCreateShader(type);

    std::ifstream t(file);
    std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    const char *c = str.c_str();

    glShaderSource(*shader, 1, &c, nullptr);
    glCompileShader(*shader);

    int success;
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(*shader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::" << type << "::COMPILATION_FAILED" << std::endl;
        std::cout << infoLog << std::endl;
    }
    return success;
}

int createAndLinkProgram(unsigned int *program, st_shaderInfo *shaders, int shaderCount) {
    unsigned int shaderIds[shaderCount];
    int success, shaderStatus = createShader(shaderIds, shaders[0].type, shaders[0].file);
    for (int i = 1; shaderStatus && i < shaderCount; ++i) {
        shaderStatus = createShader(shaderIds + i, shaders[i].type, shaders[i].file);
    }

    if (shaderStatus) {
        *program = glCreateProgram();
        for (int i = 0; i < shaderCount; ++i) {
            glAttachShader(*program, shaderIds[i]);
        }
        glLinkProgram(*program);

        glGetProgramiv(*program, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(*program, 512, nullptr, infoLog);
            std::cout << "ERROR::PROGRAM::LINKING_FAILED" << std::endl;
            std::cout << infoLog << std::endl;
        }
    }

    for (int i = 0; i < shaderCount; ++i) {
        glDetachShader(*program, shaderIds[i]);
        glDeleteShader(shaderIds[i]);
    }

    return shaderStatus && success;
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
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

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

    unsigned int mainProgram, edgesComputeProgram, lifeComputeProgram, textureComputeProgram;
    if (createAndLinkProgram(&mainProgram, allShaders, 2) &&
        createAndLinkProgram(&edgesComputeProgram, allShaders + 2, 1) &&
        createAndLinkProgram(&lifeComputeProgram, allShaders + 3, 1) &&
        createAndLinkProgram(&textureComputeProgram, allShaders + 4, 1)) {
        glClearColor(1.f, 1.f, 1.f, 1.f);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glEnable(GL_DEBUG_OUTPUT);

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

        unsigned int vao, vbo, params_ssbo, board1_ssbo, board2_ssbo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &params_ssbo);
        glGenBuffers(1, &board1_ssbo);
        glGenBuffers(1, &board2_ssbo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) nullptr);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));

        const int params[] = {
                BOARD_WIDTH,
                BOARD_HEIGHT,
                // neighborIndices:
                -(BOARD_WIDTH + 2) - 1,  // BOTTOM LEFT
                -(BOARD_WIDTH + 2),  // BOTTOM
                -(BOARD_WIDTH + 2) + 1,  // BOTTOM RIGHT
                -1,  // LEFT
                // SKIP CENTER
                +1,  // RIGHT
                +(BOARD_WIDTH + 2) - 1,  // UPPER LEFT
                +(BOARD_WIDTH + 2),  // UP
                +(BOARD_WIDTH + 2) + 1  // UPPER RIGHT
        };
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, params_ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, params_ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(params), params, GL_DYNAMIC_COPY);

        float board[(BOARD_WIDTH + 2) * (BOARD_HEIGHT + 2)];
        bool boardFlag = true;
        for (int i = 0; i < BOARD_WIDTH; ++i) {
            for (int j = 0; j < BOARD_HEIGHT; ++j) {
                board[i + 1 + (j + 1) * (BOARD_WIDTH + 2)] = getRandom();
            }
        }
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, board1_ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(board), board, GL_DYNAMIC_COPY);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, board2_ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(board), nullptr,
                     GL_DYNAMIC_COPY);  // set size for the second buffer??

        unsigned int tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, BOARD_WIDTH * TEX_SCALE, BOARD_HEIGHT * TEX_SCALE, 0, GL_RGBA,
                     GL_FLOAT,
                     nullptr);
        glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

        float rotx = .0f;
        float roty = PI * 2.f / 3.f;
        float rotz = .0f;

        int gen = 0;
        double referenceTime = glfwGetTime();
        while (!glfwWindowShouldClose(window)) {
            processInput(window);

            double now = glfwGetTime();
            while (now - referenceTime > PERIOD) {
                referenceTime += PERIOD;
                std::cout << "Generation: " << ++gen << std::endl;

                boardFlag = !boardFlag;
                if (boardFlag) {
                    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, board1_ssbo);
                    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, board2_ssbo);
                } else {
                    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, board2_ssbo);
                    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, board1_ssbo);
                }

                glUseProgram(edgesComputeProgram);
                glDispatchCompute(BOARD_HEIGHT, 1, 1);

                glMemoryBarrier(GL_ALL_BARRIER_BITS);

                glUseProgram(lifeComputeProgram);
                glDispatchCompute(BOARD_WIDTH, BOARD_HEIGHT, 1);

                glMemoryBarrier(GL_ALL_BARRIER_BITS);

                glUseProgram(textureComputeProgram);
                glDispatchCompute(BOARD_WIDTH, BOARD_HEIGHT, 1);

                glMemoryBarrier(GL_ALL_BARRIER_BITS);

                glGenerateMipmap(GL_TEXTURE_2D);
            }

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glUseProgram(mainProgram);
            glBindVertexArray(vao);

            glUniform3f(2, rotx, roty, rotz);

            glDrawArrays(GL_TRIANGLES, 0, STEPS * 6);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    glDeleteProgram(mainProgram);
    glDeleteProgram(edgesComputeProgram);
    glDeleteProgram(lifeComputeProgram);
    glDeleteProgram(textureComputeProgram);

    glfwTerminate();

    return 0;
}
