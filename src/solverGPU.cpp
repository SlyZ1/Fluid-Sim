#include "solverGPU.hpp"

SolverGPU::SolverGPU(int partN, float radius, float h, int gridX, int gridY, float timestep)
: partN(partN), radius(radius), h(h), gridX(gridX), gridY(gridY), dt(timestep) {
    vector<float> velX = vector<float>((gridX + 1) * gridY, 0.f);
    vector<float> velY = vector<float>(gridX * (gridY + 1), 0.f);
    vector<float> rX = vector<float>((gridX + 1) * gridY, 0.f);
    vector<float> rY = vector<float>(gridX * (gridY + 1), 0.f);
    vector<vec2> partVel = vector<vec2>(partN, vec2(0.f));
    vector<vec2> partPos = vector<vec2>(partN, vec2(0.f));
    int a = (int)glm::floor(sqrt(partN));
    for (int i = 0; i < partN; i++)
    {
        float x = (float)(i % a);
        int y = (int)((i - x) / a);
        x += (y % 2) * 0.5f;
        partPos[i] = (vec2(x - 10,y) - vec2(a * 0.5f)) * 2.f * radius * 1.f;
    }

    glGenBuffers(1, &rXBuffer); glGenBuffers(1, &rYBuffer);
    glGenBuffers(1, &velXBuffer); glGenBuffers(1, &velYBuffer); 
    glGenBuffers(1, &partPosBuffer); glGenBuffers(1, &partVelBuffer); 

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, rXBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, rX.size() * sizeof(float), rX.data(), GL_DYNAMIC_DRAW);
     
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, rYBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, rY.size() * sizeof(float), rY.data(), GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velXBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, velX.size() * sizeof(float), velX.data(), GL_DYNAMIC_DRAW);
     
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velYBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, velY.size() * sizeof(float), velY.data(), GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, partPosBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, partPos.size() * sizeof(vec2), partPos.data(), GL_DYNAMIC_DRAW);
     
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, partVelBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, partVel.size() * sizeof(vec2), partVel.data(), GL_DYNAMIC_DRAW);

    integrateShader.create();
    integrateShader.load(GL_COMPUTE_SHADER, "src/shaders/FLIP/integrate.glsl");
    integrateShader.link();

    collisionShader.create();
    collisionShader.load(GL_COMPUTE_SHADER, "src/shaders/FLIP/collisions.glsl");
    collisionShader.link();

    glGenBuffers(1, &blockSumBuffer); glGenBuffers(1, &cellOfBuffer);
    glGenBuffers(1, &firstCellParticleBuffer); glGenBuffers(1, &cellParticleIdsBuffer);
    int ceiledN = (int)glm::ceil((float)(gridX * gridY + 1) / 512.f) * 512;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, blockSumBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, ceiledN / 512 * sizeof(uint), nullptr, GL_DYNAMIC_DRAW);
     
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellOfBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, partN * sizeof(uint), nullptr, GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, firstCellParticleBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, ceiledN * sizeof(uint), nullptr, GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellParticleIdsBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, partN * sizeof(uint), nullptr, GL_DYNAMIC_DRAW);

    resetUintBuffersShader.create();
    resetUintBuffersShader.load(GL_COMPUTE_SHADER, "src/shaders/FLIP/prefixsum/resetuintbuffer.glsl");
    resetUintBuffersShader.link();

    partCountShader.create();
    partCountShader.load(GL_COMPUTE_SHADER, "src/shaders/FLIP/prefixsum/partcount.glsl");
    partCountShader.link();

    localSumShader.create();
    localSumShader.load(GL_COMPUTE_SHADER, "src/shaders/FLIP/prefixsum/localsum.glsl");
    localSumShader.link();

    smallSumShader.create();
    smallSumShader.load(GL_COMPUTE_SHADER, "src/shaders/FLIP/prefixsum/smallsum.glsl");
    smallSumShader.link();

    globalSumShader.create();
    globalSumShader.load(GL_COMPUTE_SHADER, "src/shaders/FLIP/prefixsum/globalsum.glsl");
    globalSumShader.link();

    resetBuffersShader.create();
    resetBuffersShader.load(GL_COMPUTE_SHADER, "src/shaders/FLIP/pushappart/resetbuffers.glsl");
    resetBuffersShader.link();

    getCorrectionsShader.create();
    getCorrectionsShader.load(GL_COMPUTE_SHADER, "src/shaders/FLIP/pushappart/getcorrections.glsl");
    getCorrectionsShader.link();

    applyCorrectionsShader.create();
    applyCorrectionsShader.load(GL_COMPUTE_SHADER, "src/shaders/FLIP/pushappart/applycorrections.glsl");
    applyCorrectionsShader.link();
}

