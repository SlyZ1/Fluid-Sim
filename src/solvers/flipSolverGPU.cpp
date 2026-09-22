#include "flipSolverGPU.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include "../helpers/utils.hpp"
#include "../ui/ui_utils.hpp"
#include "../ui/ui.hpp"

using namespace glm;
using namespace std;

void FlipSolverGPUConfig::drawImgui() const {
#define DRAW_FIELD(type, name, val) \
        UI::Label(#name); \
        type temp_##name = name; \
        UIUtils::drawField(#name, temp_##name);

    FLIP_GPU_CONFIG_FIELDS(DRAW_FIELD)
#undef DRAW_FIELD
}

void FlipSolverGPU::deleteBuffers(){
    glDeleteBuffers(1, &m_rXBuffer); glDeleteBuffers(1, &m_rYBuffer); glDeleteBuffers(1, &m_rZBuffer);
    glDeleteBuffers(1, &m_velXBuffer); glDeleteBuffers(1, &m_velYBuffer); glDeleteBuffers(1, &m_velZBuffer);
    glDeleteBuffers(1, &m_oldVelXBuffer); glDeleteBuffers(1, &m_oldVelYBuffer); glDeleteBuffers(1, &m_oldVelZBuffer);
    glDeleteBuffers(1, &m_partPosBuffer); glDeleteBuffers(1, &m_partVelBuffer); 
    glDeleteBuffers(1, &m_isAirBuffer); glDeleteBuffers(1, &m_oldPartPosBuffer);

    glDeleteBuffers(1, &m_blockSumBuffer); glDeleteBuffers(1, &m_cellOfBuffer);
    glDeleteBuffers(1, &m_firstCellParticleBuffer); glDeleteBuffers(1, &m_cellParticleIdsBuffer);
    glDeleteBuffers(1, &m_firstCellParticleBuffer2);

    glDeleteBuffers(1, &m_minusDivBuffer); glDeleteBuffers(1, &m_pressureBuffer);

    glDeleteBuffers(1, &m_rhoBuffer); glDeleteBuffers(1, &m_smoothRhoBuffer); glDeleteBuffers(1, &m_gradRhoBuffer);
    glDeleteBuffers(1, &m_curvatureBuffer);
}

void FlipSolverGPU::createBuffers(){
    deleteBuffers();
    vector<vec4> partVel = vector<vec4>(m_config.partN, vec4(0.f));
    vector<vec4> partPos = vector<vec4>(m_config.partN, vec4(0.f));
    int a = (int)glm::floor(pow(m_config.partN, 1.0f / 3.0f));
    for (int i = 0; i < m_config.partN; i++)
    {
        int x = (i % a);
        int rest = (i - x) / a;
        int y = rest % a;
        int z = (rest - y) / a;
        partPos[i] = (vec4(x, y, z, 1) + 1.f * vec4(1, 0, 1, 0) * 0.5f * (float)((int)y % 2) - vec4(a * 0.5f)) * 2.f * m_config.partRadius * 1.f;
        // partPos[i].x -= h * 30;
        // partPos[i].y -= h * 30;
    }

    glGenBuffers(1, &m_rXBuffer); glGenBuffers(1, &m_rYBuffer); glGenBuffers(1, &m_rZBuffer);
    glGenBuffers(1, &m_velXBuffer); glGenBuffers(1, &m_velYBuffer); glGenBuffers(1, &m_velZBuffer);
    glGenBuffers(1, &m_oldVelXBuffer); glGenBuffers(1, &m_oldVelYBuffer); glGenBuffers(1, &m_oldVelZBuffer);
    glGenBuffers(1, &m_partPosBuffer); glGenBuffers(1, &m_partVelBuffer); 
    glGenBuffers(1, &m_isAirBuffer); glGenBuffers(1, &m_oldPartPosBuffer);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_rXBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, (m_config.gridX + 1) * m_config.gridY * m_config.gridZ * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_rYBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_config.gridX * (m_config.gridY + 1) * m_config.gridZ * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_rZBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_config.gridX * m_config.gridY * (m_config.gridZ + 1) * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_velXBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, (m_config.gridX + 1) * m_config.gridY * m_config.gridZ * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_velYBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_config.gridX * (m_config.gridY + 1) * m_config.gridZ * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_velZBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_config.gridX * m_config.gridY * (m_config.gridZ + 1) * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_oldVelXBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, (m_config.gridX + 1) * m_config.gridY * m_config.gridZ * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_oldVelYBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_config.gridX * (m_config.gridY + 1) * m_config.gridZ * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_oldVelZBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_config.gridX * m_config.gridY * (m_config.gridZ + 1) * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_partPosBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, partPos.size() * sizeof(vec4), partPos.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_oldPartPosBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, partPos.size() * sizeof(vec4), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_partVelBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, partVel.size() * sizeof(vec4), partVel.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_isAirBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_config.gridX * m_config.gridY * m_config.gridZ * sizeof(uint), nullptr, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &m_blockSumBuffer); glGenBuffers(1, &m_cellOfBuffer);
    glGenBuffers(1, &m_firstCellParticleBuffer); glGenBuffers(1, &m_cellParticleIdsBuffer);
    glGenBuffers(1, &m_firstCellParticleBuffer2);
    int ceiledN = (int)glm::ceil((float)(m_config.gridX * m_config.gridY * m_config.gridZ + 1) / 512.f) * 512;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_blockSumBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, ceiledN / 512 * sizeof(uint), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_cellOfBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_config.partN * sizeof(uint), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_firstCellParticleBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, ceiledN * sizeof(uint), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_firstCellParticleBuffer2);
    glBufferData(GL_SHADER_STORAGE_BUFFER, ceiledN * sizeof(uint), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_cellParticleIdsBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_config.partN * sizeof(uint), nullptr, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &m_minusDivBuffer); glGenBuffers(1, &m_pressureBuffer);
    
    vector<float> zeros = vector<float>(m_config.gridX * m_config.gridY * m_config.gridZ, 0.f);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_minusDivBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_config.gridX * m_config.gridY * m_config.gridZ * sizeof(float), zeros.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_pressureBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_config.gridX * m_config.gridY * m_config.gridZ * sizeof(float), zeros.data(), GL_DYNAMIC_DRAW);

    glGenBuffers(1, &m_rhoBuffer); glGenBuffers(1, &m_smoothRhoBuffer); glGenBuffers(1, &m_gradRhoBuffer);
    glGenBuffers(1, &m_curvatureBuffer);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_rhoBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_config.gridX * m_config.gridY * m_config.gridZ * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_smoothRhoBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_config.gridX * m_config.gridY * m_config.gridZ * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_gradRhoBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_config.gridX * m_config.gridY * m_config.gridZ * sizeof(vec4), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_curvatureBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_config.gridX * m_config.gridY * m_config.gridZ * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
}

