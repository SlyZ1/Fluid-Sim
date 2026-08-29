#include "solver.hpp"
#include <omp.h>

Solver::Solver(int partN, float radius, float h, int gridX, int gridY, float timestep) 
: partN(partN), radius(radius), h(h), gridX(gridX), gridY(gridY), dt(timestep) {
    isWall = vector<bool>(gridX * gridY, false);
    velX = vector<float>((gridX + 1) * gridY, 0.f);
    velY = vector<float>(gridX * (gridY + 1), 0.f);
    rX = vector<float>((gridX + 1) * gridY, 0.f);
    rY = vector<float>(gridX * (gridY + 1), 0.f);
    partVel = vector<vec2>(partN, vec2(0.f));
    partPos = vector<vec2>(partN, vec2(0.f));
    int a = (int)glm::floor(sqrt(partN));
    for (int i = 0; i < partN; i++)
    {
        float x = (float)(i % a);
        int y = (int)((i - x) / a);
        x += (y % 2) * 0.5f;
        partPos[i] = (vec2(x - 10,y) - vec2(a * 0.5f)) * 2.f * radius * 1.f;
    }
}

ivec2 Solver::cellToCoord(int cell, int nx){
    int column = cell % nx;
    int row = (cell - column) / nx;
    return ivec2(column, row);
}

int Solver::coordToCell(ivec2 coord, int nx, int ny){
    return glm::clamp(coord.x % nx + nx * coord.y, 0, nx * ny - 1);
}

int Solver::posToCell(vec2 pos, int nx, int ny){
    int column = (int)glm::floor(pos.x / h + (nx % 2) * 0.5f);
    int row = (int)glm::floor(pos.y / h + (ny % 2) * 0.5f);
    column += (int)glm::floor(nx * 0.5f);
    row += (int)glm::floor(ny * 0.5f);
    return coordToCell(ivec2(column, row), nx, ny);
}

vec2 Solver::cellToPos(int cell, int nx, int ny){
    ivec2 coord = cellToCoord(cell, nx);
    vec2 pos = ((vec2)coord - vec2(nx - 1, ny - 1) * 0.5f) * h;
    return pos;
}

vec2 Solver::coordToPos(ivec2 coord, int nx, int ny){
    vec2 pos = ((vec2)coord - vec2(nx - 1, ny - 1) * 0.5f) * h;
    return pos;
}

void Solver::posToCoordAndDp(vec2 pos, int nx, int ny, ivec2& coord, vec2& dp){
    vec2 g = pos / h + vec2((nx % 2) * 0.5f, (ny % 2) * 0.5f);
    vec2 flooredG = glm::floor(g);
    coord = ivec2(flooredG) + ivec2((int)glm::floor(nx * 0.5f), (int)glm::floor(ny * 0.5f));
    dp = g - flooredG;
}

void Solver::integrateParticles(){
    #pragma omp parallel for
    for (int i = 0; i < partN; i++)
    {
        partVel[i] += (vec2(0.f, -9.81f)) * dt;
        partPos[i] += partVel[i] * dt;
    }
}

void Solver::pushAppartParticles(){
    int numCells = gridX * gridY;

    vector<int> numCellParticles(numCells, 0);
    vector<int> cellOf(partN);

    for (int i = 0; i < partN; i++)
    {
        int cell = posToCell(partPos[i], gridX, gridY);
        cellOf[i] = cell;
        numCellParticles[cell]++;
    }

    vector<int> cellEnd(numCells, 0);
    vector<int> firstCellParticle(numCells + 1, 0);
    int first = 0;
    for (int i = 0; i < numCells; i++)
    {
        first += numCellParticles[i];
        firstCellParticle[i] = first;
        cellEnd[i] = first;
    }
    firstCellParticle[numCells] = first;

    vector<int> cellParticleIds(partN);
    for (int i = 0; i < partN; i++)
    {
        int cell = cellOf[i];
        firstCellParticle[cell]--;
        cellParticleIds[firstCellParticle[cell]] = i;
    }

    vector<vec2> corrections(partN, vec2(0.f));
    vector<int> numCorrections(partN, 0);

    const int numIters = 2;
    const float minDist = 2.0f * radius;
    const float minDist2 = minDist * minDist;

    #pragma omp parallel
    {
        for (int iter = 0; iter < numIters; iter++)
        {
            #pragma omp for
            for (int i = 0; i < partN; i++)
            {
                corrections[i] = vec2(0.f);
                numCorrections[i] = 0;
            }

            #pragma omp for
            for (int i = 0; i < partN; i++)
            {
                int cell = cellOf[i];
                ivec2 coord = cellToCoord(cell, gridX);

                for (int x = -1; x <= 1; x++)
                {
                    for (int y = -1; y <= 1; y++)
                    {
                        int cx = coord.x + x;
                        int cy = coord.y + y;
                        if (cx < 0 || cx >= gridX || cy < 0 || cy >= gridY) continue;

                        int newCell = coordToCell(ivec2(cx, cy), gridX, gridY);
                        int rangeStart = firstCellParticle[newCell];
                        int rangeEnd = cellEnd[newCell];

                        for (int k = rangeStart; k < rangeEnd; k++)
                        {
                            int j = cellParticleIds[k];
                            if (j == i) continue;

                            vec2 diff = partPos[i] - partPos[j];
                            float dist2 = length2(diff);
                            if (dist2 < minDist2 && dist2 > 0.f)
                            {
                                float dist = sqrt(dist2);
                                vec2 push = 0.7f * 0.5f * diff * (minDist - dist) / dist;
                                corrections[i] += push;
                                numCorrections[i]++;
                            }
                        }
                    }
                }
            }

            #pragma omp for
            for (int i = 0; i < partN; i++)
                if (numCorrections[i] > 0)
                    partPos[i] += corrections[i] / (float)numCorrections[i];
        }
    }
}

