#include "CallQuanto.hpp"	

CallQuanto::CallQuanto(PnlVect* dates_, double strike_)
{
    strike = strike_;
    dates = dates_;
}
double CallQuanto::payoff(const PnlMat* path) {

    double payoff = pnl_mat_get(path, path->m - 1, 0) - strike;
    if (payoff > 0) {
        return payoff;
    } else {
        return 0;
    }   
}
CallQuanto::~CallQuanto() {
    //pnl_vect_free(dates);
}