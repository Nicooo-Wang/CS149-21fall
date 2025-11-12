#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>

// 方法1：位掩码（最快）
__m256 abs_ps(__m256 x) {
    __m256 mask = _mm256_castsi256_ps(_mm256_set1_epi32(0x7FFFFFFF));
    return _mm256_and_ps(x, mask);
}

void sqrtAVX(int N,
                float initialGuess,
                float values[],
                float output[])
{
  // static const float kThreshold = 0.00001f;
  __m256 kThreshold = _mm256_set_ps(0.00001f, 0.00001f, 0.00001f, 0.00001f,
                                    0.00001f, 0.00001f, 0.00001f, 0.00001f);

  // for (int i=0; i<N; i++) {
  for (int i = 0; i < N; i += 8) {
    //     float x = values[i];
    auto x = _mm256_loadu_ps(values + i);  // 使用不对齐加载
    //     float guess = initialGuess;
    __m256 guess =
        _mm256_set_ps(initialGuess, initialGuess, initialGuess, initialGuess,
                      initialGuess, initialGuess, initialGuess, initialGuess);
    __m256 ones = _mm256_set_ps(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

    //     float error = fabs(guess * guess * x - 1.f);
    __m256 error = abs_ps(
        _mm256_sub_ps(_mm256_mul_ps(_mm256_mul_ps(guess, guess), x), ones));

    //     while (error > kThreshold) {
    //         guess = (3.f * guess - x * guess * guess * guess) * 0.5f;
    //         error = fabs(guess * guess * x - 1.f);
    //     }

    // 向量化的while循环
    __m256 convergenceMask = _mm256_cmp_ps(error, kThreshold, _CMP_GT_OQ);

    while (_mm256_movemask_ps(convergenceMask) != 0) {
      // 计算新的guess: guess = (3.f * guess - x * guess * guess * guess) * 0.5f
      __m256 guess3 = _mm256_mul_ps(_mm256_set1_ps(3.0f), guess);
      __m256 guess4 =
          _mm256_mul_ps(guess, _mm256_mul_ps(guess, _mm256_mul_ps(guess, x)));
      __m256 newGuess =
          _mm256_mul_ps(_mm256_sub_ps(guess3, guess4), _mm256_set1_ps(0.5f));

      // 只对未收敛的元素更新guess
      guess = _mm256_blendv_ps(guess, newGuess, convergenceMask);

      // 计算新的error
      error = abs_ps(
          _mm256_sub_ps(_mm256_mul_ps(_mm256_mul_ps(guess, guess), x), ones));

      // 更新收敛mask
      convergenceMask = _mm256_cmp_ps(error, kThreshold, _CMP_GT_OQ);
    }

    //     output[i] = x * guess;
    __m256 result = _mm256_mul_ps(x, guess);
    _mm256_storeu_ps(output + i, result);  // 使用不对齐存储
  }

  // 处理剩余元素（N不是8的倍数时）
  for (int i = (N / 8) * 8; i < N; i++) {
    float x = values[i];
    float guess = initialGuess;

    float error = fabsf(guess * guess * x - 1.f);

    while (error > 0.00001f) {
      guess = (3.f * guess - x * guess * guess * guess) * 0.5f;
      error = fabsf(guess * guess * x - 1.f);
    }

    output[i] = x * guess;
  }
}