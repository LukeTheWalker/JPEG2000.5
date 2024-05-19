#ifndef FFT2D_HPP
#define FFT2D_HPP

#include <opencv2/opencv.hpp>

#include <IterativeFastFourierTransform.hpp>
#include <RecursiveFastFourierTransform.hpp>
#include <DiscreteFourierTransform.hpp>

template <class FT>
class FourierTransform2D : public Transform<cv::Mat> {
    private:
    FT ft;
    public:
    class InputSpace : public Transform::InputSpace {
        public:
        cv::Mat data;
        InputSpace(const cv::Mat& og_image);
        auto get_data() const -> cv::Mat override;
    };
    class OutputSpace : public Transform::OutputSpace {
        public:
        Typedefs::vcpx3D data;
        void rearrenge_quadrants_to_center(Typedefs::vcpx3D& data_to_arrange) const;
        auto get_plottable_representation() const -> cv::Mat override;
        auto compress(const std::string& method, const double kept) -> void override;
        void pass_filter(double cutoff_perc, bool is_high_pass);
        void magnitude_filter(double cutoff_percentage);
    };
    
    auto compute2DFFT(cv::Mat &image, Typedefs::vcpx3D& fft_coeff, bool is_inverse) const -> void;
    auto get_input_space(const cv::Mat& og_image) const -> std::unique_ptr<Transform::InputSpace> override;
    auto get_output_space() const -> std::unique_ptr<Transform::OutputSpace> override ;
    auto operator()(Transform::InputSpace& in, Transform::OutputSpace& out, bool inverse) const -> void override;
};

#endif