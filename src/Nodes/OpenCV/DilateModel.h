#ifndef DILATEMODEL_H
#define DILATEMODEL_H

#include "Nodes/OpenCV/MorphologyModelBase.h"

class DilateModel final : public MorphologyModelBase {
public:
    DilateModel();

    ~DilateModel() override;
};

#endif //DILATEMODEL_H
