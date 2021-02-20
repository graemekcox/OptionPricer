// OptionPricer.cpp : Defines the entry point for the application.
//

#include "OptionPricer.h"

#include <any>


void make_req()
{
    auto fileStream = std::make_shared<ostream>();

    // Open stream to output file.
    pplx::task<void> requestTask = fstream::open_ostream(U("results.html")).then([=](ostream outFile)
        {
            *fileStream = outFile;

            // Create http_client to send the request.
            http_client client(U("https://api03.iq.questrade.com/v1/accounts"));

            // Build request URI and start the request.
            //uri_builder builder(U("/stats/"));
            //builder.append_query(U("stats"), U("careerRegularSeason"));
            //return client.request(methods::GET, builder.to_string());

            return client.request(methods::GET);
        })

        // Handle response headers arriving.
            .then([=](http_response response)
                {
                    printf("Received response status code:%u\n", response.status_code());

                    // Write response body into the file.
                    if (response.status_code() == status_codes::OK) {
                        std::cout << "status_ok";
                        return response.extract_json();
                    }
                    //Error cases. For now just return empty json

                    std::cout << "\n get json data";
                    return pplx::task_from_result(json::value());
                })
            .then([=](pplx::task<json::value> previousTask)
                {
                    try
                    {
                        std::cout << "\nparse json data here!";
                        const json::value& v = previousTask.get();

                        utility::string_t jsonval = v.serialize();

                        std::wcout << jsonval;

                        auto array = v.at(U("row")).as_array();
                        for (int i = 0; i < array.size(); i++) {
                            auto id = array[i].at(U("id")).as_string();
                            std::wcout << "\n" << id;
                            auto key = array[i].at(U("key")).as_string();
                            std::wcout << "\n" << key;
                            auto array2 = array[i].at(U("value")).as_array();

                            std::wcout << array2[0];
                            std::wcout << array2[1];
                        }
                    }
                    catch (const http_exception& e) {
                        std::cout << "\nError";
                    }

                    fileStream->close();
                });

                // Wait for all the outstanding I/O to complete and handle any exceptions
                try
                {
                    requestTask.wait();
                }
                catch (const std::exception& e)
                {
                    printf("Error exception:%s\n", e.what());
                }

}

static const utility::string_t s_qtrade_key     = U("");
static const utility::string_t s_qtrade_secret  = U("");
static const utility::string_t s_auth_endpoint  = U("https://login.questrade.com/oauth2/authorize");
static const utility::string_t s_token_endpoint = U("https://login.questrade.com/oauth2/token");
static const utility::string_t s_redirect_uri   = U("http://localhost:8888/");
               std::string     expiry_date      = "2021-02-26T00:00:00.000000-05:00"; // Array of options is stored by expiry date


static const std::string api_server = "https://api07.iq.questrade.com/"; // FIXME This should be returned in our auth flow

http_client_config m_http_config;
oauth2_config m_oauth2_config(s_qtrade_key, s_qtrade_secret, s_auth_endpoint, s_token_endpoint, s_redirect_uri);

class Symbol {
public:
    utility::string_t symbol_name;
    int symbol_id; // Used for fetching from API
    double prev_close; //Previous day's closing price
    utility::string_t description;
    bool has_options;
    utility::string_t currency;

    Symbol(int id) {
        this->symbol_id = id;
        fetch_ticker();
    }

    void fetch_ticker() { // Make API request
        // TODO API server should be used instead of hardcoding this string.The 'api0#' changes
        const std::string api_url = api_server + "v1/symbols/" + std::to_string(this->symbol_id);
        http_client api(uri(utility::conversions::to_string_t(api_url)), m_http_config);

        const json::value& ret_json = api.request(methods::GET).get().extract_json().get();
        //ucout << ret_json << std::endl;

        if (ret_json.has_field(U("symbols"))) {
            auto symbols = ret_json.at(U("symbols")).as_array();

            ucout << "Num of tickers returned =" << symbols.size() << std::endl;

            auto ticker = symbols[0];

            this->prev_close    = ticker.at(U("prevDayClosePrice")).as_double();
            this->symbol_name   = ticker.at(U("symbol")).as_string();
            this->currency      = ticker.at(U("currency")).as_string();
            this->has_options   = ticker.at(U("hasOptions")).as_bool();
            this->description   = ticker.at(U("description")).as_string();
            
            // Print all the info we grabbed
            //print_info();
        }
        else {
            ucout << "ERROR: This fetch_ticker error case has not been handled yet" << std::endl;
            ucout << ret_json << std::endl;
        }

    }

