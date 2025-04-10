// Copyright (c) Meta Platforms, Inc. and affiliates.
// All rights reserved.
//
// This source code is licensed under the license found in the
// LICENSE file in the root directory of this source tree.

#if defined(__aarch64__) || defined(__ARM_NEON)

#include <arm_neon.h>
#include <vector>

#include <gtest/gtest.h>
#include <torchao/experimental/kernels/cpu/aarch64/matmul/matmul.h>
#include <torchao/experimental/kernels/cpu/aarch64/tests/test_utils.h>

float kTol = 0.0001;

template <
    bool a_has_zeros,
    bool b_has_zeros,
    bool a_transposed,
    bool b_transposed>
struct test_channelwise_8bit_channelwise_8bit_b {
  static void Run(int m, int k, int n);
};

template <bool a_has_zeros, bool b_has_zeros>
struct test_channelwise_8bit_channelwise_8bit_b<
    a_has_zeros,
    b_has_zeros,
    false,
    true> {
  static void Run(int m, int k, int n, int stride = 1) {
    auto test_case =
        torchao::channelwise_8bit_a_channelwise_8bit_b_qmatmul_test_case::
            generate(m, k, n, a_has_zeros, a_has_zeros, false, true, stride);

    using namespace torchao::kernels::cpu::aarch64::quantized_matmul::
        channelwise_8bit_a_channelwise_8bit_b_1x8x16_f32_neondot;

    std::vector<float> output(m * n);
    kernel<a_has_zeros, b_has_zeros, false, true>(
        m,
        n,
        k,
        test_case.lhs_qvals.data(),
        k * stride /*lsh_stride_m*/,
        test_case.rhs_qvals.data(),
        k * stride /*rsh_stride_n*/,
        output.data(),
        n /*out_stride_n*/,
        test_case.lhs_zeros.data(),
        test_case.rhs_zeros.data(),
        test_case.lhs_scales.data(),
        test_case.rhs_scales.data(),
        stride, /*lhs qparams stride*/
        stride /*rhs qparams stride*/);

    for (int i = 0; i < m * n; i++) {
      EXPECT_NEAR(output[i], test_case.expected_output[i], kTol);
    }
  }
};

template <bool a_has_zeros, bool b_has_zeros>
struct test_channelwise_8bit_channelwise_8bit_b<
    a_has_zeros,
    b_has_zeros,
    false,
    false> {
  static void Run(int m, int k, int n, int stride = 1) {
    auto test_case =
        torchao::channelwise_8bit_a_channelwise_8bit_b_qmatmul_test_case::
            generate(m, k, n, a_has_zeros, a_has_zeros, false, false);

    using namespace torchao::kernels::cpu::aarch64::quantized_matmul::
        channelwise_8bit_a_channelwise_8bit_b_1x16x16_f32_smlal;

    std::vector<float> output(m * n);
    kernel<a_has_zeros, b_has_zeros, false, false>(
        m,
        n,
        k,
        test_case.lhs_qvals.data(),
        k /*lsh_stride_m*/,
        test_case.rhs_qvals.data(),
        n /*rsh_stride_n*/,
        output.data(),
        n /*out_stride_n*/,
        test_case.lhs_zeros.data(),
        test_case.rhs_zeros.data(),
        test_case.lhs_scales.data(),
        test_case.rhs_scales.data(),
        stride, /*lhs qparams stride*/
        stride /*rhs qparams stride*/);

    for (int i = 0; i < m * n; i++) {
      EXPECT_NEAR(output[i], test_case.expected_output[i], kTol);
    }
  }
};

TEST(test_channelwise_8bit_channelwise_8bit_b, TransposedBWithZeroPoints) {
  test_channelwise_8bit_channelwise_8bit_b<
      true /*a_has_zeros*/,
      true /*b_has_zeros*/,
      false /*a_transposed*/,
      true /*b_transposed*/>::
      Run(
          /*m=*/1, /*k=*/128, /*n=*/16);
}

TEST(test_channelwise_8bit_channelwise_8bit_b, TransposeBWithZeroPointsLargeM) {
  test_channelwise_8bit_channelwise_8bit_b<
      true /*a_has_zeros*/,
      true /*b_has_zeros*/,
      false /*a_transposed*/,
      true /*b_transposed*/>::
      Run(
          /*m=*/4, /*k=*/128, /*n=*/16);
}

TEST(
    test_channelwise_8bit_channelwise_8bit_b,
    TransposedBWithZeroPointsOddSizes) {
  test_channelwise_8bit_channelwise_8bit_b<
      true /*a_has_zeros*/,
      true /*b_has_zeros*/,
      false /*a_transposed*/,
      true /*b_transposed*/>::
      Run(
          /*m=*/4, /*k=*/37, /*n=*/24);
}

TEST(
    test_channelwise_8bit_channelwise_8bit_b,
    TransposedBWithZeroPointsOddSizes2) {
  test_channelwise_8bit_channelwise_8bit_b<
      true /*a_has_zeros*/,
      true /*b_has_zeros*/,
      false /*a_transposed*/,
      true /*b_transposed*/>::
      Run(
          /*m=*/4, /*k=*/37, /*n=*/19);
}

