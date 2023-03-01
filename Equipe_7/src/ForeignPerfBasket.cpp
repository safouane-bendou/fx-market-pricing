#include "ForeignPerfBasket.hpp"



ForeignPerfBasket::ForeignPerfBasket(PnlVect* dates_, double strike_, std::vector<int> assetCurrencyMapping_)
{
    dates = dates_;
    strike = strike_;
    assetCurrencyMapping = assetCurrencyMapping_;
    
}
double ForeignPerfBasket::payoff(const PnlMat* path)
{
    int nombreDeDevisesEtrangeres = assetCurrencyMapping[assetCurrencyMapping.size() - 1];
    std::vector<double> vecDesPi = std::vector<double>();
    int colIndex = assetCurrencyMapping[0];
    int deviseIndex = std::accumulate(assetCurrencyMapping.begin(), assetCurrencyMapping.end(), 0) - nombreDeDevisesEtrangeres ;
    for (int i = 1; i < nombreDeDevisesEtrangeres+1; i++) {
        int nombreDesActifsEtrangers = assetCurrencyMapping[i];
        double somme = 0;
        double somme2 = 0;
        
        for (int j = 0; j < nombreDesActifsEtrangers; j++) {
            somme += pnl_mat_get(path, 2, colIndex+j) / pnl_mat_get(path, 2, deviseIndex);
            somme2 += pnl_mat_get(path, 1, colIndex+j) / pnl_mat_get(path, 1, deviseIndex);
        }
        deviseIndex += i;
        vecDesPi.push_back(somme / somme2); 
        colIndex += nombreDesActifsEtrangers;
    }
    int maxPiIndex = findMaxIndex(vecDesPi);
    int nbrAssets = assetCurrencyMapping[maxPiIndex + 1];
    int startingIndex = 0;
    for (int i = 0; i < maxPiIndex+1; i++) {
        startingIndex += assetCurrencyMapping[i];
    }
    double sommeEtrangere = 0;
    double sommeDomestique = 0;
    for (int j = 0; j < assetCurrencyMapping[0]; j++)
    {
        sommeDomestique += pnl_mat_get(path, path->m - 1, j);
    }
    for (int j = 0; j < nbrAssets; j++)
    {
        sommeEtrangere += pnl_mat_get(path, path->m - 1, startingIndex+j);
    }
    double mypayoff = sommeEtrangere / (double)nbrAssets - sommeDomestique / (double)assetCurrencyMapping[0] - strike;
    if (mypayoff > 0) {
        return mypayoff;
    } else {
        return 0;
    }
}
ForeignPerfBasket::~ForeignPerfBasket() {
    //pnl_vect_free(dates);
}

int ForeignPerfBasket::findMaxIndex(std::vector<double> list)
{
    return std::distance(list.begin(), std::max_element(list.begin(), list.end()));
}