    void print_info() 
    {
        ucout << this->symbol_name << " Closing Price = $" << this->prev_close << " " << this->currency << std::endl;
        ucout << "Description: " << this->description << std::endl;

        if (this->has_options) {
            ucout << this->symbol_name<< " has options!" << std::endl;
        }
    }
};


/*
    Function:
        get_userid
    Return:
        string_t : Currently logged in account user id 
    Description:
        Request account info information and return user id
*/
utility::string_t get_userid() // Return user id
{
    const std::string api_url = api_server + "v1/accounts/";
    http_client api(uri(utility::conversions::to_string_t(api_url)), m_http_config);

    //ucout << "Requesting account information:" << std::endl;

    const json::value & ret_json = api.request(methods::GET).get().extract_json().get();

    utility::string_t account_num = U("-1");
    if (ret_json.has_field(U("accounts"))) {
        auto accounts = ret_json.at(U("accounts")).as_array();
        for (int i = 0; i < accounts.size(); i++) {
            account_num= accounts[i].at(U("number")).as_string();
            // TODO maybe add some checking for account status? or account type?
        }
    }

    return account_num;
}

void get_ticker()
{

    http_client api(U("https://api05.iq.questrade.com/v1/symbols/8049"), m_http_config);

    ucout << "Requesting AAPL ticker:" << std::endl;

    const json::value& ret_json = api.request(methods::GET).get().extract_json().get();
    //ucout << "Information: " << ret_json
        //<< std::endl;

    if (ret_json.has_field(U("symbols"))) {
        auto symbols = ret_json.at(U("symbols")).as_array();

        //ucout << "Num of tickers returned =" << symbols.size() << std::endl;
        auto ticker = symbols[0];

        double closePrice = ticker.at(U("prevDayClosePrice")).as_double();
        utility::string_t sym_name = ticker.at(U("symbol")).as_string();
        utility::string_t currency = ticker.at(U("currency")).as_string();
        bool has_options = ticker.at(U("hasOptions")).as_bool();

        ucout << sym_name << " Closing Price = $" << closePrice << " " << currency << std::endl;
        ucout << "Description: " << ticker.at(U("description")).as_string() << std::endl;
        
        if (has_options) {
            ucout << sym_name << " has options!" << std::endl;
        }
        //ucout << ticker << std::endl;
    }
    else {
        ucout << "ERROR FETCHING TICKER" << std::endl;
    }
}

std::tm convert_to_time(const std::string& dt) {
    std::tm date;
    // FIXME Don't think hours,min and sec are saved properly
    sscanf(dt.c_str(), "%4d-%2d-%02dT%02d:%2d:%2d",
        &date.tm_year, &date.tm_mon, &date.tm_mday, &date.tm_hour, &date.tm_min, &date.tm_sec);

    //printf("Time is %0d %0d %0d %02:%02:%02", date.tm_year, date.tm_mon, date.tm_mday, date.tm_hour, date.tm_min, date.tm_sec);

    return date;
}

