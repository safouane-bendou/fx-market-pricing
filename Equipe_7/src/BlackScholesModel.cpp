#include <iostream>
#include "BlackScholesModel.hpp"
using namespace std;

 /**
     Constructeur à quatre paramètres définissant le modèle BlackScholes 
     */

BlackScholesModel::BlackScholesModel(vector<Asset *> assets, vector<Currency *> currencies, PnlVect* spot)
{
    assets_ = assets;
    currencies_ = currencies;
    spot_ = pnl_vect_create(spot->size);
    pnl_vect_clone(spot_, spot);
    size = assets.size() + currencies.size(); 
    gaussianVector = pnl_vect_create(size);
    currentShares = pnl_vect_create(size);
    nextShares = pnl_vect_create(size);
}


void BlackScholesModel::asset(PnlMat* path, PnlVect * dates, PnlRng* rng)
{
    PnlMat* gaussian = pnl_mat_create(dates->size, size);
    pnl_mat_rng_normal(gaussian, dates->size + 1, size, rng);
    pnl_mat_set_row(path, spot_, 0);
    pnl_mat_get_row(currentShares, path, 0);
    double time = 0;
    for(int i = 1; i < dates->size + 1; i++)
    {
        double step = (pnl_vect_get(dates, i - 1) - time);
        pnl_mat_get_row(gaussianVector, gaussian, i);
        for(int d = 0; d < assets_.size(); d++)
        {
            double volatility = pnl_vect_scalar_prod(assets_[d]->volatilityVector, assets_[d]->volatilityVector);
            double scale = pnl_vect_scalar_prod(assets_[d]->volatilityVector, gaussianVector);
            double computedShare = pnl_vect_get(currentShares, d) * exp((assets_[d]->drift - volatility / 2) * step + sqrt(step) * scale);
            pnl_vect_set(nextShares, d, computedShare);
        }
        for(int d = 0; d < currencies_.size(); d++)
        {
            double volatility = pnl_vect_scalar_prod(currencies_[d]->volatilityVector, currencies_[d]->volatilityVector);
            double scale = pnl_vect_scalar_prod(currencies_[d]->volatilityVector, gaussianVector);
            double computedShare = pnl_vect_get(currentShares, d + assets_.size()) * exp((currencies_[d]->drift - volatility / 2) * step + sqrt(step) * scale);
            pnl_vect_set(nextShares, d + assets_.size(), computedShare);
        }
        pnl_mat_set_row(path, nextShares, i);
        pnl_vect_clone(currentShares, nextShares);
        time = pnl_vect_get(dates, i - 1);
    }
    pnl_mat_free(&gaussian);
}


void BlackScholesModel::asset(PnlMat* path, double t, PnlVect * dates, PnlRng* rng, const PnlMat* past)
{   
    int startingStep;
    PnlVect *savedSpot = pnl_vect_create(size);
    PnlMat *blockPast = pnl_mat_create(past->m - 1, size);
    pnl_mat_get_row(savedSpot, past, past->m - 1);
    pnl_mat_extract_subblock(blockPast, past, 0, past->m - 1, 0, size);
    PnlVect *nextShares = pnl_vect_create(size);
    bool isMonitoringDate = false;
    for(int i = 0; i < dates->size; i++)
    {
        if(pnl_vect_get(dates, i) == t)
        {
            isMonitoringDate = true;
            break;
        }
    }
    if(isMonitoringDate) 
    {
        pnl_mat_set_subblock(path, past, 0, 0);
        startingStep = past->m;
    } 
    else
    {
        pnl_mat_set_subblock(path, blockPast, 0, 0);
        startingStep = blockPast->m;
    }

    PnlMat *gaussian = pnl_mat_create(dates->size + 1 - startingStep, size);
    pnl_mat_rng_normal(gaussian, dates->size + 1, size, rng);
    for (int i = startingStep; i < dates->size + 1; i++) {
        PnlVect *gaussianVector = pnl_vect_create(size);
        pnl_mat_get_row(gaussianVector, gaussian, i);
        double indexedDate = pnl_vect_get(dates, i - 1);
        
        for (int d = 0; d < assets_.size(); d++) {
            double volatility = pnl_vect_scalar_prod(assets_[d]->volatilityVector, assets_[d]->volatilityVector);
            double scale = pnl_vect_scalar_prod(assets_[d]->volatilityVector, gaussianVector);
            double computedShare = pnl_vect_get(savedSpot, d) * exp((assets_[d]->drift - volatility / 2) * (indexedDate - t) + sqrt(indexedDate - t) * scale);
            pnl_vect_set(nextShares, d, computedShare);
        }
        for(int d = 0; d < currencies_.size(); d++)
        {
            double volatility = pnl_vect_scalar_prod(currencies_[d]->volatilityVector, currencies_[d]->volatilityVector);
            double scale = pnl_vect_scalar_prod(currencies_[d]->volatilityVector, gaussianVector);
            double computedShare = pnl_vect_get(savedSpot, d + assets_.size()) * exp((currencies_[d]->drift - volatility / 2) * (indexedDate - t) + sqrt(indexedDate - t) * scale);
            pnl_vect_set(nextShares, d + assets_.size(), computedShare);
        }
        pnl_mat_set_row(path, nextShares, i);
    }
    pnl_mat_free(&blockPast);
    pnl_vect_free(&savedSpot);
    pnl_mat_free(&gaussian);
   
}


void BlackScholesModel::shiftAsset(PnlMat* shift_path, const PnlMat* path, int d, double h, double t, PnlVect * dates)
{
    int startingStep = 0;
    pnl_mat_clone(shift_path, path);
    bool isMonitoringDate = false;
    while (startingStep < dates->size + 1 && pnl_vect_get(dates, startingStep) < t) {
      startingStep++;
    }
    PnlVect* shiftComponent = pnl_vect_create(size);
    pnl_mat_get_col(shiftComponent, shift_path, d);
    for(int i = startingStep; i < shiftComponent->size; i++)
    {
        double shifted = (1 + h) * pnl_vect_get(shiftComponent, i);
        pnl_vect_set(shiftComponent, i, shifted);
    }
    pnl_mat_set_col(shift_path, shiftComponent, d);
    pnl_vect_free(&shiftComponent);
}






//Destructeur de la classe
   
BlackScholesModel::~BlackScholesModel(){
    pnl_vect_free(&sigma_);
    pnl_vect_free(&spot_);
    pnl_vect_free(&trend_);
    pnl_vect_free(&gaussianVector);
    pnl_vect_free(&currentShares);
    pnl_vect_free(&nextShares);

}