void Solver::particleCollisions(){
    vec2 minPos = cellToPos(0, gridX, gridY);
    vec2 maxPos = cellToPos(gridX * gridY - 1, gridX, gridY);
    #pragma omp parallel for
    for (int i = 0; i < partN; i++)
    {
        vec2 pos = partPos[i];

        vec2 diff = pos - obstaclePos;
        float dist2 = glm::length2(diff);
        if (dist2 < (radius + obstacleRadius) * (radius + obstacleRadius)){
            float d = sqrt(dist2);
            vec2 dir = diff / d;
            partPos[i] += (radius + obstacleRadius - d) * dir;
            partVel[i] = partVel[i] - glm::max(0.f, dot(partVel[i], dir)) * dir + obstacleVel;
        }

        if (pos.x < minPos.x + 1e-2f){
            partPos[i].x = minPos.x + 1e-2f;
            partVel[i].x = - partVel[i].x * 0.2f;
        }
        else if (pos.x > maxPos.x - 1e-2f){
            partPos[i].x = maxPos.x - 1e-2f;
            partVel[i].x = 0.f;
        }
        if (pos.y < minPos.y + 1e-2f){
            partPos[i].y = minPos.y + 1e-2f;
            partVel[i].y = 0.f;
        }
        else if (pos.y > maxPos.y - 1e-2f){
            partPos[i].y = maxPos.y - 1e-2f;
            partVel[i].y = 0.f;
        }
    }
}