void FlipSolverGPU::loadCompute(ShaderProgram& prog, const std::string& path){
    prog.create();
    prog.load(GL_COMPUTE_SHADER, path);
    prog.link();
}

FlipSolverGPU::FlipSolverGPU(FlipSolverGPUConfig config) 
: IParticleSolver(make_unique<FlipSolverGPUConfig>(move(config)), "FLIP GPU Solver"), m_config(static_cast<FlipSolverGPUConfig&>(*m_baseConfig)) {
    m_integrationStatIndex        = m_stats->registerTimer("Integration");
    m_pushAppartStatIndex         = m_stats->registerTimer("Push Appart");
    m_collisionStatIndex          = m_stats->registerTimer("Collision");
    m_p2gStatIndex                = m_stats->registerTimer("P2G");
    m_surfaceTensionStatIndex     = m_stats->registerTimer("Surface Tension");
    m_incompressibilityStatIndex  = m_stats->registerTimer("Incompressibility");
    m_g2pStatIndex                = m_stats->registerTimer("G2P");

    StatIndex numPartStatIndex = m_stats->registerCounter("Num Particles");
    StatIndex numCellsStatIndex = m_stats->registerCounter("Num Cells");
    m_stats->setCounter(numPartStatIndex, m_config.partN);
    m_stats->setCounter(numCellsStatIndex, m_config.gridX * m_config.gridY * m_config.gridZ);

    m_integrateTimer.init();
    m_collisionTimer.init();
    m_pushAppartTimer.init();
    m_p2gTimer.init();
    m_surfaceTensionTimer.init();
    m_incompressibilityTimer.init();
    m_g2pTimer.init();
    m_scanTimer.init();

    loadCompute(m_integrateShader, "src/shaders/FLIP/integrate.glsl");
    loadCompute(m_collisionShader, "src/shaders/FLIP/collisions.glsl");
    loadCompute(m_resetUintBuffersShader, "src/shaders/FLIP/prefixsum/resetuintbuffer.glsl");
    loadCompute(m_partCountShader, "src/shaders/FLIP/pushappart/partcount.glsl");
    loadCompute(m_localSumShader, "src/shaders/FLIP/prefixsum/localsum.glsl");
    loadCompute(m_smallSumShader, "src/shaders/FLIP/prefixsum/smallsum.glsl");
    loadCompute(m_globalSumShader, "src/shaders/FLIP/prefixsum/globalsum.glsl");
    loadCompute(m_cellParticleIdShader, "src/shaders/FLIP/pushappart/cellparticleid.glsl");
    loadCompute(m_resetBuffersShader, "src/shaders/FLIP/pushappart/resetbuffers.glsl");
    loadCompute(m_getCorrectionsShader, "src/shaders/FLIP/pushappart/getcorrections.glsl");
    loadCompute(m_applyCorrectionsShader, "src/shaders/FLIP/pushappart/applycorrections.glsl");
    loadCompute(m_p2gShader, "src/shaders/FLIP/p2g/p2g.glsl");
    loadCompute(m_resetFloatBufferShader, "src/shaders/FLIP/p2g/resetfloatbuffer.glsl");
    loadCompute(m_applyWeightsShader, "src/shaders/FLIP/p2g/applyweights.glsl");
    loadCompute(m_solveIncompressibilityShader, "src/shaders/FLIP/solveincompressibility.glsl");
    loadCompute(m_g2pShader, "src/shaders/FLIP/g2p.glsl");    
    loadCompute(m_sparseMatVecShader, "src/shaders/FLIP/incompressibility/matvec.glsl");
    loadCompute(m_computeMinusDivShader, "src/shaders/FLIP/incompressibility/computeminusdiv.glsl");
    loadCompute(m_pressureToVelShader, "src/shaders/FLIP/incompressibility/pressuretovel.glsl");
    loadCompute(m_setAirCellsToZeroShader, "src/shaders/FLIP/incompressibility/setaircellstozero.glsl");
    loadCompute(m_computeRhoShader, "src/shaders/FLIP/surfacetension/computerho.glsl");
    loadCompute(m_smoothDataShader, "src/shaders/FLIP/surfacetension/smoothdata.glsl");
    loadCompute(m_computeGradShader, "src/shaders/FLIP/surfacetension/computegrad.glsl");
    loadCompute(m_computeCurvatureShader, "src/shaders/FLIP/surfacetension/computecurvature.glsl");
    loadCompute(m_integrateGridShader, "src/shaders/FLIP/integrategrid.glsl");
    createBuffers();

    m_cgs.init(m_config.gridX * m_config.gridY * m_config.gridZ, 0, m_minusDivBuffer, m_pressureBuffer);
}

