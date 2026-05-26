// test_main.cpp - host-side validation harness.
//
// Reads pzc_out_first10k.txt (one float per line), runs
// blind_deconvolve on window [7000, 8000), prints summary, and dumps
// the input window plus h_hat / x_hat / y_hat / g / x_tilde to TXT
// for element-wise comparison
// against the Python reference (example_result.npz).
//
// LOCAL test50 COPY of the user's `01_source/test_main.cpp`. Unmodified
// from upstream. Pinned here so we can build it against this folder's
// local blind_deconv.hpp (which has WINDOW_LEN=100, the test-local value).

#include "blind_deconv.hpp"
#include "dsp_kernels.hpp"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr int WIN_START = 7000;

struct OutputPaths {
    std::string y_input = "y_input.txt";
    std::string h_hat = "h_hat.txt";
    std::string x_hat = "x_hat.txt";
    std::string y_hat = "y_hat.txt";
    std::string g = "g.txt";
    std::string x_tilde = "x_tilde.txt";
};

bool load_text_vector(const std::string& path, std::vector<float>& out) {
    std::ifstream f(path);
    if (!f) {
        std::cerr << "failed to open " << path << "\n";
        return false;
    }
    out.clear();
    out.reserve(16384);
    double v;
    while (f >> v) out.push_back(static_cast<float>(v));
    return true;
}

void dump_txt(const std::string& path, const float* data, int n) {
    std::ofstream f(path);
    if (!f) {
        std::cerr << "failed to write " << path << "\n";
        return;
    }
    f.setf(std::ios::scientific);
    f.precision(8);
    for (int i = 0; i < n; ++i) f << data[i] << "\n";
}

void print_usage(const char* program) {
    std::fprintf(
        stderr,
        "usage:\n"
        "  %s\n"
        "  %s input.txt\n"
        "  %s input.txt y_input.txt h_hat.txt x_hat.txt y_hat.txt g.txt x_tilde.txt\n",
        program, program, program);
}

} // namespace

