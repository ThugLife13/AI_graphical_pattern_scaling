#include "../lib/imageProcessing.h"

bool imageProcessing::procesImages(wxString filePath) {
    // is working
    if (!createGrayScale(filePath)) {
        spdlog::error("procesImages: Failed to create gray scale image");
        return false;
    }

    // is working
    if (!createColorPalette(filePath)) {
        spdlog::error("procesImages: Failed to create color palette");
        return false;
    }

    // is working
    if (!createColorPalette("../tmp/images/gray.jpg")) {
        spdlog::error("procesImages: Failed to create 8-bit gray palette");
        return false;
    }

    spdlog::info("procesImages: All components passed succesfully");
    return true;
}

bool imageProcessing::createGrayScale(wxString filePath) {
    cv::Mat image = cv::imread(filePath.ToStdString());

    if (image.empty()) {
        spdlog::error("createGrayScale: Failed to open image");
        return false;
    }

    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

    if (!cv::imwrite("../tmp/images/gray.jpg", gray))
    {
        spdlog::error("createGrayScale: Failed to create image in gray scale");
        return false;
    }

    spdlog::info("createGrayScale: Created gray image successfully");
    return true;
}

bool imageProcessing::createColorPalette(wxString filePath) {
    cv::Mat image = cv::imread(filePath.ToStdString());

    if (image.empty()) {
        spdlog::error("createColorPalette: Failed to open image");
        return false;
    }

    cv::Mat data;
    image.convertTo(data, CV_32F);          // 8U -> 32F
    data = data.reshape(1, data.total());   // from [rows x cols x 3] to [rows*cols, 3]

    int K = 8;          //colors in palette
    cv::Mat labels;
    cv::Mat centers;

    cv::kmeans(
        data,
        K,
        labels,
        cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 15, 1.0),
        3,
        cv::KMEANS_PP_CENTERS,
        centers
    );

    centers = centers.reshape(3, centers.rows);
    cv::Mat new_image(data.size(), data.type());

    for (int i = 0; i < data.rows; ++i)
    {
        int cluster_idx = labels.at<int>(i, 0);
        new_image.at<cv::Vec3f>(i, 0) = centers.at<cv::Vec3f>(cluster_idx, 0);
    }

    new_image = new_image.reshape(3, image.rows);
    new_image.convertTo(new_image, CV_8U);

    if (filePath == "../tmp/images/gray.jpg") {
        if (!cv::imwrite("../tmp/images/grayQuantized.jpg", new_image))
        {
            spdlog::error("createColorPalette: Failed to create quantized image");
            return false;
        }
        spdlog::info("createColorPalette: Created 8-bit gray image successfully");
        return true;
    }

    else {
        if (!cv::imwrite("../tmp/images/quantized.jpg", new_image))
        {
            spdlog::error("createColorPalette: Failed to create quantized image");
            return false;
        }
        spdlog::info("createColorPalette: Created quantized image successfully");
        return true;
    }
}
