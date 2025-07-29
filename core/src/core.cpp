#include "../lib/core.h"

//rotation step for base angle: -10 - 10
const double BASE_ROTATION_STEP = 1.0;
const double BASE_ROTATION_RANGE = 5.0;

//scale: 0.85 - 1.15
const double MIN_SCALE = 0.8;
const double MAX_SCALE = 1.2;
const double SCALE_STEP= 0.1;

const double MAX_OVERLAP = 0.4;

const double GLOBAL_ACCURACY_THRESHOLD = 50.0;

namespace fs = std::filesystem;

std::mutex cout_mutex;

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

    wxString folderPath = "..\\tmp\\images\\forAnalyzing.jpg";

    int firstColorIndex = 0;
    int secondColorIndex = 1;

    for (const auto& croppedImageEntry : fs::directory_iterator("..\\tmp\\cropped")) {
        //big decomposed images
        if (!croppedImageEntry.is_directory()) continue;
        std::string folderName = croppedImageEntry.path().filename().string();

        wxColour firstColor = colors[firstColorIndex % 8];
        int fColorIndex = firstColorIndex % 8;
        firstColorIndex++;

        for (const auto& element : fs::directory_iterator(croppedImageEntry.path())) {
            //cropped images from decomposed images
            if (element.is_regular_file()) {

                wxColour secondColor = colors[(secondColorIndex + 1) % 8];
                int sColorIndex = (secondColorIndex + 1) % 8;
                secondColorIndex++;

                wxString croppedImage = element.path().string();

                spdlog::info("matchEveryElement: Cropped Image Entry Path {}", croppedImage.ToStdString());
                spdlog::info("matchEveryElement: First Color R_{} G_{} B_{} ", firstColor.Red(), firstColor.Green(), firstColor.Blue());
                spdlog::info("matchEveryElement: Second Color R_{} G_{} B_{} ", secondColor.Red(), secondColor.Green(), secondColor.Blue());

                findMatchingElements(croppedImage, folderPath, fColorIndex, sColorIndex);
                croppedImageID++;
            }
        }
        decomposedImageID++;
    }

    if (!NMSForAllResultsCombined()) {
        return false;
    }

    return true;
}

bool core::saveToJson(std::vector<MatchResult> results, bool withCounter) {
    if (firstSaving) {
        firstSaving = false;
        clearImages("..\\tmp\\jsons");
    }
    std::string path;
    spdlog::info("saveToJson: saving json file");
    json j;
    for (const auto& result : results) {
        json r;

        r["Meta.croppedID"] = result.croppedID;
        r["Meta.decomposedID"] = result.decomposedID;

        r["Pos.X"] = result.posX;
        r["Pos.Y"] = result.posY;

        r["Pos.Width"] = result.width;
        r["Pos.Height"] = result.height;

        r["Pos.Accuracy"] = result.accuracy;
        r["Pos.Scale"] = result.scale;
        r["Pos.Rotation"] = result.rotation;

        r["Color.First"] = result.fColor;
        r["Color.Second"] = result.sColor;

        j.push_back(r);
    }

    if (withCounter) {
        path = "../tmp/jsons/results_" + std::to_string(jsonCounter++) + ".json";
    } else {
        path = "../tmp/jsons/resultsCombined/combined.json";
    }

    std::ofstream file(path);
    file << j.dump(4);
    file.close();
    spdlog::info("saveToJson: json file saved {}", path);

    return true;
}

