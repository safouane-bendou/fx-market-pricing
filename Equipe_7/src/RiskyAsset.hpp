#pragma once

#include "pnl/pnl_vector.h"
#include "pnl/pnl_matrix.h"

/// \brief Classe Option abstraite
class RiskyAsset
{
  public:
    double drift;
    PnlVect * volatilityVector;   
};
