#ifndef ERODEMODEL_H
#define ERODEMODEL_H

#include "Nodes/OpenCV/MorphologyModelBase.h"

class ErodeModel final : public MorphologyModelBase {
public:
    ErodeModel();

    ~ErodeModel() override;
};

#endif //ERODEMODEL_H
