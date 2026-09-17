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
#include "helpers/metrics.hpp"
#include "solvers/solver.hpp"
#include "solvers/solverGPU.hpp"
#include "ui/ui.hpp"
#include <omp.h>

#include <glm/gtc/matrix_transform.hpp>

using namespace std;

int frameCount = 0;
GLuint VBO, VAO, EBO = 0;
GLuint gridVBO = 0;
GLuint posesVBO = 0;
GLuint colorsVBO = 0;
GLuint depthFBO = 0;

GLuint cumulativeDepthFBO = 0;
GLuint normalFBO = 0;
GLuint blurredFBO = 0;

GLuint depthTex = 0;
GLuint depthColorTex = 0;
GLuint cumulativeDepthTex = 0;
GLuint normalTex = 0;
GLuint blurredTex = 0;

ShaderProgram particleShader = {};
ShaderProgram cumulativeParticleShader = {};
ShaderProgram blurShader = {};
ShaderProgram normalShader = {};
ShaderProgram waterShader = {};
ShaderProgram gridShader = {};
shared_ptr<Camera> camera;
shared_ptr<App> app;
shared_ptr<Solver> solver;
shared_ptr<SolverGPU> solverGPU;
shared_ptr<UI> ui;

vector<vec3> poses = { vec3(0,0,0), vec3(0.5f, 0.f, 0.f) };
vector<vec4> colors = { vec4(1.f), vec4(1.f) };
float particleRadius = 1.5f;
int numParticle = (int)5e5;
int iterations = 1;

vec2 previousObstaclePos = vec2(0.f);
bool previousEnableObstacle = false;
bool enableObstacle = false;

bool paused = false;
bool freeView = true;

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

    particleShader.create();
    particleShader.load(GL_VERTEX_SHADER, "src/shaders/particleVert.glsl");
    particleShader.load(GL_FRAGMENT_SHADER, "src/shaders/particleFrag.glsl");
    particleShader.link();

    cumulativeParticleShader.create();
    cumulativeParticleShader.load(GL_VERTEX_SHADER, "src/shaders/particleVert.glsl");
    cumulativeParticleShader.load(GL_FRAGMENT_SHADER, "src/shaders/particleFragCumulative.glsl");
    cumulativeParticleShader.link();

    gridShader.create();
    gridShader.load(GL_VERTEX_SHADER, "src/shaders/gridVert.glsl");
    gridShader.load(GL_FRAGMENT_SHADER, "src/shaders/gridFrag.glsl");
    gridShader.link();

    blurShader.create();
    blurShader.load(GL_VERTEX_SHADER, "src/shaders/blurVert.glsl");
    blurShader.load(GL_FRAGMENT_SHADER, "src/shaders/blurFrag.glsl");
    blurShader.link();

    normalShader.create();
    normalShader.load(GL_VERTEX_SHADER, "src/shaders/normalVert.glsl");
    normalShader.load(GL_FRAGMENT_SHADER, "src/shaders/normalFrag.glsl");
    normalShader.link();

    waterShader.create();
    waterShader.load(GL_VERTEX_SHADER, "src/shaders/waterVert.glsl");
    waterShader.load(GL_FRAGMENT_SHADER, "src/shaders/waterFrag.glsl");
    waterShader.link();

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

    glGenTextures(1, &depthTex);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, app->width(), app->height(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGenTextures(1, &depthColorTex);
    glBindTexture(GL_TEXTURE_2D, depthColorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, app->width(), app->height(), 0, GL_RGBA, GL_FLOAT, nullptr); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGenTextures(1, &cumulativeDepthTex);
    glBindTexture(GL_TEXTURE_2D, cumulativeDepthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, app->width(), app->height(), 0, GL_RGBA, GL_FLOAT, nullptr); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGenTextures(1, &normalTex);
    glBindTexture(GL_TEXTURE_2D, normalTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, app->width(), app->height(), 0, GL_RGBA, GL_FLOAT, nullptr); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenTextures(1, &blurredTex);
    glBindTexture(GL_TEXTURE_2D, blurredTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, app->width(), app->height(), 0, GL_RGBA, GL_FLOAT, nullptr); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenFramebuffers(1, &depthFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, depthColorTex, 0);
    glGenFramebuffers(1, &cumulativeDepthFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, cumulativeDepthFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cumulativeDepthTex, 0);
    glGenFramebuffers(1, &normalFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, normalFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, normalTex, 0);
    glGenFramebuffers(1, &blurredFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, blurredFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurredTex, 0);

    float solverH = 2 * 2 * particleRadius;
    solver = make_shared<Solver>(numParticle, particleRadius, solverH, app->width() / solverH, app->height() / solverH, 0.03f);

    glEnable(GL_DEPTH_TEST);
    
    solverGPU = make_shared<SolverGPU>(
        numParticle, 
        particleRadius, 
        solverH, 
        app->height() / solverH, 
        app->height() / solverH, 
        app->height() * 0.5 / solverH, 
        0.05f
    );

    camera = make_shared<Camera>(0.02f, 0.25f);
    camera->resetMousePos(app->mouseX(), app->mouseY());

    UIContext ctx = { app };
    ui = make_shared<UI>(ctx);
    ui->setStatsContext({ app, solverGPU });

    cout << "Program started." << endl;
}

