#include "../lib/core.h"

//rotation step for base angle: -10 - 10
const double BASE_ROTATION_STEP = 1.0;
const double BASE_ROTATION_RANGE = 5.0;

//scale: 0.85 - 1.15
const double MIN_SCALE = 0.8;
const double MAX_SCALE = 1.2;
const double SCALE_STEP= 0.1;

const double MAX_OVERLAP = 0.5;

const double GLOBAL_ACCURACY_THRESHOLD = 50.0;

namespace fs = std::filesystem;

using json = nlohmann::json;

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
                spdlog::info("clearImages: Deleted file {}", entry.path().string());
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

bool core::matchEveryElement() {

    wxString folderPath = "..\\tmp\\images\\toDecompose.jpg";

    std::vector<wxColour> colors = {
        wxColour(255, 0, 0),
        wxColour(0, 255, 0),
        wxColour(0, 0, 255),
        wxColour(255, 255, 0),
        wxColour(0, 255, 255),
        wxColour(255, 0, 255),
        wxColour(255, 165, 0),
        wxColour(128, 0, 128)
    };
    int firstColorIndex = 0;
    int secondColorIndex = 1;

    for (const auto& croppedImageEntry : fs::directory_iterator("..\\tmp\\cropped")) {
        //big decomposed images
        if (!croppedImageEntry.is_directory()) continue;
        std::string folderName = croppedImageEntry.path().filename().string();

        wxColour firstColor = colors[firstColorIndex % 8];
        firstColorIndex++;

        for (const auto& element : fs::directory_iterator(croppedImageEntry.path())) {
            //cropped images from decomposed images
            if (element.is_regular_file()) {

                wxColour secondColor = colors[(secondColorIndex + 1) % 8];
                secondColorIndex++;

                wxString croppedImage = element.path().string();

                spdlog::info("matchEveryElement: Cropped Image Entry Path {}", croppedImage.ToStdString());
                spdlog::info("matchEveryElement: First Color R_{} G_{} B_{} ", firstColor.Red(), firstColor.Green(), firstColor.Blue());
                spdlog::info("matchEveryElement: Second Color R_{} G_{} B_{} ", secondColor.Red(), secondColor.Green(), secondColor.Blue());

                findMatchingElements(croppedImage, folderPath, firstColor, secondColor);
            }
        }
    }

    return true;
}

bool core::saveToJson(std::vector<MatchResult> results) {
    if (firstSaving) {
        firstSaving = false;
        clearImages("..\\tmp\\jsons");
    }

    spdlog::info("saveToJson: saving json file");
    json j;
    for (const auto& result : results) {
        json r;
        r["x"] = result.posX;
        r["y"] = result.posY;
        r["w"] = result.width;
        r["h"] = result.height;
        r["a"] = result.accuracy;

        r["fr"] = result.fColorR;
        r["fg"] = result.fColorG;
        r["fb"] = result.fColorB;

        r["sr"] = result.sColorR;
        r["sg"] = result.sColorG;
        r["sb"] = result.sColorB;
        j.push_back(r);
    }
    std::string path = "../tmp/jsons/results_" + std::to_string(jsonCounter++) + ".json";
    std::ofstream file(path);
    file << j.dump(4);
    file.close();
    spdlog::info("saveToJson: json file saved {}", path);

    return true;
}

bool core::drawAllRectangles() {
    for (const auto& element : fs::directory_iterator("../tmp/jsons")) {
        if (element.is_regular_file()) {
            wxString path{element.path()};
            drawRectanglesFromJson(path);
        }
        firstDrawing = false;
    }
    return true;
}

double computeIOU(const cv::Rect& a, const cv::Rect& b) {
    cv::Rect intersection = a & b;
    if (intersection.area() == 0) return 0.0;
    double union_area = a.area() + b.area() - intersection.area();
    return intersection.area() / union_area;
}

