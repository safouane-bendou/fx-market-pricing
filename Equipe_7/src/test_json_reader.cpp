#include <iostream>
#include <fstream>
#include "json_reader.hpp"
#include "BlackScholesModel.hpp"
#include "MonteCarlo.hpp"
#include "CallCurrency.hpp"
#include "CallQuanto.hpp"
#include "ForeignAsian.hpp"
#include "ForeignPerfBasket.hpp"
#include "QuantoExchange.hpp"
#include "Position.hpp"

/*Converts each asset in market data to domestic*/
static void convertToDomestic(PnlMat * marketData, std::vector<int> assetCurrencyMapping)
{
    PnlVect * foreignShare = pnl_vect_create(marketData->m);
    PnlVect * foreignCurrency = pnl_vect_create(marketData->m);
    int start = assetCurrencyMapping[0];
    int nBarre = assetCurrencyMapping[assetCurrencyMapping.size() - 1];
    int sumNi = std::accumulate(assetCurrencyMapping.begin(), assetCurrencyMapping.end(), 0) - nBarre;
    for(int i = 1; i < assetCurrencyMapping.size() - 1; i++)
    {
        int ni = assetCurrencyMapping[i];
        pnl_mat_get_col(foreignCurrency, marketData, sumNi + i - 1);
        for(int d = start; d < start + ni; d++)
        {
            pnl_mat_get_col(foreignShare, marketData, d);
            pnl_vect_mult_vect_term(foreignShare, foreignCurrency);
            pnl_mat_set_col(marketData, foreignShare, d);
        }
        start += ni;
    }
}



static std::vector<int> getAssetCurrencyMapping(nlohmann::json jsonParams)
{
    std::vector<int> assetCurrencyMapping;
    auto jsonCurrencies = jsonParams.at("Currencies");
    auto jsonAssets = jsonParams.at("Assets");
    for (auto jsonCurrency : jsonCurrencies)
    {
        int compteur = 0;
        std::string currencyId = jsonCurrency.at("id").get<std::string>();
        for(auto jsonAsset : jsonAssets)
        {
            std::string assetId = jsonAsset.at("CurrencyId").get<std::string>();
            if(currencyId == assetId)
            {
            compteur++;
            }
        }
        assetCurrencyMapping.push_back(compteur);
    }
    int currencyNb = jsonParams.at("Currencies").size();
    assetCurrencyMapping.push_back(currencyNb);
    return assetCurrencyMapping;
}


static PnlVect * convertDatesToList(std::string type, nlohmann::json jsonParams)
{
    PnlVect * dates;
    int numberOfDaysPerYear = jsonParams.at("NumberOfDaysInOneYear").get<int>();
    if (type == "Grid") {
        jsonParams.at("Option").at("FixingDatesInDays").at("DatesInDays").get_to(dates);
    }
    if (type == "Fixed") {
        double period, maturity;
        jsonParams.at("Option").at("FixingDatesInDays").at("Period").get_to(period);
        jsonParams.at("Option").at("MaturityInDays").get_to(maturity);
        dates = pnl_vect_create(maturity / period);
        int compteur = period;
        for (int i = 0; i < dates->size; i++) {
            pnl_vect_set(dates, i, compteur);
            compteur += period;
        }
    }
    pnl_vect_mult_scalar(dates, 1 / (double) numberOfDaysPerYear);
    return dates;
}







