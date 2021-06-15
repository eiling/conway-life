#define GLFW_INCLUDE_NONE

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <streambuf>
#include <random>

const int WIDTH = 800;
const int HEIGHT = 800;
const int BOARD_CELL_SIZE = 2;
const int BOARD_WIDTH = WIDTH / BOARD_CELL_SIZE;
const int BOARD_HEIGHT = HEIGHT / BOARD_CELL_SIZE;

const float PERIOD = .005f;

float getRandom() {
    static std::random_device rd;
    static std::default_random_engine e(rd());
    static std::uniform_int_distribution<> d(0, 1);
    return (float) d(e);
}

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
    if (!compileShader(vertexShader, "vertex.glsl")) {
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED" << std::endl;
        printErrorInfo(vertexShader);
        return -1;
    }

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    if (!compileShader(fragmentShader, "fragment.glsl")) {
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

    float vertices[] = {
            -1.f, 1.f,
            -1.f, -1.f,
            1.f, 1.f,
            1.f, 1.f,
            -1.f, -1.f,
            1.f, -1.f,
    };

    float steps[] = {
            20.f,
            1.f,
            20.f,
            20.f,
            1.f,
            1.f,
    };

    unsigned int vao, vbo, ssbo, params_ssbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ssbo);
    glGenBuffers(1, &params_ssbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(steps), vertices, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(steps), steps);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void *) sizeof(vertices));

    float board1[(BOARD_WIDTH + 2) * (BOARD_HEIGHT + 2)];
    float board2[(BOARD_WIDTH + 2) * (BOARD_HEIGHT + 2)];
    float *board = board1;

    const int neighborIndices[] = {
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

    for (int i = 0; i < BOARD_WIDTH; ++i) {
        for (int j = 0; j < BOARD_HEIGHT; ++j) {
            board[i + 1 + (j + 1) * (BOARD_WIDTH + 2)] = getRandom();
        }
    }

    const int params[] = {WIDTH, HEIGHT, BOARD_WIDTH, BOARD_HEIGHT};

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, params_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 4 * sizeof(int), params, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, params_ssbo);

    int gen = 0;
    double referenceTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        if (glfwGetTime() - referenceTime > PERIOD) {
            referenceTime += PERIOD;
            std::cout << "Generation: " << ++gen << std::endl;
            float *old = board;
            board = board == board1 ? board2 : board1;
            for (int i = 0; i < BOARD_WIDTH; ++i) {
                for (int j = 0; j < BOARD_HEIGHT; ++j) {
                    int index = i + 1 + (j + 1) * (BOARD_WIDTH + 2);
                    float sum = .0f;
                    for (int neighborIndex : neighborIndices) {
                        sum += old[index + neighborIndex];
                    }
                    if (old[index] > .5f) {
                        if (sum < 2.f || sum >= 4.f) {
                            board[index] = .0f;
                        } else {
                            board[index] = 1.f;
                        }
                    } else {
                        if (sum >= 3.f && sum < 4.f) {
                            board[index] = 1.f;
                        } else {
                            board[index] = .0f;
                        }
                    }
                }
            }
        }

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(vao);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(board1), board, GL_DYNAMIC_COPY);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shaderProgram);

    glfwTerminate();

    return 0;
}
