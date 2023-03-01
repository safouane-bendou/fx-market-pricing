#include "MonteCarlo.hpp"
using namespace std;


  /**
     Constructeur à cinq arguments d'un objet Monte carlo
  */

MonteCarlo::MonteCarlo(BlackScholesModel* mod, Option* opt, PnlRng* rng, double fdStep, long nbSamples)
{
    mod_ = mod;
    opt_ = opt;
    rng_ = rng;
    fdStep_ = fdStep;
    nbSamples_ = nbSamples;
}


/**
 * Calcule le prix de l'option à la date 0
 *
 * @param[out] prix valeur de l'estimateur Monte Carlo
 * @param[out] ic écart type de l'estimateur
 */
void MonteCarlo::price(double& prix, double& std_dev){

    int nbTimeSteps = opt_->dates->size;
    double domesticInterestRate = mod_->currencies_[0]->domesticInterestRate_;
    double maturity = pnl_vect_get(opt_->dates, nbTimeSteps - 1);
    PnlMat* path = pnl_mat_create (nbTimeSteps + 1, mod_->size);   
    
    double sum = 0;
    double sumDesCarres = 0;
    
    for(int round = 0; round < nbSamples_; round++){
        mod_->asset(path, opt_->dates, rng_);
        sum += opt_->payoff(path);
        sumDesCarres += pow(opt_->payoff(path), 2);  
    }

    //pnl_mat_print(path);
    
    double ourPrice = (sum/nbSamples_)*exp(-domesticInterestRate * maturity);
    prix = ourPrice; // Valeur de l'option 
    double s = pow(sum/nbSamples_, 2);
    double volatility = (sumDesCarres/nbSamples_ - s)*exp(-2*domesticInterestRate * maturity);
    std_dev = sqrt(volatility/nbSamples_); //valeur de l'écart-type
    pnl_mat_free(&path);
    
}

/**
 * Calcule le prix de l'option à la date t
 *
 * @param[in]  past contient la trajectoire du sous-jacent
 * jusqu'à l'instant t
 * @param[in] t date à laquelle le calcul est fait
 * @param[out] prix contient le prix
 * @param[out] std_dev contient l'écart type de l'estimateur
 */
void MonteCarlo::price(const PnlMat* past, double t, double& prix, double& std_dev)
{
    int nbTimeSteps = opt_->dates->size;
    double domesticInterestRate = mod_->currencies_[0]->domesticInterestRate_;
    double maturity = pnl_vect_get(opt_->dates, nbTimeSteps - 1);
    PnlMat* path = pnl_mat_create(nbTimeSteps + 1, mod_->size);  
    double sum = 0;
    double sumOfSquares = 0;
    for(int round = 0; round < nbSamples_; round++)
    {
        mod_->asset(path, t, opt_->dates, rng_, past);
        sum += opt_->payoff(path);
        sumOfSquares += pow(opt_->payoff(path), 2);  
    }
    double ourPrice = (sum/(double)nbSamples_) * exp(-domesticInterestRate * (maturity - t));
    prix = ourPrice; // Valeur de l'option 
    // Calcul de l'écart-type
    double s = pow(sum / nbSamples_,2);
    double volatility = (sumOfSquares/nbSamples_ - s) * exp(-2 * domesticInterestRate * (maturity - t));
    std_dev = sqrt(volatility / nbSamples_); //valeur de l'écart-type
    pnl_mat_free(&path);

}