FlipSolverGPU::~FlipSolverGPU() {
    deleteBuffers();
}

void FlipSolverGPU::reload() {
    m_integrateShader.reload();
    m_collisionShader.reload();
    m_resetUintBuffersShader.reload();
    m_partCountShader.reload();
    m_localSumShader.reload();
    m_smallSumShader.reload();
    m_globalSumShader.reload();
    m_cellParticleIdShader.reload();
    m_resetBuffersShader.reload();
    m_getCorrectionsShader.reload();
    m_applyCorrectionsShader.reload();
    m_p2gShader.reload();
    m_resetFloatBufferShader.reload();
    m_applyWeightsShader.reload();
    m_solveIncompressibilityShader.reload();
    m_g2pShader.reload();
    m_sparseMatVecShader.reload();
    m_computeMinusDivShader.reload();
    m_pressureToVelShader.reload();
    m_setAirCellsToZeroShader.reload();
    m_computeRhoShader.reload();
    m_smoothDataShader.reload();
    m_computeGradShader.reload();
    m_computeCurvatureShader.reload();
    m_integrateGridShader.reload();
    createBuffers();
    
    m_cgs.reloadArgs(0, m_minusDivBuffer, m_pressureBuffer);
}

ivec3 FlipSolverGPU::cellToCoord(int cell, int nx, int ny){
    int column = cell % nx;
    int rest = (cell - column) / nx;
    int row = rest % ny;
    int layer = (rest - row) / ny;
    return ivec3(column, row, layer);
}

