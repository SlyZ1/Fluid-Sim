#include "cgs.hpp"

void CGS::initBuffers(){
    glDeleteBuffers(1, &ATABuffer);
    glGenBuffers(1, &ATABuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ATABuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, n * n * sizeof(float), nullptr, GL_DYNAMIC_COPY);

    glDeleteBuffers(1, &ATBuffer);
    glGenBuffers(1, &ATBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ATBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, n * n * sizeof(float), nullptr, GL_DYNAMIC_COPY);

    glDeleteBuffers(1, &ATbBuffer);
    glGenBuffers(1, &ATbBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ATbBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, n * sizeof(float), nullptr, GL_DYNAMIC_COPY);

    glDeleteBuffers(1, &zeroBuffer);
    glGenBuffers(1, &zeroBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, zeroBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, n * sizeof(float), vector<float>(n, 0.0f).data(), GL_DYNAMIC_COPY);

    glDeleteBuffers(1, &dBuffer);
    glGenBuffers(1, &dBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, dBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, n * sizeof(float), vector<float>(n, 0.0f).data(), GL_DYNAMIC_COPY);
    
    glDeleteBuffers(1, &AdBuffer);
    glGenBuffers(1, &AdBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, AdBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, n * sizeof(float), vector<float>(n, 0.0f).data(), GL_DYNAMIC_COPY);

    glDeleteBuffers(1, &rBuffer);
    glGenBuffers(1, &rBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, rBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, n * sizeof(float), vector<float>(n, 0.0f).data(), GL_DYNAMIC_COPY);
    
    vector<float> scalars = vector<float>(NUM_SCALARS, 0);
    scalars[MINUS_ONE_SCALAR_INDEX] = -1;
    glDeleteBuffers(1, &scalarBuffer);
    glGenBuffers(1, &scalarBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, scalarBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_SCALARS * sizeof(float), scalars.data(), GL_DYNAMIC_COPY);

    dispatchParams = vector<DispatchParams>{
        {GLuint((n + 63) / 64), 1, 1},     // matVec
        {GLuint((n + 127) / 128), 1, 1},   // dot1
        {GLuint((n + 255) / 256), 1, 1},   // saxpy
        {1, 1, 1},                         // constant
    };

    glDeleteBuffers(1, &indirectBuffer);
    glGenBuffers(1, &indirectBuffer);
    glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, indirectBuffer);
    glBufferData(GL_DISPATCH_INDIRECT_BUFFER, dispatchParams.size() * sizeof(DispatchParams), 
                                              dispatchParams.data(), GL_DYNAMIC_DRAW);
}

void CGS::compute_ATA_ATb(GLuint AMatrixbuffer, GLuint bMatrixBuffer){
    matOps->transpose(AMatrixbuffer, ATBuffer, n);
    matOps->multiply(ATBuffer, AMatrixbuffer, ATABuffer, n);
    matOps->copy(ATABuffer, AMatrixbuffer, n * n);

    matOps->matVec(ATBuffer, bMatrixBuffer, ATbBuffer, n);
    matOps->copy(ATbBuffer, bMatrixBuffer, n);
}

void CGS::init(int nNew, GLuint ABufferNew, GLuint bBufferNew, GLuint xBufferNew){
    this->n = nNew;
    this->ABuffer = ABufferNew;
    this->bBuffer = bBufferNew;
    this->xBuffer = xBufferNew;
    initBuffers();

    stopProg.create();
    stopProg.load(GL_COMPUTE_SHADER, "src/shaders/mat/cgs_stop_cond.glsl");
    stopProg.link();

    matOps = new MatOps();
}

void CGS::swap_rtr_indices(){
    int temp = RTR_NEW_SCALAR_INDEX;
    RTR_NEW_SCALAR_INDEX = RTR_SCALAR_INDEX;
    RTR_SCALAR_INDEX = temp;
}

GLuint CGS::solve(int maxIter, float tol){
    std::function<void(GLuint, GLuint, GLuint, int, bool)> matVec =
    [this](GLuint Abuf, GLuint dBuf, GLuint AdBuf, int n, bool dispatch) {
        matOps->matVec(Abuf, dBuf, AdBuf, n, dispatch);
    };
    return solve(maxIter, tol, matVec, dispatchParams[0]);
}

GLuint CGS::solve(int maxIter, float tol, function<void(GLuint, GLuint, GLuint, int, bool)> matVec, DispatchParams matVecParams){
    return solve(zeroBuffer, maxIter, tol, matVec, matVecParams);
}

