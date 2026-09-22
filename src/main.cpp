#include <iostream>
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "app.hpp"
#include "camera.hpp"
#include "shader_program.hpp"

#include "helpers/stats.hpp"
#include "helpers/metrics.hpp"
#include "helpers/logger.hpp"

#include "solvers/flipSolverGPU.hpp"
#include "renderers/particleRenderer3D.hpp"

#include "ui/ui.hpp"


using namespace std;
using namespace glm;

int frameCount = 0;

shared_ptr<App> app;
shared_ptr<Camera> camera;
shared_ptr<ISolver> solver;
shared_ptr<IRenderer> renderer;
shared_ptr<UI> ui;

vector<vec3> poses = { vec3(0,0,0), vec3(0.5f, 0.f, 0.f) };
float particleRadius = 1.5f;
int numParticle = (int)5e5;
int iterations = 1;

vec2 previousObstaclePos = vec2(0.f);
bool previousEnableObstacle = false;
bool enableObstacle = false;

bool paused = false;
bool freeView = false;

#ifdef _WIN32
extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

void init(){
    app = make_shared<App>();
    app->init(1280, 720, "Simulator");
    app->setClearColor(0, 0, 0, 1.0f);
    app->toggleCursor(!freeView);
    
    camera = make_shared<Camera>(app, 60.0f, 0.02f, 1.5f);
    camera->resetMousePos(app->mouseX(), app->mouseY());
    
    FlipSolverGPUConfig flipConfigGPU = FlipSolverGPUConfig();
    flipConfigGPU.partN = numParticle;
    flipConfigGPU.partRadius = particleRadius;
    flipConfigGPU.hPartRatio = 2;
    flipConfigGPU.gridX = 600 / flipConfigGPU.h();
    flipConfigGPU.gridY = 600 / flipConfigGPU.h();
    flipConfigGPU.gridZ = 300 / flipConfigGPU.h();
    flipConfigGPU.dt = 0.05f;

    solver = make_shared<FlipSolverGPU>(flipConfigGPU);
    renderer = make_shared<ParticleRenderer3D>(static_cast<const IParticleSolver&>(*solver), camera);
    
    UIContext ctx = { app, solver, renderer };
    ui = make_shared<UI>(ctx);
    ui->setStatsContext({ app, solver });
    
    Logger::logSuccess("Program started.", __LOG_DATA__);
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
        camera->move(inputs, app->dt());
        camera->rotate(app->mouseX(), app->mouseY(), app->dt());
    };

    // Hot reload shaders
    if (app->keyPressedOnce(GLFW_KEY_R, frameCount)){
        renderer->reload();
        Logger::logInfo("Shaders reloaded.", __LOG_DATA__);
    }
    
    if (app->keyPressedOnce(GLFW_KEY_ENTER, frameCount)){
        solver->reload();
        Logger::logInfo("Simulation restarted", __LOG_DATA__);
    }

    if (app->keyPressedOnce(GLFW_KEY_P, frameCount)){
        paused = !paused;
    }

    if (app->keyPressedOnce(GLFW_KEY_RIGHT, frameCount)){
        if (paused){
            for (int i = 0; i < iterations; i++)
                solver->update();
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
    renderer.reset();
    solver.reset();

    ui.reset();
    camera.reset();

    app.reset();
}

int main(){
    init();
    while(!app->shouldClose())
    {
        app->startFrame(frameCount);
        ui->render();
        
        if (!paused){
            if (enableObstacle){
                vec2 obstaclePos = vec2(app->mouseX() - app->width() * 0.5f, app->height() * 0.5f - app->mouseY());
                vec2 obstacleVel = (obstaclePos - previousObstaclePos) / 0.03f;
                if (!previousEnableObstacle) obstacleVel = vec2(0.f);
                previousObstaclePos = obstaclePos;
                
                //solver->updateObstacle(obstaclePos, obstacleVel, 10);
            }
            
            for (int i = 0; i < iterations; i++)
                solver->update();
        }
        previousEnableObstacle = enableObstacle;

        renderer->render();
        inputs();

        frameCount++;
        app->endFrame();
    }
    end();
    return EXIT_SUCCESS;
}