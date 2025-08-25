//
// Created by Mateusz on 18.06.2025.
//

#include "../lib/aiGeneration.h"

aiGeneration::aiGeneration() {
}

aiGeneration::~aiGeneration() {
}

void aiGeneration::startGeneration(int sizeX, int sizeY) {
    newSizeX = sizeX;
    newSizeY = sizeY;
    generation();
}

bool aiGeneration::generation() {
    return false;
}