/*
    Function: 
        get_option_data
    Parameters:
        int id = API Symbol Identifer. Ex: AAPL = 8049
        optionmap opt_map = Map to store option chain data we'll use to make requests later
        int num_expiry = Number of options expiration dates to loop through. Defaults to 1, which is weekly options;
        int num_option_chain = Number of strike prices to loop through. Defaults to 10, with index=0 being the most ITM call option
*/
void get_option_data(int id, optionmap& opt_map, int num_expiry=1, int num_option_chain=10)
{
    const std::string api_url = api_server + "v1/symbols/" + utility::conversions::to_utf8string(std::to_string(id)) +"/options";
    http_client api(uri(utility::conversions::to_string_t(api_url)), m_http_config);

    const json::value& ret_json = api.request(methods::GET).get().extract_json().get();
    //ucout << "Information: " << ret_json << std::endl;
    //ucout << utility::conversions::to_string_t(api_url) << std::endl;

    if (ret_json.has_field(U("optionChain"))) {
        auto options = ret_json.at(U("optionChain")).as_array();
     
        for (int exp_cnt = 0; exp_cnt < num_expiry; exp_cnt++) {
            auto opt = options[exp_cnt];

            utility::string_t desc = opt.at(U("description")).as_string();
            //expiry_date = utility::conversions::to_utf8string(opt.at(U("expiryDate")).as_string());
            //std::cout << "Grab options chain for expiry = " << expiry_date << std::endl;

            auto chain_per_root = opt.at(U("chainPerRoot")).as_array();
            auto cpr = chain_per_root[0]; // TODO check when this type has more than 1 chain_per_root
            auto chain_per_strike = cpr.at(U("chainPerStrikePrice")).as_array();

            for (int i = 0; i < num_option_chain; i++) { // FIXME Temp constraint to keep option_chain small 
                Option option;
                // TODO Maybe start at index = current stock price. Then just look at OTM calls?
                auto cps = chain_per_strike[i];
                //ucout << desc << " Strike=" << cps.at(U("strikePrice")).as_double() << " Call_id = " << cps.at(U("callSymbolId")).as_integer() << std::endl;

                option.strike = cps.at(U("strikePrice")).as_double();
                option.id = cps.at(U("callSymbolId")).as_integer(); // FIXME Hardcode to only look at calls for now
                option.expiry = expiry_date;

                opt_map[expiry_date].push_back(option);
            }
        }
    }
    else {
        ucout << "Error finding 'optionChain' key" << std::endl;
    }
}

/*
    Function:
        get_option_market_quotes
    Parameters:
        optionmap opt_map = Map containing store option chain data such as option IDs. We will update the options info stored here
    Description:
        API requires us to make requests with specific option IDs to pull specific greeks. Use list of option IDs to update
        option chain data stored in opt_data with any specific information.
        Currently mainly focusing on greeks, but can also grab bid/ask, volumes, etc

        API found at: https://www.questrade.com/api/documentation/rest-operations/market-calls/markets-quotes-options

*/
void get_option_market_quotes(optionmap& opt_map)
{
    //const std::string api_url = api_server + "v1/symbols/" + utility::conversions::to_utf8string(std::to_string(id)) + "/options";
    const std::string api_url = api_server + "v1/markets/quotes/options";

    
    std::vector<Option> temp_array = opt_map[expiry_date];
    std::vector<int> temp_ids;
    for (const auto& ind : temp_array)
        temp_ids.push_back(ind.id);

    web::json::value postData = web::json::value::object();

    // FIXME Trying to pass filters leads to "Invalid or malformed argument 
    //postData[U("filters")] = json::value::object();
    //postData[U("filters")][U("underlyingId")] = json::value::number(8049);
    //postData[U("filters")][U("expiryDate")] = json::value::string(U("2021 - 02 - 05T00:00 : 00.000000 - 05 : 00"));
    //postData[U("filters")][U("minStrikePrice")] = json::value::number(100);
    //postData[U("filters")][U("optionType")] = json::value::string(U("Call"));

    postData[U("optionIds")] = json::value::array();
    for (int i = 0; i < temp_ids.size(); i++)
        postData[U("optionIds")][i] = temp_ids[i];

    http_client api(uri(utility::conversions::to_string_t(api_url)), m_http_config);

    web::http::http_request postReq(methods::POST);
    postReq.set_body(postData.to_string().c_str());

    const json::value& ret_json = api.request(postReq).get().extract_json().get();
    //ucout << "Information: " << ret_json << std::endl;

    // Parse response data, and update our option_map with the quote data.
    if (ret_json.has_field(U("optionQuotes"))) {
        auto options = ret_json.at(U("optionQuotes")).as_array();

        // TODO I'm assuming the optionQuotes are returned in the same order as the ID's order in the body
        for (int i = 0; i < options.size(); i++) {
            auto opt = options[i];

            opt_map[expiry_date][i].symbol = utility::conversions::to_utf8string(opt.at(U("underlying")).as_string());
            opt_map[expiry_date][i].volatility = opt.at(U("volatility")).as_double();
            opt_map[expiry_date][i].delta = opt.at(U("delta")).as_double();
            opt_map[expiry_date][i].gamma = opt.at(U("gamma")).as_double();
            opt_map[expiry_date][i].theta = opt.at(U("theta")).as_double();
            opt_map[expiry_date][i].vega = opt.at(U("vega")).as_double();
            opt_map[expiry_date][i].rho = opt.at(U("rho")).as_double();
        }
    }
}