void Solver::particlesToGrid(){
    isAir.clear();
    isAir = vector<bool>(gridX * gridY, true);
    rY.clear();
    rX.clear();
    velY.clear();
    velX.clear();
    rY = vector<float>(gridX * (gridY + 1), 0.f);
    rX = vector<float>((gridX + 1) * gridY, 0.f);
    velY = vector<float>(gridX * (gridY + 1), 0.f);
    velX = vector<float>((gridX + 1) * gridY, 0.f);
    for (int i = 0; i < partN; i++)
    {
        vec2 currentPos = partPos[i];
        vec2 vel = partVel[i];
        isAir[posToCell(partPos[i], gridX, gridY)] = false;

        {
            vec2 pos = currentPos;
            // ivec2 coord;
            // vec2 dp;
            // posToCoordAndDp(pos - vec2(h * 0.5f, 0.f), gridX, gridY, coord, dp);

            // int cell = posToCell(pos - vec2(h * 0.5f, 0.f), gridX, gridY);
            // ivec2 coord = cellToCoord(cell, gridX);
            // vec2 cellCenter = cellToPos(cell, gridX, gridY) - vec2(0.f, h * 0.5f);
            // vec2 dp = (pos - cellCenter) / h;
            int cell = posToCell(pos - vec2(h * 0.5f, 0.f), gridX, gridY);
            ivec2 coord = cellToCoord(cell, gridX);
            vec2 cellCenter = cellToPos(cell, gridX, gridY) - vec2(0.f, h * 0.5f);
            vec2 dp = (pos - cellCenter) / h;

            float w1 = (1.f - dp.x) * (1.f - dp.y);
            float w2 = dp.x * (1.f - dp.y);
            float w3 = dp.x * dp.y;
            float w4 = (1.f - dp.x) * dp.y;
    
            int bottomLeft = coordToCell(coord + ivec2(0, 0), gridX, gridY+1);
            int bottomRight = coordToCell(coord + ivec2(1, 0), gridX, gridY+1);
            int topRight = coordToCell(coord + ivec2(1, 1), gridX, gridY+1);
            int topLeft = coordToCell(coord + ivec2(0, 1), gridX, gridY+1);
            
            velY[bottomLeft] += w1 * vel.y;
            velY[bottomRight] += w2 * vel.y;
            velY[topRight] += w3 * vel.y;
            velY[topLeft] += w4 * vel.y;
            rY[bottomLeft] += w1;
            rY[bottomRight] += w2;
            rY[topRight] += w3;
            rY[topLeft] += w4;
        }
        {
            vec2 pos = currentPos;
            // ivec2 coord;
            // vec2 dp;
            // posToCoordAndDp(pos, gridX, gridY, coord, dp);
            int cell = posToCell(pos - vec2(0.f, h * 0.5f), gridX, gridY);
            ivec2 coord = cellToCoord(cell, gridX);
            vec2 cellCenter = cellToPos(cell, gridX, gridY) - vec2(h * 0.5f, 0.f);
            vec2 dp = (pos - cellCenter) / h;
            
            float w1 = (1.f - dp.x) * (1.f - dp.y);
            float w2 = dp.x * (1.f - dp.y);
            float w3 = dp.x * dp.y;
            float w4 = (1.f - dp.x) * dp.y;

            int bottomLeft = coordToCell(coord + ivec2(0, 0), gridX+1, gridY);
            int bottomRight = coordToCell(coord + ivec2(1, 0), gridX+1, gridY);
            int topRight = coordToCell(coord + ivec2(1, 1), gridX+1, gridY);
            int topLeft = coordToCell(coord + ivec2(0, 1), gridX+1, gridY);

            velX[bottomLeft] += w1 * vel.x;
            velX[bottomRight] += w2 * vel.x;
            velX[topRight] += w3 * vel.x;
            velX[topLeft] += w4 * vel.x;
            rX[bottomLeft] += w1;
            rX[bottomRight] += w2;
            rX[topRight] += w3;
            rX[topLeft] += w4;
        }
    }

    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < (gridX + 1) * gridY; i++)
        {
            float r = rX[i];
            if (r > 0) velX[i] /= r;
        }
        #pragma omp for
        for (int i = 0; i < gridX * (gridY + 1); i++)
        {
            float r = rY[i];
            if (r > 0) velY[i] /= r;
        }
    
        // #pragma omp for
        // for (int i = 0; i < gridX; i++)
        // {
        //     velY[i] = velY[i + gridX];
        //     velY[i + gridX * gridY] = velY[i + gridX * (gridY - 1)];
        // }
        // #pragma omp for
        // for (int j = 0; j < gridY; j++)
        // {
        //     velX[j * (gridX + 1)] = velX[1 + j * (gridX + 1)];
        //     velX[gridX + j * (gridX + 1)] = velX[gridX - 1 + j * (gridX + 1)];
        // }

        #pragma omp for
        for (int i = 0; i < gridX; i++)
        {
            velY[i] = 0.3f;
            velY[i + gridX * gridY] = -0.3f;
        }
        #pragma omp for
        for (int j = 0; j < gridY; j++)
        {
            velX[j * (gridX + 1)] = 0.3f;
            velX[gridX + j * (gridX + 1)] = -0.3f;
        }
    }
}