int main(int argc, char **argv) {

    /*if (argc != 2) {
        std::cerr << "Wrong number of arguments. Exactly one argument is required" << std::endl;
        std::exit(0);
    }*/

    std::ifstream ifs(argv[1]);
    nlohmann::json jsonParams = nlohmann::json::parse(ifs);

    PnlMat *correlation;
    jsonParams.at("Correlations").get_to(correlation);
    pnl_mat_chol(correlation); //cholesky composition of the corelation matrix
    int numberOfDaysPerYear = jsonParams.at("NumberOfDaysInOneYear").get<int>();
    std::string domesticCurrencyId;
    jsonParams.at("DomesticCurrencyId").get_to(domesticCurrencyId);
    auto jsonCurrencies = jsonParams.at("Currencies");
    auto jsonAssets = jsonParams.at("Assets");
    int assetNb = jsonParams.at("Assets").size();
    int currencyNb = jsonParams.at("Currencies").size();

    //Finding the Domestic Interest Rate
    double domesticInterestRate = 0;
    for (auto jsonCurrency : jsonCurrencies) {
        std::string currencyId = jsonCurrency.at("id").get<std::string>();
        if (currencyId == domesticCurrencyId) {
            domesticInterestRate = jsonCurrency.at("InterestRate").get<double>();
            break;
        }
    }

    
    PnlVect* spot = pnl_vect_create(correlation->m); // the spot has to be extracted from the JSON file
    //Création du vecteur des assets
    vector<Asset*> assets = vector<Asset*>();
    int correlationIndex = 0;
    if(assetNb > 0)
    {
        for (auto jsonAsset : jsonAssets) 
        {
            int currencyIndex = assetNb;
            double spotCurrency = 1;
            std::string assetCurrencyId = jsonAsset.at("CurrencyId").get<std::string>();
            double realVolatility = jsonAsset.at("Volatility").get<double>();
            PnlVect* volatilityVector_ = pnl_vect_create(correlation->n);
            pnl_mat_get_row(volatilityVector_, correlation, correlationIndex);
            pnl_vect_mult_scalar(volatilityVector_, realVolatility);
            PnlVect* volatilityCurrencyVector_ = pnl_vect_create(correlation->n);
            for (auto jsonCurrency : jsonCurrencies)
            {
                std::string currencyId = jsonCurrency.at("id").get<std::string>();
                if (currencyId != domesticCurrencyId) 
                {
                    if (assetCurrencyId == currencyId) 
                    {
                        spotCurrency = jsonCurrency.at("Spot").get<double>();
                        double realCurrencyVolatility = jsonCurrency.at("Volatility").get<double>();
                        pnl_mat_get_row(volatilityCurrencyVector_, correlation, currencyIndex);
                        pnl_vect_mult_scalar(volatilityCurrencyVector_, realCurrencyVolatility);
                    }
                    currencyIndex++;
                }
            }
            pnl_vect_set(spot, correlationIndex, spotCurrency * jsonAsset.at("Spot").get<double>());
            pnl_vect_plus_vect(volatilityVector_, volatilityCurrencyVector_);
            Asset * asset = new Asset(domesticInterestRate, volatilityVector_, domesticInterestRate);
            assets.push_back(asset);
            correlationIndex++;
        }
    }
    
    

    //Cr�ation du vecteur des currencies
    vector<Currency *> currencies = vector<Currency*>();
    for (auto jsonCurrency : jsonCurrencies)
    {
        std::string currencyId(jsonCurrency.at("id").get<std::string>());
        if (currencyId != domesticCurrencyId)
        {
            double rf = jsonCurrency.at("InterestRate").get<double>();
            double realVolatility = jsonCurrency.at("Volatility").get<double>();
            PnlVect* volatilityVector_ = pnl_vect_create(correlation->n);
            pnl_mat_get_row(volatilityVector_, correlation, correlationIndex);
            pnl_vect_set(spot, correlationIndex, jsonCurrency.at("Spot").get<double>());
            correlationIndex++;
            pnl_vect_mult_scalar(volatilityVector_, realVolatility);
            Currency* currency = new Currency(domesticInterestRate - rf, volatilityVector_, rf, domesticInterestRate);
            currencies.push_back(currency);
        }
    }
    

    BlackScholesModel* newModel = new BlackScholesModel(assets, currencies, spot);

    std::string optionLabel = jsonParams.at("Option").at("Type").get<std::string>();

    PnlRng* rng = pnl_rng_create(0);
    double prix;
    double std_dev;
    double strike;
    double fdStep = jsonParams.at("RelativeFiniteDifferenceStep").get<double>();;
    long nbSamples = jsonParams.at("SampleNb").get<long>();
    PnlVect* delta = pnl_vect_create(correlation->n);
    PnlVect* std_dev_delta = pnl_vect_create(correlation->n);
    std::string type = jsonParams.at("Option").at("FixingDatesInDays").at("Type").get<std::string>();;
    PnlVect* dates = convertDatesToList(type, jsonParams);

    PnlVect * datesWithoutSpot = pnl_vect_create(dates->size - 1);
    for(int i = 1; i < dates->size; i++)
    {
        pnl_vect_set(datesWithoutSpot, i - 1, pnl_vect_get(dates, i));
    }


    
    Option * option;
    if (optionLabel == "call_quanto")
    {
        strike = jsonParams.at("Option").at("Strike").get<double>();
        option = new CallQuanto(dates, strike);
    }

    else if (optionLabel == "foreign_asian")
    {
        option = new ForeignAsian(dates);
    }

    else if (optionLabel == "call_currency")
    {
        strike = jsonParams.at("Option").at("Strike").get<double>();
        double foreignRate = currencies[0]->interestRate_;
        option = new CallCurrency(dates, strike, foreignRate);
    }


    else if(optionLabel ==  "quanto_exchange")
    {
        strike = jsonParams.at("Option").at("Strike").get<double>();
        option = new QuantoExchange(dates, strike);
    }
        
    else if(optionLabel ==  "foreign_perf_basket")
    {
        std::vector<int> assetCurrencyMapping = getAssetCurrencyMapping(jsonParams);
        strike = jsonParams.at("Option").at("Strike").get<double>();
        option = new ForeignPerfBasket(datesWithoutSpot, strike, assetCurrencyMapping);
    }
        
    
    
    MonteCarlo * pricer = new MonteCarlo(newModel, option, rng, fdStep, nbSamples);
    pricer->price(prix, std_dev);

    cout << "Le prix est: " << prix << " et l'écart-type est: " << std_dev << endl;
    pricer->delta(delta, std_dev_delta);
    cout << "Composition Delta : ";
    for (int i = 0; i < delta->size; i++)
    {
        cout << pnl_vect_get(delta, i) << ", ";
    }
    cout << endl;
    cout << "Ecart-type Deltas : ";
    for (int i = 0; i < delta->size; i++)
    {
        cout << pnl_vect_get(std_dev_delta, i) << ", ";
    }


    std::vector<int> assetCurrencyMapping = getAssetCurrencyMapping(jsonParams);


    PnlMat * marketData = pnl_mat_create_from_file(argv[2]);
    convertToDomestic(marketData, assetCurrencyMapping);
   
    
    double error;

    std::vector<Position> positions = pricer->portfolioValues(marketData, error);

        

    nlohmann::json jsonPortfolio = positions;
    std::ofstream ifout(argv[3], std::ios_base::out);
    if (!ifout.is_open()) {
        std::cout << "Unable to open file " << argv[3] << std::endl;
        std::exit(1);
    }
    ifout << jsonPortfolio.dump(4);
    ifout.close(); // Required to make sure flush is called


    double maturity = jsonParams.at("Option").at("MaturityInDays").get<int>() / double (numberOfDaysPerYear);
    

    //pnl_mat_free(&correlation);
    std::exit(0);
}