/*
    Function:
        clean_opt_map
    Parameter:
        optionmap opt_map = Map containing option chain info to be cleaned
    Description:
        Currently removing all options with a IV ~= 0

    Todo:
        - Remove options with gamma < 0.3 ?
        - 
*/
void clean_opt_map(optionmap& opt_map, int debug = 0) {
    int opt_size = opt_map[expiry_date].size();
    int i = 0;
    int num_removed = 0;

    while (i < opt_size) {
        if (opt_map[expiry_date][i].volatility == 0) {
            if (debug) std::cout << "Volatility at index = " << i << " is close to 0!" << std::endl;
            opt_map[expiry_date].erase(opt_map[expiry_date].begin() + i - num_removed);
            opt_size--;
        }
        else {
            if (debug) std::cout << "Volatility at index = " << i << " is " << opt_map[expiry_date][i].volatility << std::endl;
            i++;
        }
    }
}


void get_ticker_data(int id) {

    const std::string api_url = api_server + "v1/symbols/" + utility::conversions::to_utf8string(std::to_string(id));
    http_client api(uri(utility::conversions::to_string_t(api_url)), m_http_config);

    ucout << "Requesting Ticker Data for id:" << id << std::endl;

    const json::value& ret_json = api.request(methods::GET).get().extract_json().get();
    ucout << "Information: " << ret_json
        << std::endl;

    auto symbols = ret_json.at(U("symbols")).as_array();
    auto symbol = symbols[0];
    utility::string_t sym = symbol.at(U("symbol")).as_string();
    double high_price = symbol.at(U("highPrice52")).as_double();

    int avg_vol_3mon = symbol.at(U("averageVol3Months")).as_integer();
    int avg_vol_20day = symbol.at(U("averageVol20Days")).as_integer();

    int outstanding_shares = symbol.at(U("outstandingShares")).as_integer();

    ucout << sym << " : high = " << high_price << "\nAvg Vol 3mon = " << avg_vol_3mon << " 20day = " << avg_vol_20day;
    ucout << "\n Outstanding Shares = " << outstanding_shares << std::endl;
}
void get_current_pos(utility::string_t id, std::vector<Position> & positions) 
{
    const std::string api_url = api_server + "v1/accounts/" + utility::conversions::to_utf8string(id) + "/positions";
    http_client api(uri(utility::conversions::to_string_t(api_url)), m_http_config);

    ucout << "Requesting account positions:" << std::endl;
    const json::value& ret_json = api.request(methods::GET).get().extract_json().get();
    //ucout << "Information: " << ret_json
    //    << std::endl;

    //std::vector<Position> positions;
    if (ret_json.has_field(U("positions"))) {
        auto ret_pos = ret_json.at(U("positions")).as_array();

        for (int i = 0; i < ret_pos.size(); i++) {
            Position cur_symbol;
   
            cur_symbol.symbol           = ret_pos[i].at(U("symbol")).as_string();
            cur_symbol.symbol_id        = ret_pos[i].at(U("symbolId")).as_integer();
            cur_symbol.open_quantity    = ret_pos[i].at(U("openQuantity")).as_double();
            cur_symbol.cur_market_val   = ret_pos[i].at(U("currentMarketValue")).as_double();
            cur_symbol.cur_price        = ret_pos[i].at(U("currentPrice")).as_double();
            cur_symbol.avg_entry_price  = ret_pos[i].at(U("averageEntryPrice")).as_double();
            cur_symbol.total_cost       = ret_pos[i].at(U("totalCost")).as_double();
            positions.push_back(cur_symbol);
            //ucout << "Just grabbed " << cur_symbol.symbol << std::endl;
        }
    }
    else {
        std::cout << "ERROR: Positions could not be fetched" << std::endl;
        return; // TODO Throw error here
    }
}

void get_high_iv_calls(std::vector<Position> positions,  double min_iv = 0.0) {
    optionmap opt_map;
    //for (int i = 0; i < positions.size(); i++) {
    for (const Position pos: positions) {
        //ucout << "Grabbed ticker = " << positions[i].symbol << " Currently have " << positions[i].open_quantity << " shares wor$th a total of $" << positions[i].cur_market_val << std::endl;
        //if (positions[i].open_quantity >= 100) { // We'll grab options data only for tickers where we can write covered calls
        ucout << "Grabbed ticker = " << pos.symbol << " Currently have " << pos.open_quantity << " shares worth a total of $" 
            << pos.cur_market_val << std::endl;
        
        get_option_data(pos.symbol_id, opt_map, 1, 10);
    };


    get_option_market_quotes(opt_map);
    clean_opt_map(opt_map);
}