vec3 FlipSolverGPU::cellToPos(int cell, int nx, int ny, int nz){
    ivec3 coord = cellToCoord(cell, nx, ny);
    vec3 pos = ((vec3)coord - vec3(nx - 1, ny - 1, nz - 1) * 0.5f) * m_config.h();
    return pos;
}

void FlipSolverGPU::integrateParticles(){
    ShaderProgram::SSBOBarrier();

    m_integrateShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_partPosBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_partVelBuffer);
    glUniform1f(ShaderProgram::getVarLoc("dt"), m_config.dt);
    glUniform1i(ShaderProgram::getVarLoc("partN"), m_config.partN);
    m_integrateShader.dispatch((m_config.partN + 255) / 256);
}

void FlipSolverGPU::resetUintBuffer(GLuint buffer, int n){
    m_resetUintBuffersShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer);
    glUniform1i(ShaderProgram::getVarLoc("n"), n);
    m_resetUintBuffersShader.dispatch((n + 255) / 256);
}

void FlipSolverGPU::prefixSum(GLuint data, GLuint blockSum, int n){
    // Scan on the local work groups, store the total sum in blockSum
    m_localSumShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, blockSum);
    m_localSumShader.dispatch((n + 511) / 512);

    ShaderProgram::SSBOBarrier();

    // Scan blockSum
    m_smallSumShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, blockSum);
    glUniform1i(ShaderProgram::getVarLoc("length"), n / 512);
    m_smallSumShader.dispatch(1); 

    ShaderProgram::SSBOBarrier();

    // Sum the scanned blockSum in all local work groups to get the scanned result 
    m_globalSumShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, blockSum);
    glUniform1i(ShaderProgram::getVarLoc("length"), n);
    m_globalSumShader.dispatch((n + 511) / 512);
}

