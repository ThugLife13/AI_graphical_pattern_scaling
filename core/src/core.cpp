#include "../lib/core.h"

core::core() {
}

core::~core() {
}

bool core::generate(wxString filePath) {

    spdlog::info("generate: Clearing images");

    if(!clearImages()) {
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