TEST(
    test_channelwise_8bit_channelwise_8bit_b,
    TransposedBWithZeroPointsStrided) {
  test_channelwise_8bit_channelwise_8bit_b<
      true /*a_has_zeros*/,
      true /*b_has_zeros*/,
      false /*a_transposed*/,
      true /*b_transposed*/>::
      Run(
          /*m=*/1, /*k=*/128, /*n=*/16, 5);
}

TEST(
    test_channelwise_8bit_channelwise_8bit_b,
    TransposedBWithZeroPointsOddSizes2Strided) {
  test_channelwise_8bit_channelwise_8bit_b<
      true /*a_has_zeros*/,
      true /*b_has_zeros*/,
      false /*a_transposed*/,
      true /*b_transposed*/>::
      Run(
          /*m=*/4, /*k=*/37, /*n=*/19, 10);
}

TEST(
    test_channelwise_8bit_channelwise_8bit_b,
    TransposedBWithZeroPointsOddSizes2Strided2) {
  test_channelwise_8bit_channelwise_8bit_b<
      true /*a_has_zeros*/,
      true /*b_has_zeros*/,
      false /*a_transposed*/,
      true /*b_transposed*/>::
      Run(
          /*m=*/3, /*k=*/64, /*n=*/24, 7);
}

TEST(test_channelwise_8bit_channelwise_8bit_b, NoTransposedWithZeroPoints) {
  test_channelwise_8bit_channelwise_8bit_b<
      true /*a_has_zeros*/,
      true /*b_has_zeros*/,
      false /*a_transposed*/,
      false /*b_transposed*/>::
      Run(
          /*m=*/1, /*k=*/128, /*n=*/16);
}

TEST(
    test_channelwise_8bit_channelwise_8bit_b,
    NoTransposedWithZeroPointsLargeM) {
  test_channelwise_8bit_channelwise_8bit_b<
      true /*a_has_zeros*/,
      true /*b_has_zeros*/,
      false /*a_transposed*/,
      false /*b_transposed*/>::
      Run(
          /*m=*/4, /*k=*/128, /*n=*/16);
}

TEST(
    test_channelwise_8bit_channelwise_8bit_b,
    NoTransposedWithZeroPointsOddSizes) {
  test_channelwise_8bit_channelwise_8bit_b<
      true /*a_has_zeros*/,
      true /*b_has_zeros*/,
      false /*a_transposed*/,
      false /*b_transposed*/>::
      Run(
          /*m=*/4, /*k=*/37, /*n=*/24);
}

TEST(
    test_channelwise_8bit_channelwise_8bit_b,
    NoTransposedWithZeroPointsOddSizes2) {
  test_channelwise_8bit_channelwise_8bit_b<
      true /*a_has_zeros*/,
      true /*b_has_zeros*/,
      false /*a_transposed*/,
      false /*b_transposed*/>::
      Run(
          /*m=*/4, /*k=*/37, /*n=*/19);
}

class FP32A_QuantizedB_FP32C_Test : public ::testing::TestWithParam<bool> {
 public:
  int m;
  int k;
  int n;
  int stride;

  bool rhs_has_zeros;
  bool lhs_is_transposed;
  bool rhs_is_transposed;

  std::vector<float> init_output;
  std::vector<float> expected_output;

  std::vector<float> lhs;

  std::vector<float> rhs;
  std::vector<int8_t> rhs_qvals;
  std::vector<float> rhs_scales;
  std::vector<int8_t> rhs_zeros;

  void generate(
      int m_,
      int k_,
      int n_,
      bool rhs_has_zeros_,
      bool lhs_is_transposed_,
      bool rhs_is_transposed_,
      int stride_ = 1) {
    // Here stride is only applicable to rhs
    // and it means that k elements are stride * napart for k x n matrix
    // and stride apart for n x k matrix
    assert(!lhs_is_transposed_);
    assert(rhs_has_zeros_);
    m = m_;
    k = k_;
    n = n_;
    stride = stride_;
    rhs_has_zeros = rhs_has_zeros_;
    lhs_is_transposed = lhs_is_transposed_;
    rhs_is_transposed = rhs_is_transposed_;

    assert(!rhs_is_transposed || stride == 1);

    // Generate activations
    lhs = torchao::get_random_vector(m * k, -1.0, 1.0);

    // The strange thing this is doing is that instead of quantizing
    // each output channel separately, we are quantizing each input channel
    // Reason why we do !rhs_is_transposed is because
    // we actually want k x n matrix not n x k matrix
    // because each input channel is quantized separately
    std::tie(rhs, rhs_qvals, rhs_scales, rhs_zeros) =
        torchao::test_utils::generate_per_token_quantized_tensor(
            k * stride, n, rhs_is_transposed);

    // Compute expected output
    init_output = torchao::get_random_vector(m * n, -1.0, 1.0);

    assert(init_output.size() == m * n);
    assert(lhs.size() == m * k);
    assert(rhs.size() == n * stride * k);
    assert(rhs_qvals.size() == n * stride * k);
    assert(rhs_scales.size() == k * stride);
    assert(rhs_zeros.size() == k * stride);
  }

