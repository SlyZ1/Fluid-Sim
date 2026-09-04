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
#include "solverGPU.hpp"
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
shared_ptr<SolverGPU> solverGPU;

CPUTimer frameTimer = {};
FPSCounter fpsCounter = {};

vector<vec3> poses = { vec3(0,0,0), vec3(0.5f, 0.f, 0.f) };
vector<vec4> colors = { vec4(1.f), vec4(1.f) };
float particleRadius = 2.f;
int numParticle = 600000;
int iterations = 1;

vec2 previousObstaclePos = vec2(0.f);
bool previousEnableObstacle = false;
bool enableObstacle = false;

bool paused = false;
bool freeView = true;

extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

void init(){
    app = make_shared<App>();
    app->init(1280, 720, "Default GLSL");
    app->setClearColor(0,0,0,1);
    app->toggleCursor(!freeView);

    particleShader.create();
    particleShader.load(GL_VERTEX_SHADER, "src/shaders/particleVert.glsl");
    particleShader.load(GL_FRAGMENT_SHADER, "src/shaders/particleFrag.glsl");
    particleShader.link();

    gridShader.create();
    gridShader.load(GL_VERTEX_SHADER, "src/shaders/gridVert.glsl");
    gridShader.load(GL_FRAGMENT_SHADER, "src/shaders/gridFrag.glsl");
    gridShader.link();

    // vector<float> quadVerts = {
    //     1.f,  1.f, 1.f,
    //     1.f, -1.f, 1.f,
    //     -1.f, -1.f, 1.f,
    //     -1.f,  1.f, 1.f,
    //     1.f,  1.f, -1.f,
    //     1.f, -1.f, -1.f,
    //     -1.f, -1.f, -1.f,
    //     -1.f,  1.f, -1.f
    // };
    // vector<unsigned int> quadIndices = {
    //     0, 1, 2,
    //     0, 2, 3,
    //     5, 4, 7,
    //     5, 7, 6,
    //     4, 0, 3,
    //     4, 3, 7,
    //     1, 5, 6,
    //     1, 6, 2,
    //     3, 2, 6,
    //     3, 6, 7,
    //     4, 5, 1,
    //     4, 1, 0
    // };
    vector<float> quadVerts = {
        1.f,  1.f, 0.f,
        1.f, -1.f, 0.f,
        -1.f, -1.f, 0.f,
        -1.f,  1.f, 0.f,
    };
    vector<unsigned int> quadIndices = {
        0, 1, 2,
        0, 2, 3
    };
    tie(VBO, VAO, EBO) = ShaderProgram::addData(quadVerts, quadIndices);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    float solverH = 2 * 2 * particleRadius;
    solver = make_shared<Solver>(numParticle, particleRadius, solverH, app->width() / solverH, app->height() / solverH, 0.03f);
    //poses = solver->getPos();
    //colors = vector<vec4>((int)poses.size(), vec4(1.0f));

    glGenBuffers(1, &gridVBO);
    glGenBuffers(1, &posesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, posesVBO);
    glBufferData(GL_ARRAY_BUFFER, poses.size() * sizeof(vec3), poses.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
    glVertexAttribDivisor(1, 1);
    glEnableVertexAttribArray(1);
    
    glGenBuffers(1, &colorsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, colorsVBO);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(vec4), colors.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)0);
    glVertexAttribDivisor(2, 1);
    glEnableVertexAttribArray(2);

    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_FRONT);
    
    solverGPU = make_shared<SolverGPU>(numParticle, particleRadius, solverH, app->width() / solverH, app->height() / solverH, app->height() * 0.5 / solverH, 0.03f);

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
    // glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    // glBufferData(GL_ARRAY_BUFFER, grid.size() * sizeof(vec4), grid.data(), GL_DYNAMIC_DRAW);

    // glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)0);
    // glVertexAttribDivisor(1, 1);
    // glEnableVertexAttribArray(1);

    // vector<vec4> gridColor = vector((int)grid.size(), vec4(0.2f, 0.2f, 0.2f, 1.f));
    // glBindBuffer(GL_ARRAY_BUFFER, colorsVBO);
    // glBufferData(GL_ARRAY_BUFFER, gridColor.size() * sizeof(vec4), gridColor.data(), GL_DYNAMIC_DRAW);

    // glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)0);
    // glVertexAttribDivisor(2, 1);
    // glEnableVertexAttribArray(2);
    
    // glUniform2f(ShaderProgram::getVarLoc("viewport"), (float)app->width(), (float)app->height());

    // glBindVertexArray(VAO);
    // glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, (int)grid.size());

    // glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    particleShader.use();

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, solverGPU->getPosBuffer());

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)0);
    glVertexAttribDivisor(1, 1);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, colorsVBO);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(vec4), colors.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)0);
    glVertexAttribDivisor(2, 1);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, solverGPU->getVelBuffer());

    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)0);
    glVertexAttribDivisor(3, 1);
    glEnableVertexAttribArray(3);

    glUniform1f(ShaderProgram::getVarLoc("particleRadius"), particleRadius);
    glUniformMatrix4fv(ShaderProgram::getVarLoc("uView"), 1, GL_FALSE, &camera->viewMatrix()[0][0]);
    mat4 uProj = perspective(radians(60.0f), (float)app->width() / app->height(), 0.1f, 20000.0f);
    glUniformMatrix4fv(ShaderProgram::getVarLoc("uProj"), 1, GL_FALSE, &uProj[0][0]);

    glBindVertexArray(VAO);
    
    glDisable(GL_BLEND);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, numParticle);
    //glDepthMask(GL_TRUE);
}