void FlipSolverGPU::countingSort(){
    ShaderProgram::SSBOBarrier();

    // Reset the buffers
    int ceiledN = (int)glm::ceil((float)(m_config.gridX * m_config.gridY * m_config.gridZ + 1) / 512.f) * 512;
    resetUintBuffer(m_firstCellParticleBuffer, ceiledN);
    resetUintBuffer(m_firstCellParticleBuffer2, ceiledN);
    resetUintBuffer(m_blockSumBuffer, ceiledN / 512);
    resetUintBuffer(m_cellParticleIdsBuffer, m_config.partN);
    resetUintBuffer(m_cellOfBuffer, m_config.partN);

    ShaderProgram::SSBOBarrier();

    // Count number of particles in each cell
    m_partCountShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_firstCellParticleBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_cellOfBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_partPosBuffer);

    glUniform1i(ShaderProgram::getVarLoc("partN"), m_config.partN);
    glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config.gridX);
    glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config.gridY);
    glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config.gridZ);
    glUniform1f(ShaderProgram::getVarLoc("h"), m_config.h());
    m_partCountShader.dispatch((m_config.partN + 63) / 64);
    
    ShaderProgram::SSBOBarrier();

    // Exclusive scan of the number of particles
    prefixSum(m_firstCellParticleBuffer, m_blockSumBuffer, ceiledN);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    // Copy in another buffer for the next step
    glBindBuffer(GL_COPY_READ_BUFFER, m_firstCellParticleBuffer);
    glBindBuffer(GL_COPY_WRITE_BUFFER, m_firstCellParticleBuffer2);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, ceiledN * sizeof(uint));
    
    ShaderProgram::SSBOBarrier();

    // Get the ordered particle ids grouped by cell 
    m_cellParticleIdShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_firstCellParticleBuffer2);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_cellOfBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_cellParticleIdsBuffer);
    glUniform1i(ShaderProgram::getVarLoc("partN"), m_config.partN);
    m_cellParticleIdShader.dispatch((m_config.partN + 63) / 64);
}

void FlipSolverGPU::pushAppartParticles(int iterations){
    countingSort();

    const float minDist = 2.0f * m_config.partRadius;
    for (int i = 0; i < iterations; i++)
    {
        glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

        glBindBuffer(GL_COPY_READ_BUFFER, m_partPosBuffer);
        glBindBuffer(GL_COPY_WRITE_BUFFER, m_oldPartPosBuffer);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, m_config.partN * sizeof(vec4));
        
        ShaderProgram::SSBOBarrier();

        m_getCorrectionsShader.use();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_firstCellParticleBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_cellParticleIdsBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_oldPartPosBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_partPosBuffer);
        glUniform1i(ShaderProgram::getVarLoc("partN"), m_config.partN);
        glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config.gridX);
        glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config.gridY);
        glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config.gridZ);
        glUniform1f(ShaderProgram::getVarLoc("h"), m_config.h());
        glUniform1f(ShaderProgram::getVarLoc("minDist"), minDist);
        m_getCorrectionsShader.dispatch((m_config.partN + 255) / 256);
        
        // ShaderProgram::SSBOBarrier();
        
        // applyCorrectionsShader.use();
        // glUniform1i(ShaderProgram::getVarLoc("partN"), partN);
        // applyCorrectionsShader.dispatch((partN + 511) / 512);

        //particleCollisions();
    }
}

void FlipSolverGPU::particleCollisions(){
    ShaderProgram::SSBOBarrier();

    m_collisionShader.use();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_partPosBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_partVelBuffer);

    vec3 minPos = cellToPos(0, m_config.gridX, m_config.gridY, m_config.gridZ);
    vec3 maxPos = cellToPos(m_config.gridX * m_config.gridY * m_config.gridZ - 1, m_config.gridX, m_config.gridY, m_config.gridZ);

    glUniform1i(ShaderProgram::getVarLoc("partN"), m_config.partN);
    glUniform3f(ShaderProgram::getVarLoc("minPos"), minPos.x, minPos.y, minPos.z);
    glUniform3f(ShaderProgram::getVarLoc("maxPos"), maxPos.x, maxPos.y, maxPos.z);
    glUniform1f(ShaderProgram::getVarLoc("radius"), m_config.partRadius);
    glUniform2f(ShaderProgram::getVarLoc("obstaclePos"), m_obstaclePos.x, m_obstaclePos.y);
    glUniform2f(ShaderProgram::getVarLoc("obstacleVel"), m_obstacleVel.x, m_obstacleVel.y);
    glUniform1f(ShaderProgram::getVarLoc("obstacleRadius"), m_obstacleRadius * 10);

    m_collisionShader.dispatch((m_config.partN + 255) / 256);
}

