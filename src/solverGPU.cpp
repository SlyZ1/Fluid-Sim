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
    glGenBuffers(1, &oldVelXBuffer); glGenBuffers(1, &oldVelYBuffer); 
    glGenBuffers(1, &partPosBuffer); glGenBuffers(1, &partVelBuffer); 
    glGenBuffers(1, &isAirBuffer);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, rXBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, rX.size() * sizeof(float), rX.data(), GL_DYNAMIC_DRAW);
     
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, rYBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, rY.size() * sizeof(float), rY.data(), GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velXBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, velX.size() * sizeof(float), velX.data(), GL_DYNAMIC_DRAW);
     
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velYBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, velY.size() * sizeof(float), velY.data(), GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, oldVelXBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, velX.size() * sizeof(float), velX.data(), GL_DYNAMIC_DRAW);
     
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, oldVelYBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, velY.size() * sizeof(float), velY.data(), GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, partPosBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, partPos.size() * sizeof(vec2), partPos.data(), GL_DYNAMIC_DRAW);
     
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, partVelBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, partVel.size() * sizeof(vec2), partVel.data(), GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, isAirBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, gridX * gridY * sizeof(uint), nullptr, GL_DYNAMIC_DRAW);

    integrateShader.create();
    integrateShader.load(GL_COMPUTE_SHADER, "src/shaders/FLIP/integrate.glsl");
    integrateShader.link();

    collisionShader.create();
    collisionShader.load(GL_COMPUTE_SHADER, "src/shaders/FLIP/collisions.glsl");
    collisionShader.link();

    glGenBuffers(1, &blockSumBuffer); glGenBuffers(1, &cellOfBuffer);
    glGenBuffers(1, &firstCellParticleBuffer); glGenBuffers(1, &cellParticleIdsBuffer);
    glGenBuffers(1, &firstCellParticleBuffer2);
    int ceiledN = (int)glm::ceil((float)(gridX * gridY + 1) / 512.f) * 512;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, blockSumBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, ceiledN / 512 * sizeof(uint), nullptr, GL_DYNAMIC_DRAW);
     
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellOfBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, partN * sizeof(uint), nullptr, GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, firstCellParticleBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, ceiledN * sizeof(uint), nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, firstCellParticleBuffer2);
    glBufferData(GL_SHADER_STORAGE_BUFFER, ceiledN * sizeof(uint), nullptr, GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellParticleIdsBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, partN * sizeof(uint), nullptr, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &correctionBuffer); glGenBuffers(1, &numCorrectionBuffer);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, correctionBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, partN * sizeof(vec2), nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, numCorrectionBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, partN * sizeof(int), nullptr, GL_DYNAMIC_DRAW);

    resetUintBuffersShader.create();
    resetUintBuffersShader.load(GL_COMPUTE_SHADER, "src/shaders/FLIP/prefixsum/resetuintbuffer.glsl");
    resetUintBuffersShader.link();

    partCountShader.create();
    partCountShader.load(GL_COMPUTE_SHADER, "src/shaders/FLIP/pushappart/partcount.glsl");
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

    cellParticleIdShader.create();
    cellParticleIdShader.load(GL_COMPUTE_SHADER, "src/shaders/FLIP/pushappart/cellparticleid.glsl");
    cellParticleIdShader.link();

    resetBuffersShader.create();
    resetBuffersShader.load(GL_COMPUTE_SHADER, "src/shaders/FLIP/pushappart/resetbuffers.glsl");
    resetBuffersShader.link();

    getCorrectionsShader.create();
    getCorrectionsShader.load(GL_COMPUTE_SHADER, "src/shaders/FLIP/pushappart/getcorrections.glsl");
    getCorrectionsShader.link();

    applyCorrectionsShader.create();
    applyCorrectionsShader.load(GL_COMPUTE_SHADER, "src/shaders/FLIP/pushappart/applycorrections.glsl");
    applyCorrectionsShader.link();

    p2gShader.create();
    p2gShader.load(GL_COMPUTE_SHADER, "src/shaders/FLIP/p2g/p2g.glsl");
    p2gShader.link();

    resetFloatBufferShader.create();
    resetFloatBufferShader.load(GL_COMPUTE_SHADER, "src/shaders/FLIP/p2g/resetfloatbuffer.glsl");
    resetFloatBufferShader.link();

    applyWeightsShader.create();
    applyWeightsShader.load(GL_COMPUTE_SHADER, "src/shaders/FLIP/p2g/applyweights.glsl");
    applyWeightsShader.link();

    solveIncompressibilityShader.create();
    solveIncompressibilityShader.load(GL_COMPUTE_SHADER, "src/shaders/FLIP/solveincompressibility.glsl");
    solveIncompressibilityShader.link();

    g2pShader.create();
    g2pShader.load(GL_COMPUTE_SHADER, "src/shaders/FLIP/g2p.glsl");
    g2pShader.link();
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
    // Scan on the local work groups, store the total sum in blockSum
    localSumShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, blockSum);
    localSumShader.dispatch((n + 511) / 512);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Scan blockSum
    smallSumShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, blockSum);
    glUniform1i(ShaderProgram::getVarLoc("length"), n / 512);
    smallSumShader.dispatch(1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Sum the scanned blockSum in all local work groups to get the scanned result 
    globalSumShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, blockSum);
    glUniform1i(ShaderProgram::getVarLoc("length"), n);
    globalSumShader.dispatch((n + 511) / 512);
}

