#include "../lib/core.h"

core::core() {
}

core::~core() {
}

bool core::generate(wxString filePath) {
    spdlog::info("generate: Clearing images");

    if(!clearImages()) {
        return false;
    }

    spdlog::info("generate: Path to image {}", filePath.ToStdString());

/*
    if (straightUpImage(filePath.ToStdString())) {
        spdlog::info("generate: Image straighted up");
        if (generateImages()) {
            spdlog::info("generate: Image generated - returning true");
            return true;
        }
        else {
            spdlog::info("generate: Image failed to generate - returning false");
            return false;
        }
    } else {
        spdlog::info("generate: Image failed to straight up - returning false");
        return false;
    }
*/
    return true;
}

bool core::clearImages() {
    std::string folderPath = "..\\tmp\\images";
    try {
        if (!fs::exists(folderPath)) {
            spdlog::error("clearImages: Folder does not exist {} ", folderPath);
            return false;
        }

        for (const auto& entry : fs::directory_iterator(folderPath)) {
            if (fs::is_regular_file(entry.path())) {
                fs::remove(entry.path());
            } else {
                spdlog::info("clearImages: Folder does not exist {} ", entry.path().string());
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

bool core::straightUpImage(std::string filePath) {
    spdlog::info("straightUpImage: Path to image {}", filePath.c_str());
    //TODO straightening up an image
    //TODO save straightened image to localization: /tmp/images/straight.jpg

    // Load the image
    cv::Mat image = cv::imread(filePath);
    if (image.empty()) {
        spdlog::error("Failed to load image from {}", filePath.c_str());
        return false;
    }

    cv::Mat straightened;

    // Save the straightened image
    std::string outputPath = "D:\\Projects\\AI_graphical_pattern_scaling\\tmp\\images\\straight.jpg";
    if (!cv::imwrite(outputPath, straightened)) {
        spdlog::error("Failed to save straightened image to {}", outputPath);
        return false;
    }

    spdlog::info("Successfully straightened and saved the image to {}", outputPath);
    return true;
}

bool core::generateImages() {
    //TODO creating images with shown located figures and objects
    /*
    // Ścieżka do obrazu wejściowego
    const std::string imagePath = "../tmp/images/straight.jpg";
    const std::string outputPath = "../tmp/images/";

   // Wczytaj obraz
    cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);
    if (image.empty()) {
        std::cerr << "Nie udało się wczytać obrazu: " << imagePath << std::endl;
        return false;
    }

    // Konwersja na skalę szarości
    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

    // Redukcja szumów za pomocą rozmycia medianowego
    cv::Mat blurred;
    cv::medianBlur(gray, blurred, 5);

    // Progowanie adaptacyjne
    cv::Mat adaptiveThresh;
    cv::adaptiveThreshold(blurred, adaptiveThresh, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 11, 2);

    // Operacje morfologiczne (zamykanie, aby usunąć drobne przerwy)
    cv::Mat morphed;
    cv::morphologyEx(adaptiveThresh, morphed, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));

    // Wykrywanie krawędzi za pomocą Canny
    cv::Mat edges;
    cv::Canny(morphed, edges, 50, 150);

    // Wykrycie konturów
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(edges, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // Filtrowanie konturów na podstawie obszaru
    const int minContourArea = 500; // Minimalny obszar konturu
    std::vector<std::vector<cv::Point>> filteredContours;
    for (const auto& contour : contours) {
        if (cv::contourArea(contour) > minContourArea) {
            filteredContours.push_back(contour);
        }
    }

    // Obraz wynikowy
    cv::Mat combinedImage = image.clone();
    std::map<int, cv::Mat> vertexImages;

    // Procesowanie konturów
    for (const auto& contour : filteredContours) {
        // Aproksymacja konturów
        std::vector<cv::Point> approx;
        cv::approxPolyDP(contour, approx, 0.01 * cv::arcLength(contour, true), true);

        int vertexCount = approx.size(); // Liczba wierzchołków
        if (vertexCount < 2) continue;

        // Generowanie otoczki wypukłej (dodatkowa dokładność)
        std::vector<cv::Point> hull;
        cv::convexHull(approx, hull);

        // Stworzenie obrazu dla danej liczby wierzchołków
        if (vertexImages.find(vertexCount) == vertexImages.end()) {
            vertexImages[vertexCount] = cv::Mat::zeros(image.size(), image.type());
        }

        // Kolory dla różnych liczby wierzchołków
        cv::Scalar color;
        switch (vertexCount) {
            case 2: color = cv::Scalar(255, 0, 0); break; // Niebieski
            case 3: color = cv::Scalar(0, 255, 0); break; // Zielony
            case 4: color = cv::Scalar(0, 0, 255); break; // Czerwony
            default: color = cv::Scalar(255, 255, 0); break; // Żółty
        }

        // Rysowanie konturów i wierzchołków
        cv::drawContours(vertexImages[vertexCount], std::vector<std::vector<cv::Point>>{hull}, -1, color, 2);
        cv::drawContours(combinedImage, std::vector<std::vector<cv::Point>>{hull}, -1, color, 2);

        for (const auto& point : hull) {
            cv::circle(vertexImages[vertexCount], point, 5, color, -1);
            cv::circle(combinedImage, point, 5, color, -1);
        }
    }

    // Zapisanie wynikowych obrazów
    cv::imwrite(outputPath + "gray.jpg", gray);
    cv::imwrite(outputPath + "adaptive_thresh.jpg", adaptiveThresh);
    cv::imwrite(outputPath + "morphed.jpg", morphed);
    cv::imwrite(outputPath + "edges.jpg", edges);
    cv::imwrite(outputPath + "combined.jpg", combinedImage);

    for (const auto& [vertexCount, vertexImage] : vertexImages) {
        cv::imwrite(outputPath + "vertices_" + std::to_string(vertexCount) + ".jpg", vertexImage);
    }
*/
    return true;
}


