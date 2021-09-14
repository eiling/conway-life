#define GLFW_INCLUDE_NONE

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <streambuf>
#include <random>

struct st_shaderInfo {
    unsigned int type;
    const char *file;
} allShaders[] = {{GL_VERTEX_SHADER,   "vertex.glsl"},
                  {GL_FRAGMENT_SHADER, "fragment.glsl"},
                  {GL_COMPUTE_SHADER,  "lifeCompute.glsl"},
                  {GL_COMPUTE_SHADER,  "texCompute.glsl"}};

const int WIDTH = 800;
const int HEIGHT = 800;

const int BOARD_HEIGHT = 800;
const int BOARD_WIDTH = BOARD_HEIGHT;

const int TEX_SCALE = 1;  // matches local group size of texture compute shader

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

    unsigned int mainProgram, lifeComputeProgram, textureComputeProgram;
    if (createAndLinkProgram(&mainProgram, allShaders, 2) &&
        createAndLinkProgram(&lifeComputeProgram, allShaders + 2, 1) &&
        createAndLinkProgram(&textureComputeProgram, allShaders + 3, 1)) {
        glClearColor(1.f, 1.f, 1.f, 1.f);

        glEnable(GL_DEBUG_OUTPUT);

        float vertices[6 * 4] = {
                -1, 1, 0, 1,
                -1, -1, 0, 0,
                1, -1, 1, 0,
                -1, 1, 0, 1,
                1, -1, 1, 0,
                1, 1, 1, 1
        };

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
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) nullptr);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) (2 * sizeof(float)));

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

        const int boardSize = (BOARD_WIDTH + 2) * (BOARD_HEIGHT + 2);
        auto *board = (float *) (calloc(boardSize, sizeof(float)));
        for (int i = 0; i < BOARD_WIDTH; ++i) {
            for (int j = 0; j < BOARD_HEIGHT; ++j) {
                board[i + 1 + (j + 1) * (BOARD_WIDTH + 2)] = getRandom();
            }
        }
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, board1_ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, board1_ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, boardSize * sizeof(float), board, GL_STATIC_COPY);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, board2_ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, board2_ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, boardSize * sizeof(float), nullptr, GL_STATIC_COPY);
        free(board);

        unsigned int tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, BOARD_WIDTH * TEX_SCALE, BOARD_HEIGHT * TEX_SCALE, 0, GL_RGBA,
                     GL_FLOAT,
                     nullptr);
        glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

        int gen = 0;
        double referenceTime = glfwGetTime();
        while (!glfwWindowShouldClose(window)) {
            processInput(window);

            double now = glfwGetTime();
            while (now - referenceTime > PERIOD) {
                referenceTime += PERIOD;
                std::cout << "Generation: " << ++gen << std::endl;

                unsigned int temp = board1_ssbo;
                board1_ssbo = board2_ssbo;
                board2_ssbo = temp;
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, board1_ssbo);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, board2_ssbo);

                glUseProgram(lifeComputeProgram);
                glDispatchCompute(BOARD_WIDTH, BOARD_HEIGHT, 1);
                glMemoryBarrier(GL_ALL_BARRIER_BITS);
            }

            glUseProgram(textureComputeProgram);
            glDispatchCompute(BOARD_WIDTH, BOARD_HEIGHT, 1);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            glGenerateMipmap(GL_TEXTURE_2D);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glUseProgram(mainProgram);
            glBindVertexArray(vao);

            glDrawArrays(GL_TRIANGLES, 0, 6);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    glDeleteProgram(mainProgram);
    glDeleteProgram(lifeComputeProgram);
    glDeleteProgram(textureComputeProgram);

    glfwTerminate();

    return 0;
}
