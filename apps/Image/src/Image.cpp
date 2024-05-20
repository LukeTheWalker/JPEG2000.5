#include "Image.hpp"
#include <bitreverse.hpp>
#include <iostream>

using namespace std;
using namespace cv;
using namespace Typedefs;

Image::Image(const cv::Mat& _img, std::shared_ptr<Transform<cv::Mat>>& _fft) : img(_img), og_size(img.size()), fft(_fft) {
    // auto is_padding_needed = n_samples & (n_samples - 1);
    // auto correct_padding = (is_padding_needed && padding) ? next_power_of_2(n_samples) : n_samples;
    
    auto is_padding_needed_row = img.rows & (img.rows - 1);
    auto is_padding_needed_col = img.cols & (img.cols - 1);

    auto correct_padding_row = (is_padding_needed_row) ? next_power_of_2(img.rows) : img.rows;
    auto correct_padding_col = (is_padding_needed_col) ? next_power_of_2(img.cols) : img.cols;

    cv::copyMakeBorder(img, img, 0, correct_padding_row - img.rows, 0, correct_padding_col - img.cols, cv::BORDER_CONSTANT, cv::Scalar(0));

    input_space = fft->get_input_space(img);
    output_space = fft->get_output_space();
}

auto Image::inverse_transform_signal() -> void {
    fft->operator()(*input_space, *output_space, true);
    img = input_space->get_data();
}

auto Image::preprocess_filter (const double percentile) -> double {
    cout << "Number of pixels before filtering: " << og_size.height * og_size.width;
    auto normalized_percentile = (1.0 - (1.0 - percentile) * og_size.height * og_size.width / img.total());
    cout << " vs after filtering: " << (1 - normalized_percentile) * img.total() << endl;
    return normalized_percentile;
}

auto Image::filter_magnitude(const double percentile) -> void {
    auto normalized_percentile = preprocess_filter(percentile);
    output_space->compress("filter_magnitude", normalized_percentile);
    inverse_transform_signal();
}

auto Image::filter_freqs(const double percentile) -> void {

    auto normalized_percentile = preprocess_filter(percentile);
    output_space->compress("filter_freqs", normalized_percentile);
    inverse_transform_signal();
}

auto Image::transform_signal() -> void {
    fft->operator()(*input_space, *output_space, false);
}

auto Image::get_image() const -> const cv::Mat {
    return img.rowRange(0, og_size.height).colRange(0, og_size.width);
}

auto Image::get_fft_freqs() const -> const cv::Mat {
    return output_space->get_plottable_representation().rowRange(0, og_size.height).colRange(0, og_size.width);
}
