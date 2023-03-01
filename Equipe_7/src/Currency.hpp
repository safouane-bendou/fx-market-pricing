#pragma once

#include "RiskyAsset.hpp"

class Currency : public RiskyAsset
{
  public:
    double interestRate_;
    double domesticInterestRate_;

    Currency(double drift_, PnlVect * volatilityVectorPnlVect_, double interestRate, double domesticInterestRate);
    ~Currency();
};