void FlipSolverGPU::resetFloatBuffer(GLuint buffer, int n){
    m_resetFloatBufferShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer);
    glUniform1i(ShaderProgram::getVarLoc("n"), n);
    m_resetFloatBufferShader.dispatch((n + 255) / 256);
}

void FlipSolverGPU::particlesToGrid(){
    countingSort();

    ShaderProgram::SSBOBarrier();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_partPosBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_partVelBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_rXBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_rYBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_rZBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_velXBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, m_velYBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, m_velZBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, m_isAirBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, m_firstCellParticleBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, m_cellParticleIdsBuffer);

    m_p2gShader.use();
    glUniform1i(ShaderProgram::getVarLoc("partN"), m_config.partN);
    glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config.gridX);
    glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config.gridY);
    glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config.gridZ);
    glUniform1f(ShaderProgram::getVarLoc("h"), m_config.h());
    m_p2gShader.dispatch((m_config.gridX + 7) / 8, (m_config.gridY + 7) / 8, (m_config.gridZ + 7) / 8);
    
    ShaderProgram::SSBOBarrier();

    m_applyWeightsShader.use();
    glUniform1i(ShaderProgram::getVarLoc("partN"), m_config.partN);
    glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config.gridX);
    glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config.gridY);
    glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config.gridZ);
    glUniform1f(ShaderProgram::getVarLoc("h"), m_config.h());
    m_applyWeightsShader.dispatch((m_config.gridX+1 + 7) / 8, (m_config.gridY+1 + 7) / 8, (m_config.gridZ+1 + 7) / 8);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    glBindBuffer(GL_COPY_READ_BUFFER, m_velXBuffer);
    glBindBuffer(GL_COPY_WRITE_BUFFER, m_oldVelXBuffer);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, (m_config.gridX + 1) * m_config.gridY * m_config.gridZ * sizeof(float));
    glBindBuffer(GL_COPY_READ_BUFFER, m_velYBuffer);
    glBindBuffer(GL_COPY_WRITE_BUFFER, m_oldVelYBuffer);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, m_config.gridX * (m_config.gridY + 1) * m_config.gridZ * sizeof(float));
    glBindBuffer(GL_COPY_READ_BUFFER, m_velZBuffer);
    glBindBuffer(GL_COPY_WRITE_BUFFER, m_oldVelZBuffer);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, m_config.gridX * m_config.gridY * (m_config.gridZ + 1) * sizeof(float));
}

void FlipSolverGPU::surfaceTension(){
    ShaderProgram::SSBOBarrier();

    m_computeRhoShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_rXBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_rYBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_rZBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_isAirBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_rhoBuffer);
    glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config.gridX);
    glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config.gridY);
    glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config.gridZ);
    glUniform1f(ShaderProgram::getVarLoc("h"), m_config.h());
    m_computeRhoShader.dispatch((m_config.gridX + 7) / 8, (m_config.gridY + 7) / 8, (m_config.gridZ + 7) / 8);
    
    ShaderProgram::SSBOBarrier();

    m_smoothDataShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_rhoBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_smoothRhoBuffer);
    glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config.gridX);
    glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config.gridY);
    glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config.gridZ);
    glUniform1f(ShaderProgram::getVarLoc("h"), m_config.h());
    m_smoothDataShader.dispatch((m_config.gridX + 7) / 8, (m_config.gridY + 7) / 8, (m_config.gridZ + 7) / 8);
    
    ShaderProgram::SSBOBarrier();

    m_computeGradShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_smoothRhoBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_gradRhoBuffer);
    glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config.gridX);
    glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config.gridY);
    glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config.gridZ);
    glUniform1f(ShaderProgram::getVarLoc("h"), m_config.h());
    m_computeGradShader.dispatch((m_config.gridX + 7) / 8, (m_config.gridY + 7) / 8, (m_config.gridZ + 7) / 8);
    
    ShaderProgram::SSBOBarrier();

    m_computeCurvatureShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_gradRhoBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_curvatureBuffer);
    glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config.gridX);
    glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config.gridY);
    glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config.gridZ);
    glUniform1f(ShaderProgram::getVarLoc("h"), m_config.h());
    m_computeCurvatureShader.dispatch((m_config.gridX + 7) / 8, (m_config.gridY + 7) / 8, (m_config.gridZ + 7) / 8);
    
    ShaderProgram::SSBOBarrier();

    m_integrateGridShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_curvatureBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_smoothRhoBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_velXBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_velYBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_velZBuffer);
    glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config.gridX);
    glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config.gridY);
    glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config.gridZ);
    glUniform1f(ShaderProgram::getVarLoc("h"), m_config.h());
    glUniform1f(ShaderProgram::getVarLoc("dt"), m_config.dt);
    glUniform1f(ShaderProgram::getVarLoc("sigma"), 0);
    m_integrateGridShader.dispatch((m_config.gridX + 7) / 8, (m_config.gridY + 7) / 8, (m_config.gridZ + 7) / 8);
}