void render(){
    mat4 uProj = glm::perspective(radians(60.0f), (float)app->width() / app->height(), 0.1f, 20000.0f);

    particleShader.use();

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
    glUniformMatrix4fv(ShaderProgram::getVarLoc("uProj"), 1, GL_FALSE, &uProj[0][0]);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, numParticle);

    // cumulativeParticleShader.use();

    // glUniform1f(ShaderProgram::getVarLoc("particleRadius"), particleRadius);
    // glUniformMatrix4fv(ShaderProgram::getVarLoc("uView"), 1, GL_FALSE, &camera->viewMatrix()[0][0]);
    // glUniformMatrix4fv(ShaderProgram::getVarLoc("uProj"), 1, GL_FALSE, &uProj[0][0]);
    
    // glBindFramebuffer(GL_FRAMEBUFFER, cumulativeDepthFBO);
    // glClear(GL_COLOR_BUFFER_BIT);
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_ONE, GL_ONE);
    // glDisable(GL_DEPTH_TEST);
    // glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, numParticle);

    // normalShader.use();
    
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, depthColorTex);
    // glUniform1i(ShaderProgram::getVarLoc("depthTex"), 0);
    
    // glUniform2f(ShaderProgram::getVarLoc("viewport"), app->width(), app->height());
    // glUniformMatrix4fv(ShaderProgram::getVarLoc("uInvView"), 1, GL_FALSE, &inverse(camera->viewMatrix())[0][0]);
    // glUniformMatrix4fv(ShaderProgram::getVarLoc("uInvProj"), 1, GL_FALSE, &inverse(uProj)[0][0]);

    // glBindFramebuffer(GL_FRAMEBUFFER, normalFBO);
    // glDisable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glEnable(GL_DEPTH_TEST);
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // blurShader.use();

    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, depthColorTex);
    // glUniform1i(ShaderProgram::getVarLoc("depthColorTex"), 0);

    // glActiveTexture(GL_TEXTURE1);
    // glBindTexture(GL_TEXTURE_2D, normalTex);
    // glUniform1i(ShaderProgram::getVarLoc("normalTex"), 1);
    
    // glUniform2f(ShaderProgram::getVarLoc("viewport"), app->width(), app->height());

    // glUniformMatrix4fv(ShaderProgram::getVarLoc("uInvView"), 1, GL_FALSE, &inverse(camera->viewMatrix())[0][0]);
    // glUniformMatrix4fv(ShaderProgram::getVarLoc("uInvProj"), 1, GL_FALSE, &inverse(uProj)[0][0]);

    // glBindFramebuffer(GL_FRAMEBUFFER, blurredFBO);
    // glDisable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glEnable(GL_DEPTH_TEST);
    // glClear(GL_COLOR_BUFFER_BIT);
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // waterShader.use();

    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, cumulativeDepthTex);
    // glUniform1i(ShaderProgram::getVarLoc("cumulativeDepthTex"), 0);

    // glActiveTexture(GL_TEXTURE1);
    // glBindTexture(GL_TEXTURE_2D, blurredTex);
    // glUniform1i(ShaderProgram::getVarLoc("normalTex"), 1);

    // glActiveTexture(GL_TEXTURE2);
    // glBindTexture(GL_TEXTURE_2D, depthColorTex);
    // glUniform1i(ShaderProgram::getVarLoc("depthTex"), 2);
    
    // glUniformMatrix4fv(ShaderProgram::getVarLoc("uInvView"), 1, GL_FALSE, &inverse(camera->viewMatrix())[0][0]);
    // glUniformMatrix4fv(ShaderProgram::getVarLoc("uInvProj"), 1, GL_FALSE, &inverse(uProj)[0][0]);
    // glUniform3f(ShaderProgram::getVarLoc("lookDir"), camera->lookDir().x, camera->lookDir().y, camera->lookDir().z);
    // glUniform3f(ShaderProgram::getVarLoc("cameraPos"), camera->position().x, camera->position().y, camera->position().z);

    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // glClear(GL_COLOR_BUFFER_BIT);
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
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
        camera->rotate(app->mouseX(), app->mouseY());
    };

    // Hot reload shaders
    if (app->keyPressedOnce(GLFW_KEY_R, frameCount)){
        particleShader.reload();
        cumulativeParticleShader.reload();
        gridShader.reload();
        normalShader.reload();
        blurShader.reload();
        waterShader.reload();
        cout << "Shaders reloaded." << endl;
    }
    
    if (app->keyPressedOnce(GLFW_KEY_ENTER, frameCount)){
        solverGPU->reload();
    }

    if (app->keyPressedOnce(GLFW_KEY_P, frameCount)){
        paused = !paused;
    }

    if (app->keyPressedOnce(GLFW_KEY_RIGHT, frameCount)){
        if (paused){
            for (int i = 0; i < iterations; i++)
                solverGPU->updateFlip();
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
        ui->render();

        if (!paused){
            if (enableObstacle){
                vec2 obstaclePos = vec2(app->mouseX() - app->width() * 0.5f, app->height() * 0.5f - app->mouseY());
                vec2 obstacleVel = (obstaclePos - previousObstaclePos) / 0.03f;
                if (!previousEnableObstacle) obstacleVel = vec2(0.f);
                previousObstaclePos = obstaclePos;
    
                solverGPU->updateObstacle(obstaclePos, obstacleVel, 10);
            }

            for (int i = 0; i < iterations; i++)
                solverGPU->updateFlip();
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