void MonteCarlo::delta(PnlVect* delta, PnlVect* std_dev)
{
    // Calcul du delta en t = 0
    double workingDays = 252;
    int nbTimeSteps = opt_->dates->size;
    double domesticInterestRate = mod_->currencies_[0]->domesticInterestRate_;
    double maturity = pnl_vect_get(opt_->dates, nbTimeSteps - 1);
    PnlMat* path = pnl_mat_create (nbTimeSteps + 1, mod_->size);
    PnlMat* shift_path =  pnl_mat_create (nbTimeSteps + 1, mod_->size);

    for(int round = 0; round < nbSamples_; round++)
    {
        mod_->asset(path, opt_->dates, rng_);
        for(int d = 0; d < mod_->size; d++)
        {
            double shareDelta = pnl_vect_get(delta, d);
            double deviation = pnl_vect_get(std_dev, d);
            mod_->shiftAsset(shift_path, path, d, fdStep_, 0, opt_->dates);
            double payoffUp = opt_->payoff(shift_path);
            mod_->shiftAsset(shift_path, path, d, -fdStep_, 0, opt_->dates);
            double payoffDown = opt_->payoff(shift_path);
            shareDelta += payoffUp - payoffDown;
            deviation += pow(payoffUp - payoffDown, 2);
            pnl_vect_set(delta, d, shareDelta);
            pnl_vect_set(std_dev, d, deviation);
        }
    }
    PnlVect* ourSpots = mod_->spot_;
    for(int d = 0; d < mod_->size; d++)
    {
        double initialValuePerAction = pnl_vect_get(ourSpots, d);
        double shareDelta = pnl_vect_get(delta, d);
        shareDelta = exp(-domesticInterestRate * maturity) * shareDelta / (2 * nbSamples_ * fdStep_ * initialValuePerAction);
        pnl_vect_set(delta, d, shareDelta); //valeurs des deltas
        double deviation = pnl_vect_get(std_dev, d);
        deviation = exp(-2 * domesticInterestRate * maturity) * deviation / (nbSamples_ * pow(2 * fdStep_ * initialValuePerAction, 2));
        deviation -= pow(shareDelta, 2);
        deviation = sqrt(deviation / (double)nbSamples_);
        pnl_vect_set(std_dev, d, deviation); // valeurs des ecart-type 
    }   
    pnl_mat_free(&path);
    pnl_mat_free(&shift_path);

}




void MonteCarlo::delta(const PnlMat* past, double t, PnlVect* delta, PnlVect* std_dev)
{
    // calcul du delta en t
    int nbTimeSteps = opt_->dates->size;
    double domesticInterestRate = mod_->currencies_[0]->domesticInterestRate_;
    double maturity = pnl_vect_get(opt_->dates, nbTimeSteps - 1);
    PnlMat* path = pnl_mat_create (nbTimeSteps + 1, mod_->size);
    PnlMat* shift_path =  pnl_mat_create (nbTimeSteps + 1, mod_->size);
    for(int round = 0; round < nbSamples_; round++)
    {
        mod_->asset(path, t, opt_->dates, rng_, past);
        for(int d = 0; d < mod_->size; d++)
        {
            double shareDelta = pnl_vect_get(delta, d);
            double deviation = pnl_vect_get(std_dev, d);
            mod_->shiftAsset(shift_path, path, d, fdStep_, t, opt_->dates);
            double payoffUp = opt_->payoff(shift_path);
            mod_->shiftAsset(shift_path, path, d, -fdStep_, t, opt_->dates);
            double payoffDown = opt_->payoff(shift_path);
            shareDelta += payoffUp - payoffDown;
            deviation += pow(payoffUp - payoffDown, 2);
            pnl_vect_set(delta, d, shareDelta);
            pnl_vect_set(std_dev, d, deviation);
        }
    }
    PnlVect* ourShares = pnl_vect_create(mod_->size);
    pnl_mat_get_row(ourShares, past, past->m - 1);
    for(int d = 0; d < mod_->size; d++)
    {
        double saved = pnl_vect_get(ourShares, d);
        double shareDelta = pnl_vect_get(delta, d);
        shareDelta = exp(-domesticInterestRate * maturity) * shareDelta / (2 * nbSamples_ * fdStep_ * saved);
        pnl_vect_set(delta, d, shareDelta); //valeurs des deltas
        double deviation = pnl_vect_get(std_dev, d);
        deviation = exp(-2 * domesticInterestRate * maturity) * deviation / (nbSamples_ * pow(2 * fdStep_ * saved, 2));
        deviation -= pow(shareDelta, 2);
        deviation = sqrt(deviation / nbSamples_);
        pnl_vect_set(std_dev, d, deviation); // valeurs des ecart-type 
    }   
    pnl_mat_free(&path);
    pnl_mat_free(&shift_path);

}

