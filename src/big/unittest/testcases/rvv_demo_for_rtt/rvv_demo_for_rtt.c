#include <stdint.h>
#include <stddef.h>
#include <riscv_vector.h>
#include <math.h>
#include <stdio.h>

#define N 31
static float input1[N] = { -0.4325648115282207, -1.6655843782380970,
    0.1253323064748307,
    0.2876764203585489, -1.1464713506814637,
    1.1909154656429988,
    1.1891642016521031, -0.0376332765933176,
    0.3272923614086541,
    0.1746391428209245, -0.1867085776814394,
    0.7257905482933027,
    -0.5883165430141887, 2.1831858181971011,
    -0.1363958830865957,
    0.1139313135208096, 1.0667682113591888,
    0.0592814605236053,
    -0.0956484054836690, -0.8323494636500225,
    0.2944108163926404,
    -1.3361818579378040, 0.7143245518189522,
    1.6235620644462707,
    -0.6917757017022868, 0.8579966728282626,
    1.2540014216025324,
    -1.5937295764474768, -1.4409644319010200,
    0.5711476236581780,
    -0.3998855777153632
};

static float out1[N] = {};
static float out2[N] = {};

static void saxpy_golden(size_t n, const float a, const float *x, float *y)
{
    for (size_t i = 0; i < n; ++i) {
        y[i] = a * x[i] + y[i];
    }
}

static void saxpy_vec(size_t n, const float a, const float *x, float *y)
{
    size_t l;
    vfloat32m8_t vx, vy;
    for (; n > 0; n -= l) {
        l = vsetvl_e32m8(n);
        vx = vle32_v_f32m8(x, l);
        x += l;
        vy = vle32_v_f32m8(y, l);
        vy = vfmacc_vf_f32m8(vy, a, vx, l);
        vse32_v_f32m8(y, vy, l);
        y += l;
    }
}

static void sqrt_vec(size_t n, const float* x, float* y)
{
    size_t l;
    vfloat32m8_t vx, vy;
    for (; n > 0; n -= l) {
        l = vsetvl_e32m8(n);
        vx = vle32_v_f32m8(x, l);
        vy = vfsqrt_v_f32m8(vx, l);
        vse32_v_f32m8(y, vy, l);
        x += l;
        y += l;
    }
}

static void sqrt_goldern(size_t n, const float* x, float* y)
{
    for(int i = 0; i < n; ++i){
        y[i] = sqrtf(x[i]);
    }
}

static void print_result(int n, float*a, float*b)
{
    for(int i = 0; i < n; ++i)
    {
        printf("--%d, %f, %f, diff:%f\n", i, a[i], b[i], a[i] - b[i]);
    }
}

int main()
{
    // axpy demo, golden function: saxpy_golden, rvv function: saxpy_vec
    saxpy_golden(N, 2.0f, input1, out1);
    saxpy_vec(N, 2.0f, input1, out2);
    print_result(N, out1, out2);
    
    // sqrt demo, golden function: sqrt_goldern, rvv function: sqrt_vec
    for(int i = 0; i < N; ++i)
    {
        if(input1[i] < 0) input1[i] = -input1[i];
    }
    sqrt_goldern(N, input1, out1);
    sqrt_vec(N, input1,out2);
    print_result(N, out1, out2);
    return 0;
}

