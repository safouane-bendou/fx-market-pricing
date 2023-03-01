#pragma once
#include "Option.hpp"

class ForeignAsian : public Option
{
  public:
    ForeignAsian(PnlVect* dates_);
    double payoff(const PnlMat* path);
    ~ForeignAsian();
};