void recommend_cc(std::vector<Position> positions) {
    optionmap opt_map;

    std::cout << " -------------- Current Positions above 100 Shares -------------" << std::endl;
     
    for (int i = 0; i < positions.size(); i++) {
        //ucout << "Grabbed ticker = " << positions[i].symbol << " Currently have " << positions[i].open_quantity << " shares wor$th a total of $" << positions[i].cur_market_val << std::endl;
        //if (positions[i].open_quantity >= 100) { // We'll grab options data only for tickers where we can write covered calls
        if (positions[i].open_quantity >= 30) {
            ucout << "Grabbed ticker = " << positions[i].symbol << " Currently have " << positions[i].open_quantity << " shares worth a total of $" << positions[i].cur_market_val << std::endl;
            get_option_data(positions[i].symbol_id, opt_map, 1, 10);
        }
    };

    if (opt_map[expiry_date].size() == 0) {
        std::cout << "No current positions have at least 100 shares. Cannot write covered calls" << std::endl;
        return;
    }

    get_option_market_quotes(opt_map);
    clean_opt_map(opt_map);
    auto options_list = opt_map[expiry_date];

    std::cout << " -------------- Filtered Option Chain -------------" << std::endl;
    for (const Option& opt : options_list) {
        std::cout <<  "Symbol=" << std::setw(5)     << opt.symbol;
        std::cout << "  Strike="  << std::setw(7)   << opt.strike;
        std::cout << "  IV=" << std::setw(7) << opt.volatility;
        std::cout << "  Delta=" << std::setw(8)      << opt.delta;
        std::cout << "  Gamma=" << std::setw(8)      << opt.gamma;
        std::cout << "  Theta=" << std::setw(8)      << opt.theta;
        //std::cout << " Vega=" << opt.vega;
        //std::cout << " Rho=" << opt.rho;
        std::cout << std::endl;
    }
}

/*
Project Ideas:
- Calculate net gains after taxes depending on exchange?
- recommend whether to use Norbert's gambit for deposits
- use Black Scholes Model to try and calculate fair price 
*/

int main(int argc, char* argv[])
{
    ucout << "Running session..." << std::endl;

    if (!m_oauth2_config.token().is_valid_access_token())
    {
        // FIXME Manually creating token from manually generate access_token. Should use the actual authorization flow
        oauth2_token token;
        token.set_access_token(U(""));
        m_oauth2_config.set_token(token);
        m_oauth2_config.set_implicit_grant(false); // set response_type == code
        m_http_config.set_oauth2(m_oauth2_config);
    }

    utility::string_t userid = get_userid();

    if (userid == U("-1")) {
        std::cout << "ERROR: User id was not properly fetched. Check access token and auth header" << std::endl;
        return 0;
    }

    // Grab accounts current positions
    std::vector<Position> positions;
    std::vector<Symbol> valid_symbols;
    get_current_pos(userid, positions);

    // TODO select function based on input arguments
    recommend_cc(positions);

    // Some samples of function usage found below...
    /*
    //get_ticker_data(19719);

    //utility::string_t userid = get_userid();
    //std::vector<Position> positions;
    //std::vector<Symbol> valid_symbols;
    //get_current_pos(userid, positions);

    for (int i = 0; i < positions.size(); i++) {
        ucout << "Grabbed ticker = " << positions[i].symbol << " Currently have " << positions[i].open_quantity << " shares worth a total of $"
            << positions[i].cur_market_val << std::endl;

        if (positions[i].open_quantity >= 100) { // We'll grab options data only for tickers where we can write covered calls
            Symbol sym(positions[i].symbol_id);

            if (sym.has_options) // Make sure that this ticker actually has options
                valid_symbols.push_back(sym);
        }
    };
   
    for (int i = 0; i < valid_symbols.size(); i++) {
        get_option_data(valid_symbols[i].symbol_id);
    }
    */

    std::cout << "Done" << std::endl;
    return 0;
}