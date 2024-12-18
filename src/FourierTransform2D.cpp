#include "FourierTransform2D.hpp"
#include "bitreverse.hpp"
#include "utils.hpp"

using namespace std;
using namespace cv;
using namespace Typedefs;

using namespace tr;

template <class FT> requires std::is_base_of_v<FourierTransform, FT>
FourierTransform2D<FT>::InputSpace::InputSpace(const cv::Mat& og_image) {
    cv::Mat image;
    og_image.convertTo(image, CV_64FC3);
    auto channels = image.channels();
    auto rows = image.rows;
    auto cols = image.cols;
    data = Typedefs::vcpx3D(channels, Typedefs::vcpx2D(rows, Typedefs::vcpx(cols)));
    for (int c = 0ull; c < channels; ++c)
        for (int i = 0ull; i < rows; ++i)
            for (int j = 0ull; j < cols; ++j)
                data[c][i][j] = Typedefs::cpx(image.at<Vec3d>(i, j)[c], 0.0);
}

template <class FT> requires std::is_base_of_v<FourierTransform, FT>
auto FourierTransform2D<FT>::InputSpace::get_data() const -> cv::Mat {
    auto channels = data.size();
    auto rows = data[0].size();
    auto cols = data[0][0].size();
    cv::Mat img_char(rows, cols, CV_64FC3);
    for (auto c = 0ull; c < channels; ++c)
        for (auto i = 0ull; i < rows; ++i)
            for (auto j = 0ull; j < cols; ++j)
                img_char.at<Vec3d>(i, j)[c] = data[c][i][j].real() / (rows * cols);
    img_char.convertTo(img_char, CV_8UC3);
    return img_char;
}

template <class FT> requires std::is_base_of_v<FourierTransform, FT>
void FourierTransform2D<FT>::OutputSpace::rearrenge_quadrants_to_center(Typedefs::vcpx3D& data_to_arrange) const {
{
    auto channels = data_to_arrange.size();
    auto rows = data_to_arrange[0].size();
    auto cols = data_to_arrange[0][0].size();

    Typedefs::vcpx3D temp_data_to_arrange(channels, Typedefs::vcpx2D(rows, Typedefs::vcpx(cols)));

    // Quadrant 1 (top-left) <-> Quadrant 3 (bottom-right)
    for (auto c = 0ull; c < channels; ++c)
    {
        for (auto i = 0ull; i < rows / 2; ++i)
        {
            for (auto j = 0ull; j < cols / 2; ++j)
            {
                temp_data_to_arrange[c][i][j] = data_to_arrange[c][i + rows / 2][j + cols / 2];
                temp_data_to_arrange[c][i + rows / 2][j + cols / 2] = data_to_arrange[c][i][j];
            }
        }
    }

    // Quadrant 2 (top-right) <-> Quadrant 4 (bottom-left)
    for (auto c = 0ull; c < channels; ++c)
    {
        for (auto i = 0ull; i < rows / 2; ++i)
        {
            for (auto j = 0ull; j < cols / 2; ++j)
            {
                temp_data_to_arrange[c][i][j + cols / 2] = data_to_arrange[c][i + rows / 2][j];
                temp_data_to_arrange[c][i + rows / 2][j] = data_to_arrange[c][i][j + cols / 2];
            }
        }
    }

    data_to_arrange = temp_data_to_arrange;
    }
}

template <class FT> requires std::is_base_of_v<FourierTransform, FT>
auto FourierTransform2D<FT>::OutputSpace::get_plottable_representation() const -> cv::Mat
{
    auto tmp = data;
    rearrenge_quadrants_to_center(tmp);
    cv::Mat fft_image_colored;
    fft_image_colored.create(tmp[0].size(), tmp[0][0].size(), CV_64FC3);
    for (auto c = 0ull; c < tmp.size(); ++c)
        for (auto i = 0ull; i < tmp[0].size(); ++i)
            for (auto j = 0ull; j < tmp[0][0].size(); ++j)
                fft_image_colored.at<cv::Vec3d>(i, j)[c] = log(1 + abs(tmp[c][i][j]));
    cv::normalize(fft_image_colored, fft_image_colored, 0, 255, NORM_MINMAX);
    fft_image_colored.convertTo(fft_image_colored, CV_8UC3);
    return fft_image_colored;
}

template <class FT> requires std::is_base_of_v<FourierTransform, FT>
auto FourierTransform2D<FT>::OutputSpace::compress(const std::string& method, const double kept) -> void {
    rearrenge_quadrants_to_center(data);
    if (method == "filter_freqs") {
        pass_filter(kept, true);
    } else if (method == "filter_magnitude") {
        magnitude_filter(kept);
    } else {
        throw std::invalid_argument("Invalid compression method");
    }
    rearrenge_quadrants_to_center(data);
}

