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




#endif