cv::Mat rotateAndScale(const cv::Mat& src, double angle, double scale) {
    cv::Point2f center(src.cols / 2.0f, src.rows / 2.0f);
    cv::Mat rotMat = cv::getRotationMatrix2D(center, angle, scale);

    //bounding box calculation
    cv::Size2f src_size(static_cast<float>(src.cols * scale),
                static_cast<float>(src.rows * scale));
    cv::RotatedRect rotated_rect(cv::Point2f(), src_size, static_cast<float>(angle));
    cv::Rect bbox = rotated_rect.boundingRect();

    //transform calculation
    rotMat.at<double>(0, 2) += bbox.width / 2.0 - center.x;
    rotMat.at<double>(1, 2) += bbox.height / 2.0 - center.y;

    cv::Mat dst;
    cv::warpAffine(src, dst, rotMat, bbox.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0));
    return dst;
}

void applyNonMaximumSuppression(std::vector<MatchResult>& results) {
    std::sort(results.begin(), results.end(), [](const MatchResult& a, const MatchResult& b) {
        return a.accuracy > b.accuracy;
    });

    std::vector<MatchResult> filtered;
    filtered.reserve(results.size());

    for (const auto& current : results) {
        bool keep = true;
        cv::Rect currentRect(current.posX, current.posY, current.width, current.height);

        for (const auto& existing : filtered) {
            cv::Rect existingRect(existing.posX, existing.posY, existing.width, existing.height);
            double iou = computeIOU(currentRect, existingRect);

            if (iou > MAX_OVERLAP) {
                keep = false;
                break;
            }
        }

        if (keep) filtered.push_back(current);
    }

    results = filtered;
}

bool core::findMatchingElements(wxString croppedImage, wxString mainImage, wxColour fColor, wxColour sColor) {
    // Set number of threads explicitly
    int num_threads = std::thread::hardware_concurrency();
    omp_set_num_threads(num_threads);
    spdlog::info("OpenMP: using {} threads", num_threads);

    cv::Mat templ = cv::imread(croppedImage.ToStdString());
    cv::Mat scene = cv::imread(mainImage.ToStdString());

    if (templ.empty() || scene.empty()) {
        spdlog::error("Failed to load images");
        return false;
    }

    std::vector<MatchResult> globalResults;

    //scale
    std::vector<double> scales;
    for (double scale = MIN_SCALE; scale <= MAX_SCALE; scale += SCALE_STEP) {
        scales.push_back(scale);
    }

    //rotation
    std::vector<double> baseAngles = {0, 90, 180, 270};
    const double threshold = GLOBAL_ACCURACY_THRESHOLD / 100.0;

    //buffer base rotation (0*, 90*, 180*, 270*)
    std::map<double, cv::Mat> baseRotations;
    for (double base : baseAngles) {
        baseRotations[base] = rotateAndScale(templ, base, 1.0);
    }

    // Total tasks calculation
    const size_t totalTasks = scales.size() * baseAngles.size();
    std::atomic<size_t> completedTasks(0);

    // Single parallel region with proper nesting
    //TODO - fix parallel not working
    #pragma omp parallel for schedule(dynamic) collapse(2) shared(globalResults, baseRotations, completedTasks)
    for (int s_idx = 0; s_idx < static_cast<int>(scales.size()); s_idx++) {
        for (int b_idx = 0; b_idx < static_cast<int>(baseAngles.size()); b_idx++) {
            const double scale = scales[s_idx];
            const double base = baseAngles[b_idx];
            const int thread_id = omp_get_thread_num();

            // Log task start
            #pragma omp critical (log_start)
            {
                spdlog::info("Thread {} started: scale={:.2f}, angle={}",
                            thread_id, scale, base);
            }

            std::vector<MatchResult> localResults;
            cv::Mat baseRotated = baseRotations[base];

            // Process angle offsets
            for (double offset = -BASE_ROTATION_RANGE; offset <= BASE_ROTATION_RANGE; offset += BASE_ROTATION_STEP) {
                const double angle = base + offset;
                cv::Mat rotatedTempl = rotateAndScale(baseRotated, offset, scale);

                if (rotatedTempl.rows > scene.rows || rotatedTempl.cols > scene.cols) {
                    continue;
                }

                cv::Mat resultMap;
                cv::matchTemplate(scene, rotatedTempl, resultMap, cv::TM_CCOEFF_NORMED);


                // Find matches
                //TODO - fix posX is always less than 5
                for (int y = 0; y < resultMap.rows; y++) {
                    const float* row = resultMap.ptr<float>(y);
                    for (int x = 0; x < resultMap.cols; x++) {
                        if (row[x] >= threshold) {
                            MatchResult mr;
                            mr.posX = x;
                            mr.posY = y;
                            mr.width = rotatedTempl.cols;
                            mr.height = rotatedTempl.rows;
                            mr.accuracy = row[x] * 100.0;

                            mr.fColorR = fColor.Red();
                            mr.fColorG = fColor.Green();
                            mr.fColorB = fColor.Blue();
                            mr.sColorR = sColor.Red();
                            mr.sColorG = sColor.Green();
                            mr.sColorB = sColor.Blue();

                            localResults.push_back(mr);
                        }
                    }
                }
            }

            // Merge results
            #pragma omp critical (merge_results)
            {
                globalResults.insert(globalResults.end(), localResults.begin(), localResults.end());
            }

            // Update progress
            completedTasks++;
            #pragma omp critical (log_progress)
            {
                spdlog::info("Thread {} finished: scale={:.2f}, angle={} | Progress: {}/{} ({:.1f}%)",
                            thread_id, scale, base,
                            completedTasks.load(), totalTasks,
                            (completedTasks * 100.0) / totalTasks);
            }
        }
    }

    //TODO try without this
    applyNonMaximumSuppression(globalResults);

    std::sort(globalResults.begin(), globalResults.end(), [](const MatchResult& a, const MatchResult& b) {
        return a.accuracy > b.accuracy;
    });

    spdlog::info("Found {} matches", globalResults.size());
    saveToJson(globalResults);

    return !globalResults.empty();
}

