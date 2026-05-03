#ifndef MORPHOLOGYEXMODEL_H
#define MORPHOLOGYEXMODEL_H

#include "Nodes/OpenCV/MorphologyModelBase.h"

class MorphologyExModel final : public MorphologyModelBase {
public:
    MorphologyExModel();

    ~MorphologyExModel() override;
};

#endif //MORPHOLOGYEXMODEL_H