void SolverGPU::countingSort(){
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Reset the buffers
    int ceiledN = (int)glm::ceil((float)(gridX * gridY + 1) / 512.f) * 512;
    resetUintBuffer(firstCellParticleBuffer, ceiledN);
    resetUintBuffer(firstCellParticleBuffer2, ceiledN);
    resetUintBuffer(blockSumBuffer, ceiledN / 512);
    resetUintBuffer(cellParticleIdsBuffer, partN);
    resetUintBuffer(cellOfBuffer, partN);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Count number of particles in each cell
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

    // Exclusive scan of the number of particles
    prefixSum(firstCellParticleBuffer, blockSumBuffer, ceiledN);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    // Copy in another buffer for the next step
    glBindBuffer(GL_COPY_READ_BUFFER, firstCellParticleBuffer);
    glBindBuffer(GL_COPY_WRITE_BUFFER, firstCellParticleBuffer2);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, ceiledN * sizeof(uint));
    
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Get the ordered particle ids grouped by cell 
    cellParticleIdShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, firstCellParticleBuffer2);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cellOfBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, cellParticleIdsBuffer);
    glUniform1i(ShaderProgram::getVarLoc("partN"), partN);
    cellParticleIdShader.dispatch((partN + 63) / 64);
}

void SolverGPU::pushAppartParticles(int iterations){
    countingSort();

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    const float minDist = 2.0f * radius;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, correctionBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, numCorrectionBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, firstCellParticleBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, cellParticleIdsBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, partPosBuffer);
    for (int i = 0; i < iterations; i++)
    {
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        
        resetBuffersShader.use();
        glUniform1i(ShaderProgram::getVarLoc("partN"), partN);
        resetBuffersShader.dispatch((partN + 255) / 256);
        
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        
        getCorrectionsShader.use();
        glUniform1i(ShaderProgram::getVarLoc("partN"), partN);
        glUniform1i(ShaderProgram::getVarLoc("gridX"), gridX);
        glUniform1i(ShaderProgram::getVarLoc("gridY"), gridY);
        glUniform1f(ShaderProgram::getVarLoc("h"), h);
        glUniform1f(ShaderProgram::getVarLoc("minDist"), minDist);
        getCorrectionsShader.dispatch((partN + 63) / 64);

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        applyCorrectionsShader.use();
        glUniform1i(ShaderProgram::getVarLoc("partN"), partN);
        applyCorrectionsShader.dispatch((partN + 255) / 256);
    }
}