GLuint CGS::solve(GLuint previousXBuffer, int maxIter, float tol, function<void(GLuint, GLuint, GLuint, int, bool)> matVec, DispatchParams matVecParams){
    dispatchParams[0] = matVecParams;
    glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, indirectBuffer);
    glBufferData(GL_DISPATCH_INDIRECT_BUFFER, dispatchParams.size() * sizeof(DispatchParams), 
                                              dispatchParams.data(), GL_DYNAMIC_DRAW);
    
    static vector<int> xAlphaScalarOperations = vector<int>(16,0);
    static vector<int> rAlphaScalarOperations = vector<int>(16,0);
    static vector<int> betaScalarOperations = vector<int>(16,0);
    static vector<int> minusOperations = vector<int>(16,0);

    minusOperations[MINUS_ONE_SCALAR_INDEX] = 1;
    xAlphaScalarOperations[RTR_NEW_SCALAR_INDEX] = 1;
    xAlphaScalarOperations[DTAD_SCALAR_INDEX] = -1;
    rAlphaScalarOperations[RTR_NEW_SCALAR_INDEX] = 1;
    rAlphaScalarOperations[DTAD_SCALAR_INDEX] = -1;
    rAlphaScalarOperations[MINUS_ONE_SCALAR_INDEX] = 1;
    betaScalarOperations[RTR_NEW_SCALAR_INDEX] = 1;
    betaScalarOperations[RTR_SCALAR_INDEX] = -1;
    
    vector<float> initScalars = vector<float>(NUM_SCALARS, 0.0f);
    initScalars[MINUS_ONE_SCALAR_INDEX] = -1.0f;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, scalarBuffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, NUM_SCALARS * sizeof(float), initScalars.data());
    
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    
    // r = b, d = r, compute rTr
    //matOps->copy(zeroBuffer, xBuffer, n);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    // Ad = A * x
    matVec(ABuffer, xBuffer, AdBuffer, n, true);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    // r = b - Ad
    matOps->saxpy(bBuffer, AdBuffer, rBuffer, scalarBuffer, n, minusOperations, true);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    //matOps->copy(bBuffer, rBuffer, n);

    matOps->copy(rBuffer, dBuffer, n);
    matOps->dot(rBuffer, rBuffer, scalarBuffer, n, glm::ivec3(RTR_NEW_SCALAR_INDEX, RTR0_VALUE_INDEX, -1));
    for (int i = 0; i < maxIter; i++)
    {
        // Compute Ad
        ShaderProgram::indirectBarrier();
        matVec(ABuffer, dBuffer, AdBuffer, n, false);
        ShaderProgram::indirectDispatch(indirectBuffer, 0 * sizeof(DispatchParams));

        // Compute dTAd
        ShaderProgram::indirectBarrier();
        matOps->dotIndirect(
            dBuffer, 
            AdBuffer, 
            scalarBuffer, 
            indirectBuffer, 
            1 * sizeof(DispatchParams),
            3 * sizeof(DispatchParams),
            n, 
            DTAD_SCALAR_INDEX
        );
        //ShaderProgram::indirectDispatch(indirectBuffer, 1 * sizeof(DispatchParams));

        // alpha = rTrnew / dTad
        // Compute x += alpha * d
        ShaderProgram::indirectBarrier();
        matOps->saxpy(xBuffer, dBuffer, xBuffer, scalarBuffer, n, xAlphaScalarOperations, false);
        ShaderProgram::indirectDispatch(indirectBuffer, 2 * sizeof(DispatchParams));

        // Compute r -= alpha * Ad
        matOps->saxpy(rBuffer, AdBuffer, rBuffer, scalarBuffer, n, rAlphaScalarOperations, false);
        ShaderProgram::indirectDispatch(indirectBuffer, 2 * sizeof(DispatchParams));

        // rTr<->rTrnew, Compute rTrnew 
        swap_rtr_indices();
        ShaderProgram::indirectBarrier();
        matOps->dotIndirect(
            rBuffer, 
            rBuffer, 
            scalarBuffer, 
            indirectBuffer,
            1 * sizeof(DispatchParams),
            3 * sizeof(DispatchParams),
            n, 
            RTR_NEW_SCALAR_INDEX
        );
        //ShaderProgram::indirectDispatch(indirectBuffer, 1 * sizeof(DispatchParams));
        xAlphaScalarOperations = vector<int>(16,0);
        xAlphaScalarOperations[RTR_NEW_SCALAR_INDEX] = 1;
        xAlphaScalarOperations[DTAD_SCALAR_INDEX] = -1;
        rAlphaScalarOperations = vector<int>(16,0);
        rAlphaScalarOperations[RTR_NEW_SCALAR_INDEX] = 1;
        rAlphaScalarOperations[DTAD_SCALAR_INDEX] = -1;
        rAlphaScalarOperations[MINUS_ONE_SCALAR_INDEX] = 1;
        betaScalarOperations = vector<int>(16,0);
        betaScalarOperations[RTR_NEW_SCALAR_INDEX] = 1;
        betaScalarOperations[RTR_SCALAR_INDEX] = -1;
        
        // Compute d = r + rTrnew / rTr * d
        ShaderProgram::indirectBarrier();
        matOps->saxpy(rBuffer, dBuffer, dBuffer, scalarBuffer, n, betaScalarOperations, false);
        ShaderProgram::indirectDispatch(indirectBuffer, 2 * sizeof(DispatchParams));
        

        // Stop condition + clean scalar
        stopProg.use();
        glUniform1i(ShaderProgram::getVarLoc("rTrIndex"), RTR_NEW_SCALAR_INDEX);
        glUniform1i(ShaderProgram::getVarLoc("rTr0ValueIndex"), RTR0_VALUE_INDEX);
        glUniform1i(ShaderProgram::getVarLoc("numScalars"), NUM_SCALARS);
        glUniform1i(ShaderProgram::getVarLoc("numIndirectParams"), (int)dispatchParams.size());
        glUniform1f(ShaderProgram::getVarLoc("tol"), tol);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, scalarBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, indirectBuffer);
        ShaderProgram::indirectDispatch(indirectBuffer, 3 * sizeof(DispatchParams));
    }

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    
    return xBuffer;
}