void updatePosesAndColors(){
    //poses = solver->getPos();
    colors.clear();
    colors = vector<vec4>((int)poses.size(), vec4(0.f));
    vector<vec2> vels = solver->getVel();
    for (int i = 0; i < (int)poses.size(); i++)
    {
        colors[i] = glm::mix(vec4(0,0,1,1), vec4(1), length(vels[i]) / 70.f);
    }
}

void inputs(){
    if (app->keyPressedOnce(GLFW_KEY_ESCAPE, frameCount)){
        freeView = !freeView;
        app->toggleCursor(!freeView);
        if (!freeView){
            camera->hasStoppedMoving();
        } else {
            camera->resetMousePos(app->mouseX(), app->mouseY());
        }
    }

    if (freeView){
        CameraMoveInputs inputs = {
            app->keyPressed(GLFW_KEY_W), 
            app->keyPressed(GLFW_KEY_S), 
            app->keyPressed(GLFW_KEY_D), 
            app->keyPressed(GLFW_KEY_A),
            app->keyPressed(GLFW_KEY_SPACE),
            app->keyPressed(GLFW_KEY_LEFT_CONTROL),
            app->keyPressed(GLFW_KEY_LEFT_SHIFT),
            app->keyPressed(GLFW_KEY_C)
        };
        camera->move(inputs, stats->frameTime);
        camera->rotate(app->mouseX(), app->mouseY());
    };

    // Hot reload shaders
    if (app->keyPressedOnce(GLFW_KEY_R, frameCount)){
        particleShader.reload();
        gridShader.reload();
        solverGPU->reload();
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
        //solverGPU->setDt(stats->frameTime * 0.001f * 5);

        if (!paused){
            if (enableObstacle){
                vec2 obstaclePos = vec2(app->mouseX() - app->width() * 0.5f, app->height() * 0.5f - app->mouseY());
                vec2 obstacleVel = (obstaclePos - previousObstaclePos) / 0.03f;
                if (!previousEnableObstacle) obstacleVel = vec2(0.f);
                previousObstaclePos = obstaclePos;
    
                solverGPU->updateObstacle(obstaclePos, obstacleVel, 10);
            }

            for (int i = 0; i < iterations; i++)
            {
                solverGPU->updateFlip();
            }
            //updatePosesAndColors();
        }
        previousEnableObstacle = enableObstacle;
        if (frameCount % 50 == 0) solverGPU->printTimers();

        render();
        inputs();

        frameCount++;
        app->endFrame();
    }
    end();
    return EXIT_SUCCESS;
}