void FlipSolverGPU::solveIncompressibility(int iterations, bool useCGS){
    ShaderProgram::SSBOBarrier();

    if (useCGS){
        m_computeMinusDivShader.use();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_velXBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_velYBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_velZBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_rhoBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_isAirBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_minusDivBuffer);
        glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config.gridX);
        glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config.gridY);
        glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config.gridZ);
        glUniform1f(ShaderProgram::getVarLoc("h"), m_config.h());
        m_computeMinusDivShader.dispatch((m_config.gridX + 7) / 8, (m_config.gridY + 7) / 8, (m_config.gridZ + 7) / 8);
        
        ShaderProgram::SSBOBarrier();
        
        m_setAirCellsToZeroShader.use();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_isAirBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_pressureBuffer);
        glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config.gridX);
        glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config.gridY);
        glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config.gridZ);
        glUniform1f(ShaderProgram::getVarLoc("h"), m_config.h());
        m_setAirCellsToZeroShader.dispatch((m_config.gridX + 7) / 8, (m_config.gridY + 7) / 8, (m_config.gridZ + 7) / 8);
    
        ShaderProgram::SSBOBarrier();
    
        std::function<void(GLuint, GLuint, GLuint, int, bool)> sparseMatVec =
        [this](GLuint, GLuint dBuf, GLuint AdBuf, int, bool dispatch) {
            m_sparseMatVecShader.use();
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, dBuf);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_isAirBuffer);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, AdBuf);
            glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config.gridX);
            glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config.gridY);
            glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config.gridZ);
            glUniform1f(ShaderProgram::getVarLoc("h"), m_config.h());
            if (dispatch){
                m_sparseMatVecShader.dispatch((m_config.gridX + 7) / 8, (m_config.gridY + 7) / 8, (m_config.gridZ + 7) / 8);
            }
        };
        DispatchParams sparseMatVecParams = { (uint)(m_config.gridX + 7) / 8, (uint)(m_config.gridY + 7) / 8, (uint)(m_config.gridZ + 7) / 8 };
        m_cgs.solve(iterations, 1e-3f, sparseMatVec, sparseMatVecParams);
    
        ShaderProgram::SSBOBarrier();
    
        m_pressureToVelShader.use();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_pressureBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_isAirBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_velXBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_velYBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_velZBuffer);
        glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config.gridX);
        glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config.gridY);
        glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config.gridZ);
        glUniform1f(ShaderProgram::getVarLoc("h"), m_config.h());
        m_pressureToVelShader.dispatch((m_config.gridX + 7) / 8, (m_config.gridY + 7) / 8, (m_config.gridZ + 7) / 8);
    
        ShaderProgram::SSBOBarrier();

        m_computeMinusDivShader.use();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_velXBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_velYBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_velZBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_rhoBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_isAirBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_minusDivBuffer);
        glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config.gridX);
        glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config.gridY);
        glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config.gridZ);
        glUniform1f(ShaderProgram::getVarLoc("h"), m_config.h());
        m_computeMinusDivShader.dispatch((m_config.gridX + 7) / 8, (m_config.gridY + 7) / 8, (m_config.gridZ + 7) / 8);
    }
    else{
        m_solveIncompressibilityShader.use();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_rXBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_rYBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_rZBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_velXBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_velYBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_velZBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, m_isAirBuffer);
        glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config.gridX);
        glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config.gridY);
        glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config.gridZ);
        glUniform1f(ShaderProgram::getVarLoc("h"), m_config.h());
    
        int isPair = 0;
    
        for (int i = 0; i < 2 * iterations; i++)
        {
            ShaderProgram::SSBOBarrier();
            
            m_solveIncompressibilityShader.use();
            glUniform1i(ShaderProgram::getVarLoc("isPair"), isPair);
            m_solveIncompressibilityShader.dispatch((m_config.gridX + 7) / 8, (m_config.gridY + 7) / 8, (m_config.gridZ + 7) / 8);
    
            isPair = 1 - isPair;
        }
    }
}

