#include "../lib/core.h"

#include <cmath>

const double ROTATION_STEP = 30.0;
const double SCALE_STEP = 0.1;
const double MIN_SCALE = 0.5;
const double MAX_SCALE = 2.0;

core::core() {
}

core::~core() {
}

bool core::generate(wxString filePath) {

    spdlog::info("generate: Clearing images");

    if(!clearImages("..\\tmp\\images")) {
        spdlog::error("generate: Failed to erase old images in /tmp/images");
        return false;
    }

    //putting chosen file in tmp
    std::string tmpFolderPath = "..\\tmp\\images";
    std::string chosenPath = filePath.ToStdString();
    std::filesystem::copy(chosenPath,tmpFolderPath);

    //image processing
    imageProcessing imProc;

    if (!imProc.procesImages(filePath)) {
        spdlog::error("generate: Failed to proces all images");
        return false;
    }

    spdlog::info("generate: Path to image {}", filePath.ToStdString());

    return true;
}

bool core::clearImages(std::string folderPath) {
    try {
        if (!fs::exists(folderPath)) {
            spdlog::error("clearImages: Folder does not exist {} ", folderPath);
            return false;
        }

        for (const auto& entry : fs::directory_iterator(folderPath)) {
            if (fs::is_regular_file(entry.path())) {
                fs::remove(entry.path());
                spdlog::debug("clearImages: Deleted file {}", entry.path().string());
            }
            else if (fs::is_directory(entry.path())) {
                fs::remove_all(entry.path());
                spdlog::info("clearImages: Deleted directory {}", entry.path().string());
            }
        }

        spdlog::info("clearImages: Deleted everything in {} ", folderPath);
        return true;

    } catch (const fs::filesystem_error& e) {
        spdlog::error("clearImages: Filesystem error {}", e.what());
        return false;
    } catch (const std::exception& e) {
        spdlog::error("clearImages: General exception {}", e.what());
        return false;
    }
}

void applyNonMaximumSuppression(std::vector<MatchResult>& results, double overlapThreshold = 0.5) {
    std::sort(results.begin(), results.end(), [](const MatchResult& a, const MatchResult& b) {
        return a.accuracy > b.accuracy;
    });

    std::vector<MatchResult> filtered;
    for (const auto& current : results) {
        bool keep = true;
        cv::Rect currentRect(current.posX, current.posY, current.width, current.height);

        for (const auto& existing : filtered) {
            cv::Rect existingRect(existing.posX, existing.posY, existing.width, existing.height);

            // calculate overlap threshold
            cv::Rect intersection = currentRect & existingRect;
            double overlap = (intersection.area() * 2.0) /
                           (currentRect.area() + existingRect.area());

            if (overlap > overlapThreshold) {
                keep = false;
                break;
            }
        }

        if (keep) filtered.push_back(current);
    }

    results = filtered;
}

std::vector<MatchResult> core::findMatchingElements(wxString croppedImage, wxString mainImage) {
    std::vector<MatchResult> results;
    cv::Mat templ = cv::imread(croppedImage.ToStdString());
    cv::Mat scene = cv::imread(mainImage.ToStdString());

    if (templ.empty() || scene.empty()) {
        spdlog::error("findMatchingElements: Failed to load images");
        return results;
    }

    // different variants
    for (double angle = 0; angle < 360; angle += ROTATION_STEP) {

        // rotation
        cv::Point2f center(templ.cols/2.0f, templ.rows/2.0f);
        cv::Mat rotMat = cv::getRotationMatrix2D(center, angle, 1.0);
        cv::Mat rotatedTempl;
        cv::warpAffine(templ, rotatedTempl, rotMat, templ.size());

        // scaling
        for (double scale = MIN_SCALE; scale <= MAX_SCALE; scale += SCALE_STEP) {
            cv::Mat scaledTempl;
            cv::resize(rotatedTempl, scaledTempl, cv::Size(), scale, scale);

            if (scaledTempl.cols > scene.cols || scaledTempl.rows > scene.rows) continue;

            cv::Mat result;
            cv::matchTemplate(scene, scaledTempl, result, cv::TM_CCOEFF_NORMED);

            // Accuracy threshold
            for (double threshold = 1.0; threshold >= 0.5; threshold -= 0.05) {
                cv::Mat locations;
                cv::threshold(result, locations, threshold, 1.0, cv::THRESH_BINARY);
                locations.convertTo(locations, CV_8U);

                std::vector<cv::Point> matches;
                cv::findNonZero(locations, matches);

                for (const cv::Point& match : matches) {
                    MatchResult res;
                    res.posX = match.x;
                    res.posY = match.y;
                    res.width = scaledTempl.cols;
                    res.height = scaledTempl.rows;
                    res.accuracy = threshold * 100;

                    // Add only on first appearance
                    bool exists = std::any_of(results.begin(), results.end(),
                        [&res](const MatchResult& r) {
                            return abs(r.posX - res.posX) < res.width/2 &&
                                   abs(r.posY - res.posY) < res.height/2;
                        });

                    if (!exists) {
                        results.push_back(res);
                    }
                }
            }
        }
    }

    // Filter overlaps
    applyNonMaximumSuppression(results);

    // Sort results
    std::sort(results.begin(), results.end(), [](const MatchResult& a, const MatchResult& b) {
        return a.accuracy > b.accuracy;
    });

    spdlog::info("findMatchingElements: Found {} potential matches", results.size());
    return results;
}