void Solver::solveIncompressibility(int iterations){
    oldVelX = velX;
    oldVelY = velY;
    vector<ivec2> redCoords = {};
    vector<ivec2> blackCoords = {};
    for (int i = 0; i < gridX; i++)
    {
        for (int j = 0; j < gridY; j++)
        {
            if (isAir[coordToCell(ivec2(i, j), gridX, gridY)]) continue;
            if ((i + j) % 2 == 0) redCoords.push_back(ivec2(i, j));
            else blackCoords.push_back(ivec2(i, j));
        }
    }
    auto solveCell = [&](ivec2 coord){
        int i = coord.x; int j = coord.y;

        float s0 = (float)(j-1 >= 0/* && !isWall[coordToCell(ivec2(i, j-1), gridX, gridY)]*/);
        float s1 = (float)(j+1 < gridY/* && !isWall[coordToCell(ivec2(i, j+1), gridX, gridY)]*/);
        float s2 = (float)(i-1 >= 0/* && !isWall[coordToCell(ivec2(i-1, j), gridX, gridY)]*/);
        float s3 = (float)(i+1 < gridX/* && !isWall[coordToCell(ivec2(i+1, j), gridX, gridY)]*/);
        //s2 = 0; s3 = 0;
        float s = s0 + s1 + s2 + s3;
        if (s <= 0) return;

        int bottom = coordToCell(ivec2(i, j), gridX, gridY+1);
        int top = coordToCell(ivec2(i, j+1), gridX, gridY+1);
        int left = coordToCell(ivec2(i, j), gridX+1, gridY);
        int right = coordToCell(ivec2(i+1, j), gridX+1, gridY);

        float d = s1 * velY[top] - s0 * velY[bottom];
        d += s3 * velX[right] - s2 * velX[left];
        d *= 1.9f; // overrelaxation
        
        float rho = s2 * rX[left] + s3 * rX[right];
        rho += s0 * rY[bottom] + s1 * rY[top];
        rho /= s;
        d -= glm::max(rho - 1.5f, 0.f); // drift compensation
        
        velY[bottom] += d * s0 / s;
        velY[top] -= d * s1 / s;
        velX[left] += d * s2 / s;
        velX[right] -= d * s3 / s;
    };
    #pragma omp parallel
    {
        for (int k = 0; k < iterations; k++)
        {
            #pragma omp for
            for (int idx = 0; idx < (int)redCoords.size(); idx++)
                solveCell(redCoords[idx]);

            #pragma omp for
            for (int idx = 0; idx < (int)blackCoords.size(); idx++)
                solveCell(blackCoords[idx]);
        }
    }
}

void Solver::gridToParticles(){
    #pragma omp parallel for
    for (int i = 0; i < partN; i++)
    {
        vec2 currentPos = partPos[i];
        {
            vec2 pos = currentPos;
            // ivec2 coord;
            // vec2 dp;
            // posToCoordAndDp(pos, gridX, gridY, coord, dp);
            int cell = posToCell(pos - vec2(h * 0.5f, 0.f), gridX, gridY);
            ivec2 coord = cellToCoord(cell, gridX);
            vec2 cellCenter = cellToPos(cell, gridX, gridY) - vec2(0.f, h * 0.5f);
            vec2 dp = (pos - cellCenter) / h;
            
            float s1 = !isAir[coordToCell(coord + ivec2(0,0), gridX, gridY)] || (coord.y + 1 <= gridY && !isAir[coordToCell(coord + ivec2(0,1), gridX, gridY)]);
            float s2 = !isAir[coordToCell(coord + ivec2(1,0), gridX, gridY)] || (coord.y + 1 <= gridY && !isAir[coordToCell(coord + ivec2(1,1), gridX, gridY)]);
            float s3 = !isAir[coordToCell(coord + ivec2(1,1), gridX, gridY)] || (coord.y + 2 <= gridY && !isAir[coordToCell(coord + ivec2(1,2), gridX, gridY)]);
            float s4 = !isAir[coordToCell(coord + ivec2(0,1), gridX, gridY)] || (coord.y + 2 <= gridY && !isAir[coordToCell(coord + ivec2(0,2), gridX, gridY)]);

            float w1 = s1 * (1.f - dp.x) * (1.f - dp.y);
            float w2 = s2 * dp.x * (1.f - dp.y);
            float w3 = s3 * dp.x * dp.y;
            float w4 = s4 * (1.f - dp.x) * dp.y;
            float sumW = w1 + w2 + w3 + w4;
            if (sumW <= 0) continue;
    
            int bottomLeft = coordToCell(coord + ivec2(0, 0), gridX, gridY+1);
            int bottomRight = coordToCell(coord + ivec2(1, 0), gridX, gridY+1);
            int topRight = coordToCell(coord + ivec2(1, 1), gridX, gridY+1);
            int topLeft = coordToCell(coord + ivec2(0, 1), gridX, gridY+1);
    
            float pic = velY[bottomLeft] * w1 + velY[bottomRight] * w2 + velY[topRight] * w3 + velY[topLeft] * w4;
            float flip = pic - (oldVelY[bottomLeft] * w1 + oldVelY[bottomRight] * w2 + oldVelY[topRight] * w3 + oldVelY[topLeft] * w4);
            pic /= sumW;
            flip /= sumW;
            partVel[i].y += flip;
            partVel[i].y = glm::mix(pic, partVel[i].y, 0.9f);
        }
        {
            vec2 pos = currentPos;
            // ivec2 coord;
            // vec2 dp;
            // posToCoordAndDp(pos, gridX, gridY, coord, dp);
            int cell = posToCell(pos - vec2(0.f, h * 0.5f), gridX, gridY);
            ivec2 coord = cellToCoord(cell, gridX);
            vec2 cellCenter = cellToPos(cell, gridX, gridY) - vec2(h * 0.5f, 0.f);
            vec2 dp = (pos - cellCenter) / h;

            float s1 = !isAir[coordToCell(coord + ivec2(0,0), gridX, gridY)] || (coord.x + 1 <= gridX && !isAir[coordToCell(coord + ivec2(1,0), gridX, gridY)]);
            float s2 = !isAir[coordToCell(coord + ivec2(1,0), gridX, gridY)] || (coord.x + 1 <= gridX && !isAir[coordToCell(coord + ivec2(2,0), gridX, gridY)]);
            float s3 = !isAir[coordToCell(coord + ivec2(1,1), gridX, gridY)] || (coord.x + 2 <= gridX && !isAir[coordToCell(coord + ivec2(2,1), gridX, gridY)]);
            float s4 = !isAir[coordToCell(coord + ivec2(0,1), gridX, gridY)] || (coord.x + 2 <= gridX && !isAir[coordToCell(coord + ivec2(1,1), gridX, gridY)]);
            
            float w1 = s1 * (1.f - dp.x) * (1.f - dp.y);
            float w2 = s2 * dp.x * (1.f - dp.y);
            float w3 = s3 * dp.x * dp.y;
            float w4 = s4 * (1.f - dp.x) * dp.y;
            float sumW = w1 + w2 + w3 + w4;
            if (sumW <= 0) continue;
        
            int bottomLeft = coordToCell(coord + ivec2(0, 0), gridX+1, gridY);
            int bottomRight = coordToCell(coord + ivec2(1, 0), gridX+1, gridY);
            int topRight = coordToCell(coord + ivec2(1, 1), gridX+1, gridY);
            int topLeft = coordToCell(coord + ivec2(0, 1), gridX+1, gridY);
        
            float pic = velX[bottomLeft] * w1 + velX[bottomRight] * w2 + velX[topRight] * w3 + velX[topLeft] * w4;
            float flip = pic - (oldVelX[bottomLeft] * w1 + oldVelX[bottomRight] * w2 + oldVelX[topRight] * w3 + oldVelX[topLeft] * w4);
            pic /= sumW;
            flip /= sumW;
            partVel[i].x += flip;
            partVel[i].x = glm::mix(pic, partVel[i].x, 0.9f);
        }
    }
}


