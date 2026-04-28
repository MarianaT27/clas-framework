#include <gtest/gtest.h>

#include <array>
#include <span>
#include <vector>

#include <clas12/framework/core/ThreadLocalHistogram.hpp>

using framework::ThreadLocalHistogram1D;
using framework::ThreadLocalHistogram2D;

// =============================================================================
// ThreadLocalHistogram1D::fill_span
// =============================================================================

TEST(ColumnFill, Histogram1DSpanMatchesScalarFill) {
    const std::vector<float> values = {0.5F, 1.5F, 2.5F, 3.5F, 4.5F, 0.1F, 4.9F};

    ThreadLocalHistogram1D h_ref(5, 0.0, 5.0);
    for (float v : values) h_ref.fill(v);

    ThreadLocalHistogram1D h_span(5, 0.0, 5.0);
    h_span.fill_span(std::span<const float>(values));

    ASSERT_EQ(h_ref.entries(), h_span.entries());
    for (int i = 0; i <= h_ref.nbins() + 1; ++i) {
        EXPECT_DOUBLE_EQ(h_ref.get_bin_content(i), h_span.get_bin_content(i))
            << "bin " << i;
    }
}

TEST(ColumnFill, Histogram1DSpanSkipsNaN) {
    const std::vector<float> values = {1.0F,
                                       std::numeric_limits<float>::quiet_NaN(),
                                       2.0F, 3.0F};

    ThreadLocalHistogram1D h(5, 0.0, 5.0);
    h.fill_span(std::span<const float>(values));

    EXPECT_EQ(h.entries(), 3U);  // NaN skipped
}

TEST(ColumnFill, Histogram1DWeightedSpan) {
    const std::vector<float>  values  = {1.5F, 2.5F, 3.5F};
    const std::vector<double> weights = {1.0,  2.0,  0.5};

    ThreadLocalHistogram1D h_ref(5, 0.0, 5.0);
    for (std::size_t i = 0; i < values.size(); ++i) {
        h_ref.fill(values[i], weights[i]);
    }

    ThreadLocalHistogram1D h_span(5, 0.0, 5.0);
    h_span.fill_span(std::span<const float>(values),
                     std::span<const double>(weights));

    ASSERT_EQ(h_ref.entries(), h_span.entries());
    for (int i = 0; i <= h_ref.nbins() + 1; ++i) {
        EXPECT_DOUBLE_EQ(h_ref.get_bin_content(i), h_span.get_bin_content(i))
            << "bin " << i;
    }
}

TEST(ColumnFill, Histogram1DSpanFromEmptyVector) {
    std::vector<float> empty;
    ThreadLocalHistogram1D h(10, 0.0, 10.0);
    h.fill_span(std::span<const float>(empty));
    EXPECT_EQ(h.entries(), 0U);
}

TEST(ColumnFill, Histogram1DSpanAcceptsStdArray) {
    std::array<float, 4> arr{1.0F, 2.0F, 3.0F, 4.0F};
    ThreadLocalHistogram1D h(5, 0.0, 5.0);
    h.fill_span(std::span<const float>(arr.data(), arr.size()));
    EXPECT_EQ(h.entries(), 4U);
}

// =============================================================================
// ThreadLocalHistogram2D::fill_span
// =============================================================================

TEST(ColumnFill, Histogram2DSpanMatchesScalarFill) {
    const std::vector<float> xs = {0.5F, 1.5F, 2.5F};
    const std::vector<float> ys = {0.5F, 1.5F, 2.5F};

    ThreadLocalHistogram2D h_ref(3, 0.0, 3.0, 3, 0.0, 3.0);
    for (std::size_t i = 0; i < xs.size(); ++i) h_ref.fill(xs[i], ys[i]);

    ThreadLocalHistogram2D h_span(3, 0.0, 3.0, 3, 0.0, 3.0);
    h_span.fill_span(std::span<const float>(xs), std::span<const float>(ys));

    ASSERT_EQ(h_ref.entries(), h_span.entries());
    for (int ix = 0; ix <= 4; ++ix) {
        for (int iy = 0; iy <= 4; ++iy) {
            EXPECT_DOUBLE_EQ(h_ref.get_bin_content(ix, iy),
                             h_span.get_bin_content(ix, iy))
                << "bin " << ix << "," << iy;
        }
    }
}

TEST(ColumnFill, Histogram2DSpanHonoursShorterSpan) {
    const std::vector<float> xs = {1.0F, 2.0F, 3.0F};
    const std::vector<float> ys = {1.0F, 2.0F};  // shorter

    ThreadLocalHistogram2D h(3, 0.0, 3.0, 3, 0.0, 3.0);
    h.fill_span(std::span<const float>(xs), std::span<const float>(ys));

    EXPECT_EQ(h.entries(), 2U);  // limited by shorter span
}

TEST(ColumnFill, Histogram2DWeightedSpan) {
    const std::vector<float>  xs      = {0.5F, 1.5F, 2.5F};
    const std::vector<float>  ys      = {0.5F, 1.5F, 2.5F};
    const std::vector<double> weights = {1.0,  2.0,  0.5};

    ThreadLocalHistogram2D h_ref(3, 0.0, 3.0, 3, 0.0, 3.0);
    for (std::size_t i = 0; i < xs.size(); ++i) {
        h_ref.fill(xs[i], ys[i], weights[i]);
    }

    ThreadLocalHistogram2D h_span(3, 0.0, 3.0, 3, 0.0, 3.0);
    h_span.fill_span(std::span<const float>(xs),
                     std::span<const float>(ys),
                     std::span<const double>(weights));

    ASSERT_EQ(h_ref.entries(), h_span.entries());
    for (int ix = 0; ix <= 4; ++ix) {
        for (int iy = 0; iy <= 4; ++iy) {
            EXPECT_DOUBLE_EQ(h_ref.get_bin_content(ix, iy),
                             h_span.get_bin_content(ix, iy))
                << "bin " << ix << "," << iy;
        }
    }
}
