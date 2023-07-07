/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "glMatrix.h"
#include <math.h>
#include <strings.h>
#include <stdio.h>

void glMatrix_mat4_identity(mat4 out) {
    for (unsigned i = 0; i < 4; i++) {
        for (unsigned j = 0; j < 4; j++) {
            out[i][j] = 0;
        }
        out[i][i] = 1;
    }
}

void glMatrix_mat4_multiply(mat4 out, mat4 a, mat4 b) {
    for (unsigned i = 0; i < 4; i++) {
        for (unsigned j = 0; j < 4; j++) {
            out[i][j] = 0;
            for (unsigned d = 0; d < 4; d++) {
                out[i][j] += a[i][d] * b[d][j];
            }
        }
    }
}

void glMatrix_vec4_transform(vec4 out, mat4 mat, vec4 vec) {
    for (unsigned i = 0; i < 4; i++) {
        out[i] = 0;
        for (unsigned j = 0; j < 4; j++) {
            out[i] += mat[i][j] * vec[j];
        }
    }
}

void glMatrix_mat4_perspective(mat4 out, float fovy, float aspect, float near, float far) {
    float f = 1.f / tanf(fovy / 2.f);
    float nf = 1.f / (near - far);

    out[0][0] = f / aspect;
    out[0][1] = 0.f;
    out[0][2] = 0.f;
    out[0][3] = 0.f;

    out[1][0] = 0.f;
    out[1][1] = f;
    out[1][2] = 0.f;
    out[1][3] = 0.f;

    out[2][0] = 0.f;
    out[2][1] = 0.f;
    out[2][2] = (far + near) * nf;
    out[2][3] = -1.f;

    out[3][0] = 0.f;
    out[3][1] = 0.f;
    out[3][2] = 2.f * far * near * nf;
    out[3][3] = 0.f;
}

void glMatrix_mat4_rotateX(mat4 out, mat4 mat, float angle) {
    float s = sinf(angle);
    float c = cosf(angle);
    float tmp[2][4] = {
        {mat[1][0], mat[1][1], mat[1][2], mat[1][3], },
        {mat[2][0], mat[2][1], mat[2][2], mat[2][3], },
    };

    if (mat != out) {
        // differ, copy the unchanged rows
        for (unsigned i = 0; i < 4; i++) {
            out[0][i] = mat[0][i];
            out[3][i] = mat[3][i];
        }
    }

    // perform rotation
    for (unsigned i = 0; i < 4; i++) {
        out[1][i] = tmp[0][i] * c + tmp[1][i] * s;
    }
    for (unsigned i = 0; i < 4; i++) {
        out[2][i] = tmp[1][i] * c - tmp[0][i] * s;
    }
}

void glMatrix_mat4_rotateY(mat4 out, mat4 mat, float angle) {
    float s = sinf(angle);
    float c = cosf(angle);
    float tmp[2][4] = {
        {mat[0][0], mat[0][1], mat[0][2], mat[0][3], },
        {mat[2][0], mat[2][1], mat[2][2], mat[2][3], },
    };

    if (mat != out) {
        // differ, copy
        for (unsigned i = 0; i < 4; i++) {
            out[1][i] = mat[1][i];
            out[3][i] = mat[3][i];
        }
    }
    for (unsigned i = 0; i < 4; i++) {
        out[0][i] = tmp[0][i] * c - tmp[1][i] * s;
    }
    for (unsigned i = 0; i < 4; i++) {
        out[2][i] = tmp[0][i] * s + tmp[1][i] * c;
    }
}

void glMatrix_mat4_rotateZ(mat4 out, mat4 mat, float angle) {
    float s = sinf(angle);
    float c = cosf(angle);
    float tmp[2][4] = {
        {mat[0][0], mat[0][1], mat[0][2], mat[0][3], },
        {mat[1][0], mat[1][1], mat[1][2], mat[1][3], },
    };

    if (mat != out) {
        // differ, copy
        for (unsigned i = 0; i < 4; i++) {
            out[2][i] = mat[2][i];
            out[3][i] = mat[3][i];
        }
    }
    for (unsigned i = 0; i < 4; i++) {
        out[0][i] = tmp[0][i] * c + tmp[1][i] * s;
    }
    for (unsigned i = 0; i < 4; i++) {
        out[1][i] = tmp[1][i] * c - tmp[0][i] * s;
    }
}

void glMatrix_mat4_print(const mat4 mat) {
    for (unsigned i = 0; i < 4; i++) {
        for (unsigned j = 0; j < 4; j++) {
            printf("%f ", mat[i][j]);
        }
        printf("\n");
    }
}
