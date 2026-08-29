ivec2 cellToCoord(int cell, int nx){
    int column = cell % nx;
    int row = (cell - column) / nx;
    return ivec2(column, row);
}

int coordToCell(ivec2 coord, int nx, int ny){
    return clamp(coord.x % nx + nx * coord.y, 0, nx * ny - 1);
}

int posToCell(vec2 pos, int nx, int ny){
    int column = int(floor(pos.x / h + float(nx % 2) * 0.5));
    int row = int(floor(pos.y / h + float(ny % 2) * 0.5));
    column += int(floor(nx * 0.5));
    row += int(floor(ny * 0.5));
    return coordToCell(ivec2(column, row), nx, ny);
}

ivec2 posToCoord(vec2 pos, int nx, int ny){
    int column = int(floor(pos.x / h + (nx % 2) * 0.5));
    int row = int(floor(pos.y / h + (ny % 2) * 0.5));
    column += int(floor(nx * 0.5));
    row += int(floor(ny * 0.5));
    return ivec2(column, row);
}

vec2 cellToPos(int cell, int nx, int ny){
    ivec2 coord = cellToCoord(cell, nx);
    vec2 pos = (vec2(coord) - vec2(nx - 1, ny - 1) * 0.5) * h;
    return pos;
}

vec2 coordToPos(ivec2 coord, int nx, int ny){
    vec2 pos = (vec2(coord) - vec2(nx - 1, ny - 1) * 0.5) * h;
    return pos;
}