bool core::drawRectanglesFromJson(wxString jsonPath) {
    std::string imagePath;

    if (firstDrawing) {
        imagePath = "../tmp/images/forAnalyzing.jpg";
    } else {
        imagePath = "../tmp/images/analyzed.jpg";
    }

    cv::Mat image = cv::imread(imagePath);

    if (image.empty()) {
        spdlog::error("drawRectanglesFromJson: Nie można wczytać obrazu z {}", imagePath);
        return false;
    }

    std::ifstream file(jsonPath.ToStdString());
    if (!file.is_open()) {
        spdlog::error("drawRectanglesFromJson: Cannot read JSON {}", jsonPath.ToStdString());
        return false;
    }

    json jsonData;
    try {
        file >> jsonData;
    } catch (const json::parse_error& e) {
        spdlog::error("drawRectanglesFromJson: JSON error: {}", e.what());
        return false;
    }

    for (const auto& element : jsonData) {
        try {
            int x = element["x"];
            int y = element["y"];
            int w = element["w"];
            int h = element["h"];

            if (x < 0 || y < 0 || w <= 0 || h <= 0 ||
                (x + w) > image.cols ||
                (y + h) > image.rows) {
                spdlog::warn("drawRectanglesFromJson: Skipped bad rectangle (x={}, y={}, w={}, h={})", x, y, w, h);
                continue;
            }

            cv::rectangle(image,
                cv::Point(x, y),
                cv::Point(x + w, y + h),
                cv::Scalar(element["fr"], element["fg"], element["fb"], 200),
                1);

            cv::circle(image,
                cv::Point(x + w/2, y + h/2),
                1,
                cv::Scalar(element["sr"], element["sg"], element["sb"], 200),
                -1);
        } catch (const json::exception& e) {
            spdlog::error("drawRectanglesFromJson: Cannot read JSON: {}", e.what());
        }
    }

    std::string outputPath = "../tmp/images/analyzed.jpg";
    if (!cv::imwrite(outputPath, image)) {
        spdlog::error("drawRectanglesFromJson: Cannot save {}", outputPath);
        return false;
    }

    spdlog::info("drawRectanglesFromJson: Saved {}", outputPath);
    return true;
}