template <class FT> requires std::is_base_of_v<FourierTransform, FT>
void FourierTransform2D<FT>::OutputSpace::pass_filter(const double cutoff_perc, const bool is_high_pass) 
{
    auto  channels = data.size();
    auto rows = data[0].size();
    auto cols = data[0][0].size();

    unsigned long int area_square = rows * cols;
    //  r = √((g/100) × x^2 / π)
    auto cutoff_freq = sqrt((1 - cutoff_perc) * (double)area_square / M_PI);

    pair<int, int> center = {rows / 2, cols / 2};
    // lamda for distance calculation
    auto distance = [&](int i, int j) { return sqrt(pow(i - center.first, 2) + pow(j - center.second, 2)); };

    // Apply the low-pass filter
    for (auto c = 0ull; c < channels; ++c)
        for (auto i = 0ull; i < rows; ++i)
            for (auto j = 0ull; j < cols; ++j)
                if ((is_high_pass && distance(i,j) > cutoff_freq) || (!is_high_pass && distance(i,j) <= cutoff_freq))
                    data[c][i][j] = 0.0;
}

template <class FT> requires std::is_base_of_v<FourierTransform, FT>
void FourierTransform2D<FT>::OutputSpace::magnitude_filter(double cutoff_percentage) {
    auto channels = data.size();
    auto rows = data[0].size();
    auto cols = data[0][0].size();

    // Calculate the magnitude of the FFT
    vector<double> magnitude(rows * cols * channels);
    for (auto c = 0ull; c < channels; ++c)
        for (auto i = 0ull; i < rows; ++i)
            for (auto j = 0ull; j < cols; ++j)
                magnitude[i * cols + j + c * rows * cols] = abs(data[c][i][j]);

    // Sort the magnitude vector
    sort(magnitude.begin(), magnitude.end());

    // Calculate the cutoff value
    double cutoff_value = magnitude[(int)(magnitude.size() * cutoff_percentage)];

    // Apply the filter
    for (auto c = 0ull; c < channels; ++c)
        for (auto i = 0ull; i < rows; ++i)
            for (auto j = 0ull; j < cols; ++j)
                if (abs(data[c][i][j]) < cutoff_value)
                    data[c][i][j] = 0.0;
}

template <class FT> requires std::is_base_of_v<FourierTransform, FT>
auto FourierTransform2D<FT>::compute2DFFT(Typedefs::vcpx3D& fft_coeff, const bool is_inverse) const -> void {
    auto channels = fft_coeff.size();
    auto rows = fft_coeff[0].size();
    auto cols = fft_coeff[0][0].size();

    for (auto c = 0ull; c < channels; ++c){
        
        // Compute the 1D FFT along each row
        for (auto i = 0ull; i < rows; ++i){
            auto tmp_vec = fft_coeff[c][i];
            ft(tmp_vec, is_inverse); // Compute the FFT
            fft_coeff[c][i]= tmp_vec;
        }

        // Compute the 1D FFT along each column
        for (auto j = 0ull; j < cols; ++j)
        {
            Typedefs::vcpx col(rows);
            for (auto i = 0ull; i < rows; ++i) col[i] = fft_coeff[c][i][j];
            ft(col, is_inverse); // Compute the FFT
            for (auto i = 0ull; i < rows; ++i) fft_coeff[c][i][j] = col[i];
        }
    }
}

template <class FT> requires std::is_base_of_v<FourierTransform, FT>
auto FourierTransform2D<FT>::get_input_space(const cv::Mat& og_image) const -> std::unique_ptr<Transform::InputSpace> {
    Mat image;

    auto is_padding_needed_row = og_image.rows & (og_image.rows - 1);
    auto is_padding_needed_col = og_image.cols & (og_image.cols - 1);

    auto correct_padding_row = (is_padding_needed_row) ? utils::next_power_of_2(og_image.rows) : og_image.rows;
    auto correct_padding_col = (is_padding_needed_col) ? utils::next_power_of_2(og_image.cols) : og_image.cols;

    cv::copyMakeBorder(og_image, image, 0, correct_padding_row - og_image.rows, 0, correct_padding_col - og_image.cols, cv::BORDER_CONSTANT, cv::Scalar(0));

    std::unique_ptr<Transform::InputSpace> in = std::make_unique<FourierTransform2D::InputSpace>(image);

    return in;
}

template <class FT> requires std::is_base_of_v<FourierTransform, FT>
auto FourierTransform2D<FT>::get_output_space() const -> std::unique_ptr<Transform::OutputSpace> {
    std::unique_ptr<Transform::OutputSpace> out = std::make_unique<FourierTransform2D::OutputSpace>();
    return out;
}

template <class FT> requires std::is_base_of_v<FourierTransform, FT>
auto FourierTransform2D<FT>::operator()(Transform::InputSpace& in, Transform::OutputSpace& out, const bool inverse) const -> void {
    auto& in_data = dynamic_cast<FourierTransform2D::InputSpace&>(in).data;
    auto& out_data = dynamic_cast<FourierTransform2D::OutputSpace&>(out).data;
    if (!inverse) {
        out_data = in_data;
        compute2DFFT(out_data, inverse);
    } else {
        in_data = out_data;
        compute2DFFT(in_data, inverse);
    }
}

// Explicit instantiation of the template classes
template class tr::FourierTransform2D<tr::IterativeFastFourierTransform>;
template class tr::FourierTransform2D<tr::RecursiveFastFourierTransform>;
template class tr::FourierTransform2D<tr::DiscreteFourierTransform>;