  void execute(float beta) {
    // Compute expected output
    expected_output = init_output;

    for (int m_idx = 0; m_idx < m; m_idx++) {
      for (int n_idx = 0; n_idx < n; n_idx++) {
        float res = 0.0;
        for (int k_idx = 0; k_idx < k; k_idx++) {
          int lhs_idx = m_idx * k + k_idx;
          int rhs_idx = k_idx * stride * n + n_idx;
          if (rhs_is_transposed) {
            rhs_idx = n_idx * k * stride + k_idx * stride;
          }
          float rhs_dequant = rhs_scales[k_idx * stride] *
              (static_cast<int16_t>(rhs_qvals[rhs_idx]) -
               static_cast<int16_t>(rhs_zeros[k_idx * stride]));

          res += lhs[lhs_idx] * rhs_dequant;
        }
        expected_output[m_idx * n + n_idx] =
            expected_output[m_idx * n + n_idx] * beta + res;
      }
    }
  }

  float beta() const {
    return GetParam();
  }
};

static void test_fp32_a_input_channelwise_8bit_b(
    int m,
    int k,
    int n,
    float beta,
    FP32A_QuantizedB_FP32C_Test& test_case,
    int stride = 1) {
  test_case.execute(beta);

  using namespace torchao::kernels::cpu::aarch64::quantized_matmul::
      fp32_a_input_channelwise_8bit_b_1x16x4_f32;

  std::vector<float> output(test_case.init_output);
  kernel<true, false, false>(
      m,
      n,
      k,
      test_case.lhs.data(),
      k /*lhs_stride_m*/,
      test_case.rhs_qvals.data(),
      n * stride /*rhs_stride_n*/,
      output.data(),
      n /*out_stride_n*/,
      test_case.rhs_zeros.data(),
      test_case.rhs_scales.data(),
      beta,
      stride /*rhs qparams stride*/);

  for (int i = 0; i < m * n; i++) {
    EXPECT_NEAR(output[i], test_case.expected_output[i], kTol);
  }
}

TEST_P(FP32A_QuantizedB_FP32C_Test, BTranposedWithZeroPoints) {
  generate(1, 128, 16, true, false, false);
  test_fp32_a_input_channelwise_8bit_b(
      /*m=*/1, /*k=*/128, /*n=*/16, beta(), *this);
}

TEST_P(FP32A_QuantizedB_FP32C_Test, BTranposedWithZeroPointsLargeM) {
  generate(4, 128, 16, true, false, false);
  test_fp32_a_input_channelwise_8bit_b(
      /*m=*/4, /*k=*/128, /*n=*/16, beta(), *this);
}

TEST_P(FP32A_QuantizedB_FP32C_Test, BTranposedWithZeroPointsOddSizes) {
  generate(4, 37, 24, true, false, false);
  test_fp32_a_input_channelwise_8bit_b(
      /*m=*/4, /*k=*/37, /*n=*/24, beta(), *this);
}

TEST_P(FP32A_QuantizedB_FP32C_Test, BTranposedWithZeroPointsOddSizes2) {
  generate(4, 37, 19, true, false, false);
  test_fp32_a_input_channelwise_8bit_b(
      /*m=*/4, /*k=*/37, /*n=*/19, beta(), *this);
}

TEST_P(FP32A_QuantizedB_FP32C_Test, BTranposedWithZeroPointsOddSizes3) {
  generate(4, 27, 21, true, false, false);
  test_fp32_a_input_channelwise_8bit_b(
      /*m=*/4, /*k=*/27, /*n=*/21, beta(), *this);
}

TEST_P(FP32A_QuantizedB_FP32C_Test, BTranposedWithZeroPointsAlpha) {
  generate(1, 128, 16, true, false, false);
  test_fp32_a_input_channelwise_8bit_b(
      /*m=*/1, /*k=*/128, /*n=*/16, beta(), *this);
}

TEST_P(FP32A_QuantizedB_FP32C_Test, BTranposedWithZeroPointsWithStrides) {
  stride = 5;
  generate(1, 128, 16, true, false, false, stride);
  test_fp32_a_input_channelwise_8bit_b(
      /*m=*/1, /*k=*/128, /*n=*/16, beta(), *this, stride);
}

TEST_P(FP32A_QuantizedB_FP32C_Test, BTranposedWithZeroPointsOddSizes2Strides) {
  stride = 11;
  generate(7, 37, 19, true, false, false, stride);
  test_fp32_a_input_channelwise_8bit_b(
      /*m=*/7, /*k=*/37, /*n=*/19, beta(), *this, stride);
}

INSTANTIATE_TEST_SUITE_P(
    F32AInt8BFP32CTest,
    FP32A_QuantizedB_FP32C_Test,
    ::testing::Values(0.0, 1.0, 2.69));

#endif // defined(__aarch64__) || defined(__ARM_NEON)
