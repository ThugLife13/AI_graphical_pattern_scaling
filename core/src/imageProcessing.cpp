#include "../lib/imageProcessing.h"

bool imageProcessing::procesImages(wxString filePath) {

    //only after creating 1/4 or full size image

    // is working - cropping white borders
    if (!trimWhiteBorder(filePath)) {
        spdlog::error("procesImages: Failed to crop white border");
        return false;
    }

    // is working - gray scale
    if (!createGrayScale("../tmp/images/cropped.jpg")) {
        spdlog::error("procesImages: Failed to create gray scale image");
        return false;
    }

    // is working - color palette
    if (!createColorPalette("../tmp/images/cropped.jpg")) {
        spdlog::error("procesImages: Failed to create color palette");
        return false;
    }

    // is working - gray quantized
    if (!createGrayScale("../tmp/images/quantized.jpg")) {
        spdlog::error("procesImages: Failed to create 8-bit gray palette");
        return false;
    }

    if (!createQuarterImage("../tmp/images/grayQuantized.jpg")) {
        spdlog::error("procesImages: Failed to create quarter image");
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

    if (filePath == "../tmp/images/quantized.jpg") {
        if (!cv::imwrite("../tmp/images/grayQuantized.jpg", gray))
        {
            spdlog::error("createGrayScale: Failed to create quantized gray image");
            return false;
        }

        //hash it if want to do 1/4
        /**/
        if (!cv::imwrite("../tmp/images/toDecompose.jpg", gray)) {
            spdlog::error("quarterImage: Failed to create quarter image");
            return false;
        }
        /**/

        spdlog::info("createGrayScale: Created quantized gray image successfully");
        return true;
    }
    else {
        if (!cv::imwrite("../tmp/images/gray.jpg", gray))
        {
            spdlog::error("createGrayScale: Failed to create gray image");
            return false;
        }
        spdlog::info("createGrayScale: Created gray image successfully");
        return true;
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

    if (!cv::imwrite("../tmp/images/quantized.jpg", new_image))
    {
        spdlog::error("createColorPalette: Failed to create quantized image");
        return false;
    }
    spdlog::info("createColorPalette: Created quantized image successfully");
    return true;
}

bool imageProcessing::trimWhiteBorder(wxString filePath) {
    cv::Mat image = cv::imread(filePath.ToStdString());
    if (image.empty()) {
        spdlog::error("trimWhiteBorder: Failed to open image");
        return false;
    }

    const int whiteThreshold = 220;
    int top;
    int bottom;
    int left;
    int right;

    for (top = 0; top < image.rows; top++) {
        cv::Mat row = image.row(top).clone();
        cv::Mat rowFlat = row.reshape(1, row.total() * 3);
        if (cv::countNonZero(rowFlat < whiteThreshold) > 0) break;
    }

    for (bottom = image.rows - 1; bottom >= 0; bottom--) {
        cv::Mat row = image.row(bottom).clone();
        cv::Mat rowFlat = row.reshape(1, row.total() * 3);
        if (cv::countNonZero(rowFlat < whiteThreshold) > 0) break;
    }

    for (left = 0; left < image.cols; left++) {
        cv::Mat col = image.col(left).clone();
        cv::Mat colFlat = col.reshape(1, col.total() * 3);
        if (cv::countNonZero(colFlat < whiteThreshold) > 0) break;
    }

    for (right = image.cols - 1; right >= 0; right--) {
        cv::Mat col = image.col(right).clone();
        cv::Mat colFlat = col.reshape(1, col.total() * 3);
        if (cv::countNonZero(colFlat < whiteThreshold) > 0) break;
    }

    if (top >= bottom || left >= right) {
        spdlog::error("trimWhiteBorder: Image is only white");
        return false;
    }

    cv::Mat cropped = image(cv::Range(top, bottom + 1), cv::Range(left, right + 1));
    if (!cv::imwrite("../tmp/images/cropped.jpg", cropped)) {
        spdlog::error("trimWhiteBorder: Failed to create cropped image");
        return false;
    }

    spdlog::info("trimWhiteBorder: Cropped {} px (top/bottom/left/right)", top, image.rows - bottom - 1, left, image.cols - right - 1);

    spdlog::info("trimWhiteBorder: Created cropped image successfully");
    return true;
}

bool imageProcessing::createQuarterImage(wxString filePath) {
    cv::Mat image = cv::imread(filePath.ToStdString());
    if (image.empty()) {
        spdlog::error("quarterImage: Failed to open image");
        return false;
    }
    int ovelap = 50;
    int newWidth = (image.cols / 2) + ovelap;
    int newHeight = (image.rows / 2) + ovelap;

    if (newWidth <= 0 || newHeight <= 0) {
        spdlog::error("quarterImage: Image too small to quarter");
        return false;
    }

    cv::Rect roi(0, 0, newWidth, newHeight);
    cv::Mat quarter = image(roi);

    if (!cv::imwrite("../tmp/images/quarter.jpg", quarter)) {
        spdlog::error("quarterImage: Failed to create quarter image");
        return false;
    }

/* //unhash it if want to do 1/4
    if (!cv::imwrite("../tmp/images/toDecompose.jpg", quarter)) {
        spdlog::error("quarterImage: Failed to create quarter image");
        return false;
    }
*/
    return true;
}

bool imageProcessing::fillSelectionWithWhite(const wxRect& selection) {

    cv::Mat image = cv::imread("../tmp/images/toDecompose.jpg");
    if (image.empty()) {
        spdlog::error("fillSelectionWithWhite: Failed to load image {}", "../tmp/images/toDecompose.jpg");
        return false;
    }

    if (selection.GetX() < 0 || selection.GetY() < 0 ||
        selection.GetRight() > image.cols || selection.GetBottom() > image.rows) {
        spdlog::error("fillSelectionWithWhite: Selection out of bounds");
        return false;
        }

    cv::Rect cvRect(selection.GetX(), selection.GetY(), selection.GetWidth(), selection.GetHeight());
    cv::rectangle(image, cvRect, cv::Scalar(255, 255, 255), -1);

    if (!cv::imwrite("../tmp/images/toDecompose.jpg", image)) {
        spdlog::error("fillSelectionWithWhite: Failed to save image");
        return false;
    }

    spdlog::info("fillSelectionWithWhite: Filled selection in {}", "../tmp/images/toDecompose.jpg");
    return true;
}

