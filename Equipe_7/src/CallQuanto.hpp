#pragma once
#include "Option.hpp"
#include "pnl/pnl_vector.h"
#include "pnl/pnl_matrix.h"

class CallQuanto : public Option
{
  public:
    double strike;
    CallQuanto(PnlVect* dates_, double strike);
    double payoff(const PnlMat* path);
    ~CallQuanto();
};