void SolverGPU::particleCollisions(){
    countingSort();

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

void SolverGPU::resetFloatBuffer(GLuint buffer, int n){
    resetFloatBufferShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer);
    glUniform1i(ShaderProgram::getVarLoc("n"), n);
    resetFloatBufferShader.dispatch((n + 255) / 256);
}

void SolverGPU::particlesToGrid(){
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, partPosBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, partVelBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, rXBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, rYBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, velXBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, velYBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, isAirBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, firstCellParticleBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, cellParticleIdsBuffer);

    p2gShader.use();
    glUniform1i(ShaderProgram::getVarLoc("partN"), partN);
    glUniform1i(ShaderProgram::getVarLoc("gridX"), gridX);
    glUniform1i(ShaderProgram::getVarLoc("gridY"), gridY);
    glUniform1f(ShaderProgram::getVarLoc("h"), h);
    p2gShader.dispatch((gridX + 7) / 8, (gridY + 7) / 8);
    
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    applyWeightsShader.use();
    glUniform1i(ShaderProgram::getVarLoc("partN"), partN);
    glUniform1i(ShaderProgram::getVarLoc("gridX"), gridX);
    glUniform1i(ShaderProgram::getVarLoc("gridY"), gridY);
    glUniform1f(ShaderProgram::getVarLoc("h"), h);
    applyWeightsShader.dispatch((gridX + 15) / 16, (gridY + 15) / 16);
    
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void SolverGPU::solveIncompressibility(int iterations){
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    glBindBuffer(GL_COPY_READ_BUFFER, velXBuffer);
    glBindBuffer(GL_COPY_WRITE_BUFFER, oldVelXBuffer);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, (gridX + 1) * gridY * sizeof(float));
    glBindBuffer(GL_COPY_READ_BUFFER, velYBuffer);
    glBindBuffer(GL_COPY_WRITE_BUFFER, oldVelYBuffer);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, gridX * (gridY + 1) * sizeof(float));
    
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    
    solveIncompressibilityShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, rXBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, rYBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, velXBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, velYBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, isAirBuffer);
    glUniform1i(ShaderProgram::getVarLoc("gridX"), gridX);
    glUniform1i(ShaderProgram::getVarLoc("gridY"), gridY);
    glUniform1f(ShaderProgram::getVarLoc("h"), h);

    int isPair = 0;

    for (int i = 0; i < 2 * iterations; i++)
    {
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        
        solveIncompressibilityShader.use();
        glUniform1i(ShaderProgram::getVarLoc("isPair"), isPair);
        solveIncompressibilityShader.dispatch((gridX + 7) / 8, (gridY + 7) / 8);

        isPair = 1 - isPair;
    }
}

void SolverGPU::gridToParticles(){
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    
    // vector<float> vel = vector<float>((gridX + 1) * gridY, float(0));
    // glBindBuffer(GL_SHADER_STORAGE_BUFFER, velXBuffer);
    // glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, vel.size() * sizeof(float), vel.data());
    // for (int i = 0; i < vel.size(); i++)
    // {
    //     cout << vel[i] << " ";
    // }
    // cout << " vel" << endl << endl;

    g2pShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, partPosBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, partVelBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, velXBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, velYBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, oldVelXBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, oldVelYBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, isAirBuffer);
    glUniform1i(ShaderProgram::getVarLoc("partN"), partN);
    glUniform1i(ShaderProgram::getVarLoc("gridX"), gridX);
    glUniform1i(ShaderProgram::getVarLoc("gridY"), gridY);
    glUniform1f(ShaderProgram::getVarLoc("h"), h);
    g2pShader.dispatch((partN + 63) / 64);
}

void SolverGPU::updateFlip(){
    integrateParticles();
    particleCollisions();
    pushAppartParticles(10);
    particleCollisions();
    
    particlesToGrid();
    solveIncompressibility(150);
    gridToParticles();
}

void SolverGPU::updateObstacle(vec2 pos, vec2 vel, float rad){
    obstaclePos = pos;
    obstacleVel = vel;
    obstacleRadius = rad;
}