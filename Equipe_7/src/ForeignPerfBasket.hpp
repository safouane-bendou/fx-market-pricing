#pragma once
#include "Option.hpp"
#include <vector>
#include <algorithm>
#include <numeric>

class ForeignPerfBasket : public Option
{
  public:
    double strike;
    std::vector<int> assetCurrencyMapping; // first element is n0, last element is n_barre
    // nbTimeSteps = 3 
    ForeignPerfBasket(PnlVect* dates, double strike_, std::vector<int> assetCurrencyMapping_);
    double payoff(const PnlMat* path);
    int findMaxIndex(std::vector<double> list);
    ~ForeignPerfBasket();
};