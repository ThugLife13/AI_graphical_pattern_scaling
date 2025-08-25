//
// Created by Mateusz on 18.06.2025.
//

#ifndef AIGENERATION_H
#define AIGENERATION_H

#include "core.h"

class aiGeneration {
    public:
        aiGeneration();
        ~aiGeneration();

    void startGeneration(int sizeX, int sizeY);

    private:
    int newSizeX;
    int newSizeY;
    bool generation();
};



#endif //AIGENERATION_H