void Solver::updateFlip(){
    integrateParticles();
    particleCollisions();
    pushAppartParticles();
    particleCollisions();
    
    particlesToGrid();
    solveIncompressibility(80);
    gridToParticles();
}

vector<vec4> Solver::getGrid(float width){
    vector<vec4> grid = {};
    for (int i = 0; i < gridX; i++)
    {
        vec2 pos = cellToPos(i, gridX, gridY);
        grid.push_back(vec4(pos.x - h * 0.5f, 0.f, width, gridY * 0.5f * h));
    }
    for (int j = 0; j < gridY; j++)
    {
        vec2 pos = cellToPos(j * gridX, gridX, gridY);
        grid.push_back(vec4(0.f, pos.y - h * 0.5f, gridX * 0.5f * h, width));
    }
    return grid;
}

vector<vec4> Solver::getCells(){
    vector<vec4> cells = vector<vec4>(gridX * gridY, vec4(0.f));
    #pragma omp parallel for
    for (int cell = 0; cell < gridX * gridY; cell++)
    {
        cells[cell] = vec4(cellToPos(cell, gridX, gridY), h * 0.5f, h * 0.5f);
    }
    return cells;
}

vector<vec4> Solver::getCellColors(){
    vector<vec4> colors = vector<vec4>(gridX * gridY, vec4(0,0,0,1));
    #pragma omp parallel for
    for (int x = 0; x < gridX; x++)
    {
        for (int y = 0; y < gridY; y++)
        {
            // int cellX = coordToCell(ivec2(x,y), gridX+1, gridY);
            // float vx = velX[cellX];
            int cellY = coordToCell(ivec2(x,y), gridX, gridY+1);
            float vy = velY[cellY];
            colors[coordToCell(ivec2(x,y), gridX, gridY)] = vec4(glm::max(0.f,vy) / 50, -glm::min(0.f,vy) / 50, 0.f, 1.f);
        }
    }
    // for (int i = 0; i < partN; i++)
    // {
    //     int cell = posToCell(partPos[i], gridX, gridY);
    //     colors[cell] += vec4(0.2f, 0.2f, 0.f, 0.f);
    // }
    
    return colors;
}

void Solver::updateObstacle(vec2 pos, vec2 vel, float rad){
    obstaclePos = pos;
    obstacleVel = vel;
    obstacleRadius = rad;
}