int main(int argc, char** argv) {
    std::string input_path = "02_input_signal/pzc_out_first10k.txt";
    OutputPaths out{};

    if (argc == 2 || argc == 8) {
        input_path = argv[1];
    } else if (argc != 1) {
        print_usage(argv[0]);
        return 2;
    }

    if (argc == 8) {
        out.y_input = argv[2];
        out.h_hat = argv[3];
        out.x_hat = argv[4];
        out.y_hat = argv[5];
        out.g = argv[6];
        out.x_tilde = argv[7];
    }

    std::vector<float> y_full;
    if (!load_text_vector(input_path, y_full)) return 1;
    std::printf("loaded %zu samples from %s\n", y_full.size(),
                input_path.c_str());

    if (WIN_START + blind::WINDOW_LEN > static_cast<int>(y_full.size())) {
        std::fprintf(stderr,
                     "window [%d, %d) exceeds data length %zu\n",
                     WIN_START, WIN_START + blind::WINDOW_LEN,
                     y_full.size());
        return 1;
    }
    const float* y_win = y_full.data() + WIN_START;
    dump_txt(out.y_input, y_win, blind::WINDOW_LEN);

    // Heap-allocate the scratch + result so we don't blow the host stack.
    auto scratch = std::vector<float>(blind::BLIND_SCRATCH_FLOATS, 0.0f);
    auto* res    = new blind::BlindResult{};

    std::printf("running blind_deconvolve on window [%d, %d)...\n",
                WIN_START, WIN_START + blind::WINDOW_LEN);

    blind::blind_deconvolve(y_win, *res, scratch.data());

    std::printf("\n=== summary ===\n");
    std::printf("converged       = %s\n", res->converged ? "true" : "false");
    std::printf("sigma_init      = %.6f\n", res->sigma_init);
    std::printf("baseline_mu     = %.6f\n", res->baseline_mu);
    std::printf("lambda_selected = %.6f\n", res->lambda_selected);
    std::printf("mse_y           = %.6f\n", res->mse_y);
    std::printf("sigma_self      = %.6f\n", res->sigma_self);
    std::printf("ratio           = %.6f\n", res->ratio);
    std::printf("active_samples  = %d / %d\n", res->active_samples,
                blind::X_LEN);
    std::printf("K_params        = %d\n", res->K_params);

    std::printf("\nh_hat = [");
    for (int k = 0; k < blind::M_PULSE; ++k) {
        std::printf("%s%.4f", (k ? ", " : ""), res->h_hat[k]);
    }
    std::printf("]\n");

    dump_txt(out.h_hat, res->h_hat.data(), blind::M_PULSE);
    dump_txt(out.x_hat, res->x_hat.data(), blind::X_LEN);
    dump_txt(out.y_hat, res->y_hat.data(), blind::WINDOW_LEN);
    std::printf("\nwrote y_input.txt, h_hat.txt, x_hat.txt, y_hat.txt\n");

    // Optional second stage: compute the calibrated inverse FIR.
    {
        blind::InverseFIR fir{};
        auto fir_scratch = std::vector<float>(
            blind::INVERSE_FIR_SCRATCH_FLOATS, 0.0f);
        const float gamma = blind::GAMMA_COEFF *
                            res->sigma_init * res->sigma_init;
        blind::inverse_tikhonov_calibrated(res->h_hat.data(), gamma,
                                            fir, fir_scratch.data());
        std::printf("\ninverse FIR: gamma=%.4f, delta_pos=%d, "
                    "noise_gain=%.4f\n",
                    fir.gamma, fir.delta_pos, fir.noise_gain);
        std::printf("g = [");
        for (int k = 0; k < blind::L_INVERSE; ++k) {
            std::printf("%s%.4f", (k ? ", " : ""), fir.g[k]);
        }
        std::printf("]\n");
        dump_txt(out.g, fir.g.data(), blind::L_INVERSE);

        // Apply g to y and dump x_tilde for comparison.
        std::vector<float> x_tilde(blind::WINDOW_LEN, 0.0f);
        blind::apply_inverse_fir(y_win, res->baseline_mu, fir,
                                  x_tilde.data());
        dump_txt(out.x_tilde, x_tilde.data(), blind::WINDOW_LEN);
        std::printf("wrote g.txt, x_tilde.txt\n");
    }

    // ---- LOCALIZER PROBE (test50-only) ----
    // Mirror the test50.cpp YANC probe so we can compare A_row_0, b_orig,
    // g_raw element-by-element between YANC and this host build. Whichever
    // array first diverges localizes where the inverse_fir bug lives.
    {
        const int M  = blind::M_PULSE;
        const int L  = blind::L_INVERSE;
        const int FL = blind::CONV_GH_LEN;
        const float gamma = blind::GAMMA_COEFF *
                            res->sigma_init * res->sigma_init;

        std::vector<float> r(M, 0.0f);
        for (int d = 0; d < M; ++d) {
            float s = 0.0f;
            const int kmax = M - d;
            for (int k = 0; k < kmax; ++k)
                s += res->h_hat[k] * res->h_hat[k + d];
            r[d] = s;
        }
        std::vector<float> A(L * L, 0.0f);
        for (int i = 0; i < L; ++i) {
            for (int j = 0; j < L; ++j) {
                const int d = (i >= j) ? (i - j) : (j - i);
                float v = (d < M) ? r[d] : 0.0f;
                if (i == j) v += gamma;
                A[i * L + j] = v;
            }
        }
        const int peak_h = blind::dsp::argmax_abs(res->h_hat.data(), M);
        int delta_pos = peak_h + (L >> 1);
        if (delta_pos < 0)    delta_pos = 0;
        if (delta_pos >= FL)  delta_pos = FL - 1;

        std::vector<float> b(L, 0.0f);
        for (int j = 0; j < L; ++j) {
            const int idx = delta_pos - j;
            b[j] = (idx >= 0 && idx < M) ? res->h_hat[idx] : 0.0f;
        }

        std::vector<float> A_row_0(A.begin(), A.begin() + L);
        std::vector<float> b_orig = b;
        const bool ok = blind::dsp::solve_spd(A.data(), b.data(), L);

        // Stage 2: re-do the calibration here, mirroring the YANC probe.
        std::vector<float> probe_gh(FL, 0.0f);
        blind::dsp::convolve_full(b.data(), L, res->h_hat.data(), M,
                                  probe_gh.data());
        const int   probe_peak = blind::dsp::argmax_abs(probe_gh.data(), FL);
        const float probe_pk   = probe_gh[probe_peak];
        const float probe_inv  = 1.0f / probe_pk;
        std::vector<float> probe_g_calibrated(L, 0.0f);
        for (int k = 0; k < L; ++k) probe_g_calibrated[k] = b[k] * probe_inv;

        // Single scalars first (delta_pos, ok), then arrays.
        {
            std::ofstream f("delta_pos.txt"); f.setf(std::ios::scientific);
            f.precision(8); f << static_cast<float>(delta_pos) << "\n";
        }
        {
            std::ofstream f("solve_ok.txt"); f.setf(std::ios::scientific);
            f.precision(8); f << (ok ? 1.0f : 0.0f) << "\n";
        }
        dump_txt("probe_A_row_0.txt", A_row_0.data(), L);
        dump_txt("probe_b_orig.txt", b_orig.data(), L);
        dump_txt("probe_g_raw.txt",  b.data(),       L);
        dump_txt("probe_gh.txt",     probe_gh.data(), FL);
        {
            std::ofstream f("probe_peak.txt"); f.setf(std::ios::scientific);
            f.precision(8); f << static_cast<float>(probe_peak) << "\n";
        }
        {
            std::ofstream f("probe_pk.txt"); f.setf(std::ios::scientific);
            f.precision(8); f << probe_pk << "\n";
        }
        dump_txt("probe_g_calibrated.txt", probe_g_calibrated.data(), L);
        std::printf("\nprobe: delta_pos=%d, solve_ok=%s\n",
                    delta_pos, ok ? "true" : "false");
        std::printf("probe_g_raw = [");
        for (int k = 0; k < L; ++k)
            std::printf("%s%.4f", (k ? ", " : ""), b[k]);
        std::printf("]\n");
        std::printf("probe: peak=%d, pk=%.6f, inv=%.6f\n",
                    probe_peak, probe_pk, probe_inv);
        std::printf("probe_g_calibrated = [");
        for (int k = 0; k < L; ++k)
            std::printf("%s%.4f", (k ? ", " : ""), probe_g_calibrated[k]);
        std::printf("]\n");
    }

    delete res;
    return 0;
}