bool core::drawAllRectangles(double accuracy) {
    for (const auto& element : fs::directory_iterator("../tmp/jsons")) {
        if (element.is_regular_file()) {
            wxString path{element.path()};
            drawRectanglesFromJson(path, accuracy, false);
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
    cv::warpAffine(src, dst, rotMat, bbox.size(), cv::INTER_CUBIC | cv::WARP_INVERSE_MAP, cv::BORDER_CONSTANT, cv::Scalar(0));
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

std::vector<MatchResult> findMatchingElementsParallel(
    //TODO - new way of finding
    const cv::Mat& baseRotated, //we are looking for this one
    const cv::Mat& scene,       //we are looking here
    double baseAngle,
    double scale,
    double threshold,
    const int fColor,
    const int sColor,
    const int croppedID,
    const int decomposedID)
{
    std::vector<MatchResult> localResults;
    std::ostringstream tid;
    tid << std::this_thread::get_id();
    std::string threadId = tid.str();

    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        spdlog::info("Thread {} STARTED: scale={:.2f}, base angle={}", threadId, scale, baseAngle);
    }

    for (double offset = -BASE_ROTATION_RANGE; offset <= BASE_ROTATION_RANGE; offset += BASE_ROTATION_STEP) {
        double angle = baseAngle + offset;
        cv::Mat rotatedTempl = rotateAndScale(baseRotated, offset, scale);

        if (rotatedTempl.rows > scene.rows || rotatedTempl.cols > scene.cols) continue;

        cv::Mat resultMap;
        cv::matchTemplate(scene, rotatedTempl, resultMap, cv::TM_CCOEFF_NORMED);

        for (int y = 0; y < resultMap.rows; y++) {
            const float* row = resultMap.ptr<float>(y);
            for (int x = 0; x < resultMap.cols; x++) {
                float score = row[x];
                if (score >= static_cast<float>(threshold)) {
                    MatchResult mr;
                    mr.croppedID = croppedID;
                    mr.decomposedID = decomposedID;
                    mr.posX = x;
                    mr.posY = y;
                    mr.width = baseRotated.cols;
                    mr.height = baseRotated.rows;
                    mr.accuracy = static_cast<double>(score);
                    mr.rotation = angle;
                    mr.scale = scale;

                    mr.fColor = fColor;
                    mr.sColor = sColor;

                    // {
                    //     std::lock_guard<std::mutex> lock(cout_mutex);
                    //     spdlog::info("Thread {} found match at: X={}, Y={}, scale={:.2f}, base angle={}, with accuracy={:.2f}",
                    //         threadId, x, y, scale, baseAngle, mr.accuracy);
                    // }

                    localResults.push_back(mr);
                }
            }
        }
    }

    return localResults;
}

bool core::findMatchingElements(wxString croppedImage, wxString mainImage, int fColor, int sColor) {
    // Set number of threads explicitly
    int num_threads = std::thread::hardware_concurrency()-2;
    spdlog::info("findMatching elements: Starting tasks using {} threads", num_threads);

    cv::Mat templ = cv::imread(croppedImage.ToStdString());
    cv::Mat scene = cv::imread(mainImage.ToStdString());

    if (templ.empty() || scene.empty()) {
        spdlog::error("Failed to load images");
        return false;
    }

    std::vector<double> scales;
    for (double s = MIN_SCALE; s <= MAX_SCALE; s += SCALE_STEP) {
        scales.push_back(s);
    }

    std::vector<double> baseAngles = {0, 90, 180, 270};
    const double threshold = GLOBAL_ACCURACY_THRESHOLD / 100.0;

    // Map of base rotations
    std::map<double, cv::Mat> baseRotations;
    for (double base : baseAngles) {
        baseRotations[base] = rotateAndScale(templ, base, 1.0);
    }

    // Parallel setup
    std::vector<MatchResult> globalResults;
    std::mutex resultsMutex;

    std::atomic<size_t> completedTasks(0);
    const size_t totalTasks = scales.size() * baseAngles.size();

    std::vector<std::thread> threads;
    size_t nextTaskIndex = 0;

    // Threads starting
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            while (true) {

                size_t taskIndex = nextTaskIndex++;
                if (taskIndex >= totalTasks) break;

                size_t s_idx = taskIndex / baseAngles.size();
                size_t b_idx = taskIndex % baseAngles.size();

                double scale = scales[s_idx];
                double baseAngle = baseAngles[b_idx];

                auto taskResults = findMatchingElementsParallel(
                    baseRotations[baseAngle],
                    scene,
                    baseAngle,
                    scale,
                    threshold,
                    fColor,
                    sColor,
                    croppedImageID,
                    decomposedImageID
                );

                {
                    std::lock_guard<std::mutex> lock(resultsMutex);
                    globalResults.insert(globalResults.end(), taskResults.begin(), taskResults.end());
                }

                size_t completed = ++completedTasks;
                double progressPercent = (completed * 100.0) / totalTasks;

                {
                    std::lock_guard<std::mutex> lock(cout_mutex);

                    std::ostringstream tid;
                    tid << std::this_thread::get_id();
                    std::string threadId = tid.str();

                    spdlog::info("Thread {} COMPLETED: scale={:.2f}, angle={} | Matches: {} | Progress: {}/{} ({:.1f}%)",
                        threadId, scale, baseAngle,
                        taskResults.size(),
                        completed,
                        totalTasks,
                        progressPercent);
                }
            }
        });
    }

    // Waiting for threads
    for (auto& t : threads) {
        t.join();
    }

    spdlog::info("Applying NMS for {} results", globalResults.size());
    applyNonMaximumSuppression(globalResults);
    spdlog::info("NMS applied. Results left: {}", globalResults.size());


    std::sort(globalResults.begin(), globalResults.end(), [](const MatchResult& a, const MatchResult& b) {
        return a.accuracy > b.accuracy;
    });

    spdlog::info("Found {} matches", globalResults.size());
    saveToJson(globalResults, true);

    return !globalResults.empty();
}

