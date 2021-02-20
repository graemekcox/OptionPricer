#ifndef _STOCK_TYPES_H
#define _STOCK_TYPES_H

struct Position {
    utility::string_t symbol;
    int symbol_id = 0;
    double open_quantity = 0;
    double cur_market_val = 0;
    double cur_price = 0;
    double avg_entry_price = 0;
    double total_cost = 0; // Total cost of position
};

struct Option {
    std::string symbol;
    int id;
    double strike;
    std::string expiry;
    double volatility;
    double delta;
    double gamma;
    double theta;
    double vega;
    double rho;
};


// Store options chains by expiration date
/*
    OptionMap  std::map< expiry, array of options>
        |
        --> Feb 12th
            [0] --> {strike, rho, id}
            [1] --> {strike, rho, id}
        --> Feb 19th
            [0] --> {strike, rho, id}
            [1] --> {strike, rho, id}
        .....
*/
typedef std::map<std::string, std::vector<Option>> optionmap;


#endif