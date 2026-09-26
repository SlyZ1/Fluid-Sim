#include <catch2/catch_test_macros.hpp>
#include "src/helpers/logger.hpp"
#include "src/cgs/cgs.hpp"

using namespace std;

static bool testCGS(float iter = 50, float eps = 1e-4f){
    CGS cgs = CGS();

    GLuint ABuffer = 0;
    GLuint bBuffer = 0;
    GLuint xBuffer = 0;

    const int N = 5;
    float a[] = { 36.987f, -10.254f, 2.168f, -5.723f, 5.91f, -10.254f, 35.349f, -4.102f, 4.83f, -1.345f, 2.168f, -4.102f, 31.451f, -2.339f, -10.968f, -5.723f, 4.83f, -2.339f, 33.895f, -1.718f, 5.91f, -1.345f, -10.968f, -1.718f, 53.52f };  // symétrique définie positive
    float b[] = { -60.999403f, -0.200832f, 49.8964f, -98.215783f, 103.875335f };
    float x[] = { -2.683f, 0.02f, 2.47f, -3.049f, 2.646f };

    glGenBuffers(1, &ABuffer); glGenBuffers(1, &bBuffer); glGenBuffers(1, &xBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ABuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(a), &a[0], GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(b), &b[0], GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, xBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(x), nullptr, GL_STATIC_DRAW);

    cgs.init(N, ABuffer, bBuffer, xBuffer);
    cgs.solve(iter, eps * 5e-2f);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, xBuffer);
    float result[N];
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(x), &result[0]);

    float rTr = 0;
    for (int i = 0; i < N; i++) {
        rTr += (x[i] - result[i]) * (x[i] - result[i]);
    }
    cout << rTr << endl;

    glDeleteBuffers(1, &ABuffer); glDeleteBuffers(1, &bBuffer); glDeleteBuffers(1, &xBuffer);

    return rTr < eps;
}

TEST_CASE( "CGS is computed", "[cgs]" ) {
    REQUIRE( testCGS(100, 1e-5f) );
}