bool core::drawRectanglesFromJson(wxString jsonPath, double accuracy, bool accuracyDrawing) {

    std::string imagePath;
    wxString workingJsonPath = jsonPath;

    if (accuracyDrawing) {
        workingJsonPath = "../tmp/jsons/" + jsonPath + ".json";
    }

    if (firstDrawing || accuracyDrawing) {
        imagePath = "../tmp/images/forAnalyzing.jpg";
    } else {
        imagePath = "../tmp/analyzed/analyzedCombined.jpg";
    }

    cv::Mat image = cv::imread(imagePath);

    std::ifstream file(workingJsonPath.ToStdString());

    if (!file.is_open()) {
        spdlog::error("drawRectanglesFromJson: Cannot read JSON {}", workingJsonPath.ToStdString());
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
            int id = element["Meta.croppedID"];
            int decomposedId = element["Meta.decomposedID"];
            int x = element["Pos.X"];
            int y = element["Pos.Y"];
            int w = element["Pos.Width"];
            int h = element["Pos.Height"];
            float acc = element["Pos.Accuracy"];
            int fistColor = element["Color.First"];
            int secondColor = element["Color.Second"];

            if (x < 0 || y < 0 || w <= 0 || h <= 0 ||
                (x + w) > image.cols ||
                (y + h) > image.rows) {
                spdlog::warn("drawRectanglesFromJson: Skipped bad rectangle (x={}, y={}, w={}, h={})", x, y, w, h);
                continue;
            }

            if (acc >= accuracy) {
                cv::rectangle(image,
                cv::Point(x, y),
                cv::Point(x + w, y + h),
                cv::Scalar(colors[fistColor].Red(), colors[fistColor].Green(), colors[fistColor].Blue(), 200),
                1);

                cv::circle(image,
                    cv::Point(x + w/2, y + h/2),
                    1,
                    cv::Scalar(colors[secondColor].Red(), colors[secondColor].Green(), colors[secondColor].Blue(), 200),
                    -1);

                spdlog::info("drawRectanglesFromJson: Rectangle drawn");
            } else {
                spdlog::warn("drawRectanglesFromJson: Skipped rectangle based on accuracy (accuracy of rectangle = {}, minimal accuracy required = {})", acc, accuracy);
            }

        } catch (const json::exception& e) {
            spdlog::error("drawRectanglesFromJson: Cannot read JSON: {}", e.what());
        }
    }

    std::string outputPath = "../tmp/analyzed/analyzedCombined.jpg";

    if (accuracyDrawing) {
        outputPath = "../tmp/analyzed/" + jsonPath.ToStdString() + ".jpg";
    }

    if (!accuracyDrawing) {
        if (!cv::imwrite(outputPath, image)) {
            spdlog::error("drawRectanglesFromJson: Cannot save {}", outputPath);
            return false;
        }
    } else {
        if (!cv::imwrite(outputPath, image)) {
            spdlog::error("drawRectanglesFromJson: Cannot save {}", outputPath);
            return false;
        }
    }

    spdlog::info("drawRectanglesFromJson: Saved {}", outputPath);
    return true;
}

