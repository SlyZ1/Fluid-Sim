#include <iostream>
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "app.hpp"
#include "camera.hpp"
#include "shader_program.hpp"
#include "helpers/stats.hpp"
#include "solver.hpp"
#include <omp.h>

using namespace std;

int frameCount = 0;
GLuint VBO, VAO, EBO;
GLuint gridVBO;
GLuint posesVBO;
GLuint colorsVBO;
ShaderProgram particleShader;
ShaderProgram gridShader;
shared_ptr<Camera> camera;
shared_ptr<App> app;
shared_ptr<Stats> stats;
shared_ptr<Solver> solver;

CPUTimer frameTimer = {};
FPSCounter fpsCounter = {};

vector<vec2> poses = { vec2(0.f, 0.f), vec2(0.5f, 0.f) };
vector<vec4> colors = { vec4(1.f), vec4(1.f) };
float particleRadius = 2.f;
int iterations = 1;

vec2 previousObstaclePos = vec2(0.f);
bool previousEnableObstacle = false;
bool enableObstacle = false;

bool paused = false;

extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

void init(){
    app = make_shared<App>();
    app->init(1280, 720, "Default GLSL");
    app->setClearColor(0,0,0,1);

    particleShader.create();
    particleShader.load(GL_VERTEX_SHADER, "src/shaders/particleVert.glsl");
    particleShader.load(GL_FRAGMENT_SHADER, "src/shaders/particleFrag.glsl");
    particleShader.link();

    gridShader.create();
    gridShader.load(GL_VERTEX_SHADER, "src/shaders/gridVert.glsl");
    gridShader.load(GL_FRAGMENT_SHADER, "src/shaders/gridFrag.glsl");
    gridShader.link();

    vector<float> quadVerts = {
        1.f,  1.f,
        1.f, -1.f,
        -1.f, -1.f,
        -1.f,  1.f
    };
    vector<unsigned int> quadIndices = {
        0, 1, 3,
        1, 2, 3
    };  
    tie(VBO, VAO, EBO) = ShaderProgram::addData(quadVerts, quadIndices);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    float solverH = 2 * 1.2f * particleRadius;
    solver = make_shared<Solver>(15000, particleRadius, solverH, app->width() / solverH, app->height() / solverH, 0.05f);
    poses = solver->getPos();
    colors = vector<vec4>((int)poses.size(), vec4(1.0f));

    glGenBuffers(1, &gridVBO);
    glGenBuffers(1, &posesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, posesVBO);
    glBufferData(GL_ARRAY_BUFFER, poses.size() * sizeof(vec2), poses.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);
    glVertexAttribDivisor(1, 1);
    glEnableVertexAttribArray(1);
    
    glGenBuffers(1, &colorsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, colorsVBO);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(vec4), colors.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)0);
    glVertexAttribDivisor(2, 1);
    glEnableVertexAttribArray(2);

    camera = make_shared<Camera>(0.02f, 0.25f);
    camera->resetMousePos(app->mouseX(), app->mouseY());
    
    stats = make_shared<Stats>();
    frameTimer.begin();
}

void recordStats(){
    fpsCounter.update();
    frameTimer.end();
    frameTimer.begin();

    stats->frameTime = frameTimer.get();
    stats->fps = fpsCounter.get();

    if (frameCount % 100 != 0) return;
    cout << setprecision(2) << stats->frameTime << "ms\t" << stats->fps << " fps" << endl; 
}

void render(){
    // gridShader.use();

    // glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    // glEnableVertexAttribArray(0);

    // vector<vec4> grid = solver->getGrid(1.f);
    // vector<vec4> cells = solver->getCells();
    // vector<vec4> finalPoses = Utils::concat<vec4>({cells, grid});
    // glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    // glBufferData(GL_ARRAY_BUFFER, finalPoses.size() * sizeof(vec4), finalPoses.data(), GL_DYNAMIC_DRAW);

    // glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)0);
    // glVertexAttribDivisor(1, 1);
    // glEnableVertexAttribArray(1);

    // vector<vec4> gridColor = vector((int)grid.size(), vec4(0.2f, 0.2f, 0.2f, 1.f));
    // vector<vec4> cellColors = solver->getCellColors();
    // vector<vec4> finalColors = Utils::concat<vec4>({cellColors, gridColor});
    // glBindBuffer(GL_ARRAY_BUFFER, colorsVBO);
    // glBufferData(GL_ARRAY_BUFFER, finalColors.size() * sizeof(vec4), finalColors.data(), GL_DYNAMIC_DRAW);

    // glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)0);
    // glVertexAttribDivisor(2, 1);
    // glEnableVertexAttribArray(2);
    
    // glUniform2f(ShaderProgram::getVarLoc("viewport"), (float)app->width(), (float)app->height());

    // glBindVertexArray(VAO);
    // glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, (int)finalPoses.size());

    particleShader.use();

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, posesVBO);
    glBufferData(GL_ARRAY_BUFFER, poses.size() * sizeof(vec2), poses.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);
    glVertexAttribDivisor(1, 1);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, colorsVBO);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(vec4), colors.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)0);
    glVertexAttribDivisor(2, 1);
    glEnableVertexAttribArray(2);

    glUniform2f(ShaderProgram::getVarLoc("viewport"), (float)app->width(), (float)app->height());
    glUniform1f(ShaderProgram::getVarLoc("particleRadius"), particleRadius);

    glBindVertexArray(VAO);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, (int)poses.size());
}

void updatePosesAndColors(){
    poses = solver->getPos();
    colors.clear();
    colors = vector<vec4>((int)poses.size(), vec4(0.f));
    vector<vec2> vels = solver->getVel();
    for (int i = 0; i < (int)poses.size(); i++)
    {
        colors[i] = glm::mix(vec4(0,0,1,1), vec4(1), length(vels[i]) / 70.f);
    }
}

void inputs(){
    // Hot reload shaders
    if (app->keyPressedOnce(GLFW_KEY_R, frameCount)){
        particleShader.reload();
        gridShader.reload();
        cout << "Shaders reloaded." << endl;
    }

    if (app->keyPressedOnce(GLFW_KEY_P, frameCount)){
        paused = !paused;
    }

    if (app->keyPressedOnce(GLFW_KEY_RIGHT, frameCount)){
        if (paused){
            for (int i = 0; i < iterations; i++)
            {
                solver->updateFlip();
            }
            updatePosesAndColors();
        }
    }
    if (app->keyPressed(GLFW_MOUSE_BUTTON_LEFT)){
        enableObstacle = true;
    }
    else{
        enableObstacle = false;
    }
}

void end(){
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    particleShader.destroy();
    gridShader.destroy();
    app->terminate();
}

int main(){
    init();
    while(!app->shouldClose())
    {
        app->startFrame(frameCount);
        recordStats();

        if (!paused){
            if (enableObstacle){
                vec2 obstaclePos = vec2(app->mouseX() - app->width() * 0.5f, app->height() * 0.5f - app->mouseY());
                vec2 obstacleVel = (obstaclePos - previousObstaclePos) * (float)stats->fps / 10.f;
                if (!previousEnableObstacle) obstacleVel = vec2(0.f);
                previousObstaclePos = obstaclePos;
    
                solver->updateObstacle(obstaclePos, obstacleVel, 10);
            }

            for (int i = 0; i < iterations; i++)
            {
                solver->updateFlip();
            }
            updatePosesAndColors();
        }
        previousEnableObstacle = enableObstacle;

        render();
        inputs();

        frameCount++;
        app->endFrame();
    }
    end();
    return EXIT_SUCCESS;
}