/**
     Met à jour au fur et à mesure la composition et les dates de past
 */

void MonteCarlo::makeReguralizedPast(PnlMat * past, PnlVect * shares, int i, double h)
{
    int nbTimeSteps = opt_->dates->size;
    int reguralizedIndex = (i - 1) * nbTimeSteps / h;
    ((i - 1) * nbTimeSteps / h == reguralizedIndex) ? pnl_mat_add_row(past, past->m, shares) : pnl_mat_set_row(past, shares, past->m - 1);
}




vector<Position> MonteCarlo::portfolioValues(PnlMat * marketData, double &error)
{
    int nbTimeSteps = opt_->dates->size;
    double maturity = pnl_vect_get(opt_->dates, nbTimeSteps - 1);
    double domesticInterestRate = mod_->currencies_[0]->domesticInterestRate_;
    PnlMat * past = pnl_mat_create(1, marketData->n);
    PnlVect * deltas = pnl_vect_create(marketData->n);
    PnlVect * currentDeltas = pnl_vect_create(marketData->n);
    PnlVect * currentDeltaStdDev = pnl_vect_create_from_zero(marketData->n);
    PnlVect * differenceDeltas = pnl_vect_create(marketData->n);
    PnlVect * deltaStdDev = pnl_vect_create_from_zero(marketData->n);
    delta(deltas, deltaStdDev);
    PnlVect * currentShares = pnl_vect_create(marketData->n);
    pnl_mat_get_row(currentShares, marketData, 0);
    pnl_mat_set_row(past, currentShares, 0);
    double stdDev;
    double value;
    double prix;
    double std_dev;
    price(prix, stdDev);
    double notRisky = prix - pnl_vect_scalar_prod(deltas, currentShares);
    double risky = pnl_vect_scalar_prod(deltas, currentShares);

    vector<Position> positions = vector<Position>();
    Position position = Position(0, prix, stdDev, deltas, deltaStdDev, notRisky + risky);
    positions.push_back(position);
    for(int i = 1; i < marketData->m; i++)
    {
        pnl_mat_get_row(currentShares, marketData, i);
        makeReguralizedPast(past, currentShares, i, marketData->m - 1);
        price(past, i * maturity / (double)(marketData->m - 1), prix, std_dev);
        delta(past, i * maturity / (double)(marketData->m - 1), currentDeltas, currentDeltaStdDev);
        pnl_vect_clone(differenceDeltas, currentDeltas);
        pnl_vect_minus_vect(differenceDeltas, deltas);
        notRisky = notRisky * exp(domesticInterestRate * maturity / marketData->m);
        notRisky -= pnl_vect_scalar_prod(differenceDeltas, currentShares);
        risky = pnl_vect_scalar_prod(currentDeltas, currentShares);
        deltas = pnl_vect_copy(currentDeltas);
        deltaStdDev = pnl_vect_copy(currentDeltaStdDev);
        Position positionLoop = Position(i, prix, std_dev, deltas, deltaStdDev, notRisky + risky);
        positions.push_back(positionLoop);
    }
    error = value + pnl_vect_scalar_prod(currentDeltas, currentShares) - opt_->payoff(past);
   
    pnl_vect_free(&currentDeltas);
    pnl_vect_free(&currentShares);
    pnl_vect_free(&currentShares);
    pnl_vect_free(&differenceDeltas);
    pnl_mat_free(&past);
  
    return positions;
}

// Destructeur de la classe Monte Carlo
MonteCarlo::~MonteCarlo(){
    pnl_rng_free(&rng_);
}