ivec2 SolverGPU::cellToCoord(int cell, int nx){
    int column = cell % nx;
    int row = (cell - column) / nx;
    return ivec2(column, row);
}

vec2 SolverGPU::cellToPos(int cell, int nx, int ny){
    ivec2 coord = cellToCoord(cell, nx);
    vec2 pos = ((vec2)coord - vec2(nx - 1, ny - 1) * 0.5f) * h;
    return pos;
}

void SolverGPU::integrateParticles(){
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    integrateShader.use();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, partPosBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, partVelBuffer);

    glUniform1f(ShaderProgram::getVarLoc("dt"), dt);
    glUniform1i(ShaderProgram::getVarLoc("partN"), partN);

    integrateShader.dispatch((partN + 255) / 256);
}

void SolverGPU::resetUintBuffer(GLuint buffer, int n){
    resetUintBuffersShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer);
    glUniform1i(ShaderProgram::getVarLoc("n"), n);
    resetUintBuffersShader.dispatch((n + 255) / 256);
}

void SolverGPU::prefixSum(GLuint data, GLuint blockSum, int n){
    localSumShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, blockSum);
    localSumShader.dispatch(n / 2);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    smallSumShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, blockSum);
    glUniform1i(ShaderProgram::getVarLoc("length"), n / 512);
    localSumShader.dispatch(1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    globalSumShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, blockSum);
    glUniform1i(ShaderProgram::getVarLoc("length"), n);
    localSumShader.dispatch(n);
}

void SolverGPU::pushAppartParticles(){
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // A enlever

    int ceiledN = (int)glm::ceil((float)(gridX * gridY + 1) / 512.f) * 512;
    resetUintBuffer(firstCellParticleBuffer, ceiledN);
    resetUintBuffer(blockSumBuffer, ceiledN / 512);
    resetUintBuffer(cellParticleIdsBuffer, partN);
    resetUintBuffer(cellOfBuffer, partN);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    partCountShader.use();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, firstCellParticleBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cellOfBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, partPosBuffer);

    glUniform1i(ShaderProgram::getVarLoc("partN"), partN);
    glUniform1i(ShaderProgram::getVarLoc("gridX"), gridX);
    glUniform1i(ShaderProgram::getVarLoc("gridY"), gridY);
    glUniform1f(ShaderProgram::getVarLoc("h"), h);

    partCountShader.dispatch((partN + 63) / 64);
    
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    prefixSum(firstCellParticleBuffer, firstCellParticleBuffer, gridX * gridY);
    
    const float minDist = 2.0f * radius;
    const float minDist2 = minDist * minDist;

}

void SolverGPU::particleCollisions(){
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    collisionShader.use();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, partPosBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, partVelBuffer);

    vec2 minPos = cellToPos(0, gridX, gridY);
    vec2 maxPos = cellToPos(gridX * gridY - 1, gridX, gridY);

    glUniform1i(ShaderProgram::getVarLoc("partN"), partN);
    glUniform2f(ShaderProgram::getVarLoc("minPos"), minPos.x, minPos.y);
    glUniform2f(ShaderProgram::getVarLoc("maxPos"), maxPos.x, maxPos.y);
    glUniform1f(ShaderProgram::getVarLoc("radius"), radius);
    glUniform2f(ShaderProgram::getVarLoc("obstaclePos"), obstaclePos.x, obstaclePos.y);
    glUniform2f(ShaderProgram::getVarLoc("obstacleVel"), obstacleVel.x, obstacleVel.y);
    glUniform1f(ShaderProgram::getVarLoc("obstacleRadius"), obstacleRadius * 10);

    collisionShader.dispatch((partN + 255) / 256);
}

void SolverGPU::particlesToGrid(){
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

}

void SolverGPU::solveIncompressibility(int iterations){
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

}

void SolverGPU::gridToParticles(){
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

}

void SolverGPU::updateFlip(){
    integrateParticles();
    particleCollisions();
    pushAppartParticles();
    particleCollisions();
    
    particlesToGrid();
    solveIncompressibility(80);
    gridToParticles();
}

void SolverGPU::updateObstacle(vec2 pos, vec2 vel, float rad){
    obstaclePos = pos;
    obstacleVel = vel;
    obstacleRadius = rad;
}