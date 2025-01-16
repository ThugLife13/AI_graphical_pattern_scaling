//
// Created by mateu on 14.01.2025.
//

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

    // i wanna die
    if (!elementsDetection()) {
        spdlog::error("procesImages: Failed to detect elements");
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

    spdlog::info("createGrayScale: Created gray image succesfully");
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

bool imageProcessing::elementsDetection() {

    //ORB config
    int nFeatures = 2000;           // key points
    float scaleFactor = 1.2f;       // density of the pyramid
    int nLevels = 4;                // number of pyramid levels
    int edgeThreshold = 19;         // edge threshold

    //matches config
    float distanceThreshold = 60.0f;

    //ransac toleration
    double ransacReprojThreshold = 2.0;


    // paths
    std::string grayImagePath = "../tmp/images/gray.jpg";
    std::string colorImagePath = "../tmp/images/quantized.jpg";

    // 1. Reading image in gray scale for features detection
    cv::Mat imgGray = cv::imread(grayImagePath, cv::IMREAD_GRAYSCALE);
    if (imgGray.empty())
    {
        spdlog::error("elementsDetection: Failed to read grau image");
        return false;
    }

    // 1.5 Reading image in color for bounding box drawing
    cv::Mat imgColor = cv::imread(colorImagePath, cv::IMREAD_COLOR);
    if (imgColor.empty())
    {
        spdlog::error("elementsDetection: Failed to read quantized image");
        return false;
    }

    //gaussian blur
    cv::Mat imgGrayBlur;
    cv::GaussianBlur(imgGray, imgGrayBlur, cv::Size(3, 3), 0);

    cv::Mat resized;
    cv::resize(imgGray, resized, cv::Size(), 0.5, 0.5, cv::INTER_AREA);

    // Creating ORB
    cv::Ptr<cv::FeatureDetector> detector = cv::FeatureDetector::create("ORB");
    cv::Ptr<cv::DescriptorExtractor> extractor = cv::DescriptorExtractor::create("ORB");
    spdlog::info("elementsDetection: Created detector and extractor successfully");

    detector->set("nFeatures", 1000);

    // detection (keypoints + descriptors)
    std::vector<cv::KeyPoint> keypoints;
    cv::Mat descriptors;


    detector->detect(imgGrayBlur, keypoints);
    extractor->compute(imgGrayBlur, keypoints, descriptors);

    detector->set("nFeatures", nFeatures);
    detector->set("scaleFactor", scaleFactor);
    detector->set("nLevels", nLevels);
    detector->set("edgeThreshold", edgeThreshold);
    //detector->set("fastThreshold", fastThreshold);

    // BFMatcher with Hamming norm
    cv::BFMatcher matcher(cv::NORM_HAMMING);

    std::vector<cv::Scalar> boxColors = {
        cv::Scalar(0,   0,   0),   // czarny
        cv::Scalar(0,   0, 255),   // czerwony
        cv::Scalar(0, 255,   0),   // zielony
        cv::Scalar(255, 0,   0),   // niebieski
        cv::Scalar(0, 255, 255)    // żółty
    };

    std::vector<cv::KeyPoint> currKeypoints = keypoints;
    cv::Mat currDescriptors = descriptors.clone();
    spdlog::info("elementsDetection: Created keypoints vector successfully");

    // finding 5 most distinctive repetitions
    for (int i = 0; i < 5; i++)
    {
        spdlog::info("elementsDetection: Main loop iteration {}", i);

        // break if too few matching
        // homografy needs min. 4 matching
        if (currKeypoints.size() < 4) {
            spdlog::error("elementsDetection: Failed to find 4 keypoints");
            break;
        }

        // 2. self matching – in loop
        std::vector<cv::DMatch> matches;
        matcher.match(currDescriptors, currDescriptors, matches);

        // 3. filtration - we reject ones with to big distance or self matches
        std::vector<cv::DMatch> goodMatches;
        for (auto &m : matches)
        {

            if (m.distance < distanceThreshold && m.queryIdx != m.trainIdx)
            {
                goodMatches.push_back(m);
            }
        }

        // If there are no good matches we skip loop
        if (goodMatches.empty()) {
            spdlog::info("elementsDetection: Failed to find matches iteration {} ", i);
            continue;
        }

        // 4. Preparing points for findHomography
        std::vector<cv::Point2f> srcPoints;
        std::vector<cv::Point2f> dstPoints;

        for (auto &gm : goodMatches)
        {
            srcPoints.push_back(currKeypoints[gm.queryIdx].pt);
            dstPoints.push_back(currKeypoints[gm.trainIdx].pt);
        }

        // 5. find Homography + inliers mask
        cv::Mat mask;
        cv::Mat H = cv::findHomography(srcPoints, dstPoints, cv::RANSAC, ransacReprojThreshold, mask);

        if (H.empty())
        {
            spdlog::info("elementsDetection: Failed to find transformation in iteration {} ", i);
            continue;
        }

        // Counting inliers
        int inlierCount = 0;
        for (int j = 0; j < mask.rows; j++)
        {
            if (mask.at<uchar>(j))
                inlierCount++;
        }

        // If we have to little inlierers, we propably don't have repetition
        if (inlierCount < 4)
        {
            spdlog::info("elementsDetection: Too few inliners in iteration {} ", i);
            continue;
        }

        spdlog::info("elementsDetection: Homografy found, number of inliners {} ", inlierCount);

        // 6. Creating bounding box based inliers points
        //    inliers min_x, max_x, min_y, max_y
        float minX = std::numeric_limits<float>::max();
        float maxX = -std::numeric_limits<float>::max();
        float minY = std::numeric_limits<float>::max();
        float maxY = -std::numeric_limits<float>::max();

        // Colecting inliers id, to remove them from pot
        std::vector<int> inlierIndices;

        for (int j = 0; j < mask.rows; j++)
        {
            if (mask.at<uchar>(j))
            {
                // srcPoints[j] (or dstPoints, both represents same geometric structure)
                cv::Point2f p = srcPoints[j];
                if (p.x < minX) minX = p.x;
                if (p.x > maxX) maxX = p.x;
                if (p.y < minY) minY = p.y;
                if (p.y > maxY) maxY = p.y;

                inlierIndices.push_back(goodMatches[j].queryIdx);
                inlierIndices.push_back(goodMatches[j].trainIdx);
            }
        }

        // Creating bounding rects
        if (maxX > minX && maxY > minY)
        {
            cv::Rect boundingRect(
                cv::Point2f(minX, minY),
                cv::Point2f(maxX, maxY)
            );

            // 7. Drawing bounding box on colored image
            cv::rectangle(
                imgColor,
                boundingRect,
                boxColors[i % boxColors.size()], // choosing color
                2 // border thickness
            );
            spdlog::info("elementsDetection: Bounding box drawed");
        }

        // 8. Removing liners from currKeypoints and currDescriptors, so we will skip them in next loop.

        std::set<int> toRemoveIndices(inlierIndices.begin(), inlierIndices.end());

        // New vectors for keypoints and descriptors
        std::vector<cv::KeyPoint> newKeypoints;
        cv::Mat newDescriptors;
        newDescriptors.reserve(currDescriptors.rows); // memory reservation

        // Creating map old_id -> new_id, so BFMatcher will make sense in next loops
        int newIdx = 0;
        std::vector<int> oldIdxToNewIdx(currKeypoints.size(), -1);

        for (int idx = 0; idx < (int)currKeypoints.size(); idx++)
        {
            if (toRemoveIndices.find(idx) == toRemoveIndices.end())
            {
                // if nothing to delete
                newKeypoints.push_back(currKeypoints[idx]);
                newDescriptors.push_back(currDescriptors.row(idx));
                oldIdxToNewIdx[idx] = newIdx;
                newIdx++;
            }
        }

        // Override
        currKeypoints = newKeypoints;
        currDescriptors = newDescriptors.clone();
    }

    // 9. Saving image

    if (!cv::imwrite("../tmp/images/result.jpg", imgColor))
    {
        spdlog::error("elementsDetection: Saving image error");
        return false;
    }

    spdlog::info("elementsDetection: Image saved");
    return true;

}
