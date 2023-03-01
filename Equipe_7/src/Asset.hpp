#pragma once

#include "RiskyAsset.hpp"

class Asset : public RiskyAsset
{
  public:
    double domesticInterestRate_;

    Asset(double drift_, PnlVect * volatilityVector_, double domesticInterestRate);
    ~Asset();
};