bool core::saveWithNewAccuracy(wxString existingJsonPath, wxString newJsonPath, double accuracy) {

    std::string path;
    spdlog::info("saveWithNewAccuracy: saving json file with minimal accuracy {}", accuracy);

    std::ifstream file(existingJsonPath.ToStdString());

    if (!file.is_open()) {
        spdlog::error("saveWithNewAccuracy: Cannot read old JSON {}", existingJsonPath.ToStdString());
        return false;
    }

    json existingJsonData;
    try {
        file >> existingJsonData;
    } catch (const json::parse_error& e) {
        spdlog::error("drawRectanglesFromJson: JSON error: {}", e.what());
        return false;
    }

    std::vector<MatchResult> results;
    json j;
    for (const auto& data : existingJsonData) {
        if (data["Pos.Accuracy"] > accuracy) {
            json r;
            r["Meta.croppedID"] = data["Meta.croppedID"];
            r["Meta.decomposedID"] = data["Meta.decomposedID"];

            r["Pos.X"] = data["Pos.X"];
            r["Pos.Y"] = data["Pos.Y"];

            r["Pos.Width"] = data["Pos.Width"];
            r["Pos.Height"] = data["Pos.Height"];

            r["Pos.Accuracy"] = data["Pos.Accuracy"];
            r["Pos.Scale"] = data["Pos.Scale"];
            r["Pos.Rotation"] = data["Pos.Rotation"];

            r["Color.First"] = data["Color.First"];
            r["Color.Second"] = data["Color.Second"];
            j.push_back(r);
        }
    }

    std::ofstream newFile(newJsonPath.ToStdString());
    newFile << j.dump(4);
    newFile.close();
    spdlog::info("saveWithNewAccuracy: json file saved {}", newJsonPath.ToStdString());

    return true;
}

bool core::NMSForAllResultsCombined() {
    std::vector<MatchResult> resultsCombined;

    fs::create_directories("..\\tmp\\jsons\\resultsCombined");

    for (const auto& croppedImageEntry : fs::directory_iterator("..\\tmp\\jsons")) {
        if (!croppedImageEntry.is_directory()) {
            std::string fileName = croppedImageEntry.path().filename().string();
            std::ifstream file(fileName);
            json jsonData;

            try {
                file >> jsonData;
            } catch (const json::parse_error& e) {
                spdlog::error("drawRectanglesFromJson: JSON error: {}", e.what());
                return false;
            }

            for (const auto& element : jsonData) {
                MatchResult resultTMP;
                resultTMP.croppedID = element["Meta.croppedID"];
                resultTMP.decomposedID = element["Meta.decomposedID"];
                resultTMP.posX = element["Pos.X"];
                resultTMP.posY = element["Pos.Y"];
                resultTMP.width = element["Pos.Width"];
                resultTMP.height = element["Pos.Height"];
                resultTMP.accuracy = element["Pos.Accuracy"];
                resultTMP.fColor = element["Color.First"];
                resultTMP.sColor = element["Color.Second"];

                resultsCombined.push_back(resultTMP);
            }
        }
    }

    applyNonMaximumSuppression(resultsCombined);

    if (!saveToJson(resultsCombined, false)) {
        return false;
    }

    return true;
}


