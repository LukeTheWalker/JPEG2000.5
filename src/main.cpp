#include <iostream>
#include <vector>
#include <cmath>
#include <complex>

#include <typedefs.hpp>
#include <matplotlibcpp.h>

#include <Signal.hpp>
#include <dft.hpp>
#include <fft.hpp>
#include <fft_it.hpp>
#include <ifft.hpp>

namespace plt = matplotlibcpp;
using namespace std;

auto plot_stuff (const Signal& s, const Signal& s_filtered, const int& N) {
    const int width = 1400;
    const int height = 700;

    auto x = s.get_x();
    auto y = s.get_real_signal();
    auto y2 = s_filtered.get_real_signal();
    
    // truncate to N
    y.resize(N);
    y2.resize(N);
    x.resize(N);

    // print x and y sizes 
    int rows = 2; int cols = 2;
    plt::figure_size(width, height);
    plt::subplot(rows, cols, 1);
    plt::title("Original signal");
    plt::plot(x, y);
    plt::subplot(rows, cols, 2);
    plt::title("Inverse FFT");
    plt::plot(x, y2);
    plt::subplot(rows, cols, 3);
    plt::title("FFT");
    plt::plot(s.get_fft_freqs());
    plt::subplot(rows, cols, 4);
    plt::title("filtered FFT");
    plt::plot(s_filtered.get_fft_freqs());
    plt::show();
    plt::save("output/fft.png");
}

auto main() -> int
{
    // generate signal
    const int N = 10000;
    vector<double> freqs = {1,100};
    vector<double> amps = {1,.1};

    Signal s(freqs, amps, N);

    s.transform_signal(iterative::fft);

    Signal s_filtered = s;

    const int freq_flat = 50;
    s_filtered.filter_freqs(freq_flat, iterative::fft);

    plot_stuff(s, s_filtered, N);
}