void FlipSolverGPU::gridToParticles(){
    ShaderProgram::SSBOBarrier();

    m_g2pShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_partPosBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_partVelBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_velXBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_velYBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_velZBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_oldVelXBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, m_oldVelYBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, m_oldVelZBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, m_isAirBuffer);
    glUniform1i(ShaderProgram::getVarLoc("partN"), m_config.partN);
    glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config.gridX);
    glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config.gridY);
    glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config.gridZ);
    glUniform1f(ShaderProgram::getVarLoc("h"), m_config.h());
    m_g2pShader.dispatch((m_config.partN + 63) / 64);
}

void FlipSolverGPU::update(){
    m_integrateTimer.beginFrame();
    integrateParticles();
    m_integrateTimer.endFrame();

    m_collisionTimer.beginFrame();
    particleCollisions();
    m_collisionTimer.endFrame();

    m_pushAppartTimer.beginFrame();
    pushAppartParticles(2);
    m_pushAppartTimer.endFrame();

    particleCollisions();
    
    m_p2gTimer.beginFrame();
    particlesToGrid();
    m_p2gTimer.endFrame();

    m_surfaceTensionTimer.beginFrame();
    surfaceTension();
    m_surfaceTensionTimer.endFrame();

    m_incompressibilityTimer.beginFrame();
    solveIncompressibility(20, true);
    m_incompressibilityTimer.endFrame();

    m_g2pTimer.beginFrame();
    gridToParticles();
    m_g2pTimer.endFrame();
    
    m_stats->setTimer(m_integrationStatIndex,       m_integrateTimer.getLastResultMs());
    m_stats->setTimer(m_collisionStatIndex,         2 * m_collisionTimer.getLastResultMs());
    m_stats->setTimer(m_pushAppartStatIndex,        m_pushAppartTimer.getLastResultMs());
    m_stats->setTimer(m_p2gStatIndex,               m_p2gTimer.getLastResultMs());
    m_stats->setTimer(m_surfaceTensionStatIndex,    m_surfaceTensionTimer.getLastResultMs());
    m_stats->setTimer(m_incompressibilityStatIndex, m_incompressibilityTimer.getLastResultMs());
    m_stats->setTimer(m_g2pStatIndex,               m_g2pTimer.getLastResultMs());
}

void FlipSolverGPU::updateObstacle(vec2 pos, vec2 vel, float rad){
    m_obstaclePos = pos;
    m_obstacleVel = vel;
    m_obstacleRadius = rad;
}