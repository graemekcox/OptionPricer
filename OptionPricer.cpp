// OptionPricer.cpp : Defines the entry point for the application.
//

#include "OptionPricer.h"

#include <iostream>
#include <vector>
#include <string>

#include <cpprest/http_client.h>

#include <cpprest/http_listener.h>
#include <cpprest/filestream.h>

#if defined(_WIN32) && !defined(__cplusplus_winrt)
// Extra includes for Windows desktop.
#include <windows.h>

#include <Shellapi.h>
#endif
using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams
using namespace web::http::oauth2::experimental;
using namespace web::http::experimental::listener;




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


void auth_account(std::string token) 
{

    auto fileStream = std::make_shared<ostream>();

    pplx::task<void> requestTask = fstream::open_ostream(U("results.html")).then([=](ostream outFile)
        {
            *fileStream = outFile;

            // Create http_client to send the request.
            // Build request URI and start the request.
            //uri_builder builder(U("/stats/"));
            //builder.append_query(U("stats"), U("careerRegularSeason"));
            //return client.request(methods::GET, builder.to_string());
            http_client client(U("https://login.questrade.com/oauth2/"));

            uri_builder builder(U("token"));
            builder.append_query(U("grant_type"), U("refresh_token"));
            builder.append_query(U("refresh_token"), U("TZ4dVphsoYefG2-AAYq2_ouQ_KuIgfEa0"));
            std::wcout << builder.to_string() << std::endl;

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
            //const std::wstring & v = previousTask.get();
            //std::wcout << v << std::endl;
            //utility::string_t strinval = v.serialize();
            try
            {
                std::cout << "\nparse json data here!";
                const json::value& v = previousTask.get();

                utility::string_t jsonval = v.serialize();

                std::wcout << jsonval;
                /*
                auto array = v.at(U("row")).as_array();
                for (int i = 0; i < array.size(); i++) {
                    auto id = array[i].at(U("id")).as_string();
                    std::wcout << "\n" << id;
                    auto key = array[i].at(U("key")).as_string();
                    std::wcout << "\n" << key;
                    auto array2 = array[i].at(U("value")).as_array();

                    std::wcout << array2[0];
                    std::wcout << array2[1];
                }*/
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

//
// A simple listener class to capture OAuth 2.0 HTTP redirect to localhost.
// The listener captures redirected URI and obtains the token.
// This type of listener can be implemented in the back-end to capture and store tokens.
//
//class oauth2_code_listener
//{
//public:
//    oauth2_code_listener(uri listen_uri, oauth2_config& config)
//        : m_listener(new http_listener(listen_uri)), m_config(config)
//    {
//        m_listener->support([this](http::http_request request) -> void {
//            if (request.request_uri().path() == U("/") && !request.request_uri().query().empty())
//            {
//                m_resplock.lock();
//
//                m_config.token_from_redirected_uri(request.request_uri())
//                    .then([this, request](pplx::task<void> token_task) -> void {
//                    try
//                    {
//                        token_task.wait();
//                        m_tce.set(true);
//                    }
//                    catch (const oauth2_exception& e)
//                    {
//                        ucout << "Error: " << e.what() << std::endl;
//                        m_tce.set(false);
//                    }
//                        });
//
//                request.reply(status_codes::OK, U("Ok."));
//
//                m_resplock.unlock();
//            }
//            else
//            {
//                request.reply(status_codes::NotFound, U("Not found."));
//            }
//            });
//
//        m_listener->open().wait();
//    }
//
//    ~oauth2_code_listener() { m_listener->close().wait(); }
//
//    pplx::task<bool> listen_for_code() { return pplx::create_task(m_tce); }
//
//private:
//    std::unique_ptr<http_listener> m_listener;
//    pplx::task_completion_event<bool> m_tce;
//    oauth2_config& m_config;
//    std::mutex m_resplock;
//};
////
//// Utility method to open browser on Windows, OS X and Linux systems.
////
//static void open_browser(utility::string_t auth_uri)
//{
//#if defined(_WIN32) && !defined(__cplusplus_winrt)
//    // NOTE: Windows desktop only.
//    //auto r = window.open(ur)
//    auto r = ShellExecuteA(NULL, "open", conversions::utf16_to_utf8(auth_uri).c_str(), NULL, NULL, SW_SHOWNORMAL);
//#elif defined(__APPLE__)
//    // NOTE: OS X only.
//    string_t browser_cmd(U("open \"") + auth_uri + U("\""));
//    (void)system(browser_cmd.c_str());
//#else
//    // NOTE: Linux/X11 only.
//    string_t browser_cmd(U("xdg-open \"") + auth_uri + U("\""));
//    (void)system(browser_cmd.c_str());
//#endif
//}

//
// Base class for OAuth 2.0 sessions of this sample.
//
//class oauth2_session_sample
//{
//public:
//    oauth2_session_sample(utility::string_t name,
//        utility::string_t client_key,
//        utility::string_t client_secret,
//        utility::string_t auth_endpoint,
//        utility::string_t token_endpoint,
//        utility::string_t redirect_uri)
//        : m_oauth2_config(client_key, client_secret, auth_endpoint, token_endpoint, redirect_uri)
//        , m_name(name)
//        , m_listener(new oauth2_code_listener(redirect_uri, m_oauth2_config))
//    {
//    }
//
//    void run()
//    {
//        if (is_enabled())
//        {
//            ucout << "Running " << m_name.c_str() << " session..." << std::endl;
//
//            if (!m_oauth2_config.token().is_valid_access_token())
//            {
//                // FIXME Manually creating token from manually generate access_token. Should use the actual authorization flow
//                oauth2_token token;
//                token.set_access_token(U("SxRGokq6WQnjEjHn8lcBO4aGpx1syjes0"));
//                m_oauth2_config.set_token(token);
//                //m_oauth2_config.set_implicit_grant(false); // set response_type == code
//                m_http_config.set_oauth2(m_oauth2_config);
//                //if (authorization_code_flow().get())
//                //{
//                //    m_http_config.set_oauth2(m_oauth2_config);
//                //}
//                //else
//                //{
//                //    ucout << "Authorization failed for " << m_name.c_str() << "." << std::endl;
//                //}
//            }
//
//            run_internal();
//        }
//        else
//        {
//            ucout << "Skipped " << m_name.c_str()
//                << " session sample because app key or secret is empty. Please see instructions." << std::endl;
//        }
//    }
//
//protected:
//    virtual void run_internal() = 0;
//
//    pplx::task<bool> authorization_code_flow()
//    {
//        open_browser_auth();
//        return m_listener->listen_for_code();
//    }
//
//    http_client_config m_http_config;
//    oauth2_config m_oauth2_config;
//
//private:
//    bool is_enabled() const
//    {
//        return !m_oauth2_config.client_key().empty(); // Secret is optional
//        //return !m_oauth2_config.client_key().empty() && !m_oauth2_config.client_secret().empty();
//    }
//
//    void open_browser_auth()
//    {
//        auto auth_uri(m_oauth2_config.build_authorization_uri(true));
//        ucout << "Opening browser in URI:" << std::endl;
//        ucout << auth_uri << std::endl;
//        open_browser(auth_uri);
//    }
//
//    utility::string_t m_name;
//    std::unique_ptr<oauth2_code_listener> m_listener;
//};
//
//class qtrade_session_sample : public oauth2_session_sample
//{
//public:
//    qtrade_session_sample()
//        : oauth2_session_sample(U("Questrade"),
//            s_qtrade_key,
//            s_qtrade_secret,
//            U("https://login.questrade.com/oauth2/authorize"),
//            U("https://login.questrade.com/oauth2/token"),
//            U("http://localhost:8888/"))
//    {
//        // Dropbox uses "default" OAuth 2.0 settings.
//    }
//
//protected:
//    void run_internal() override
//    {
//        //http_client api(U("https://api.dropbox.com/1/"), m_http_config); // FIXME use Qtrade api
//        http_client api(U("https://api03.iq.questrade.com/v1/accounts"), m_http_config);
//
//        ucout << "Requesting account information:" << std::endl;
//        ucout << "Information: " << api.request(methods::GET).get().extract_json().get()
//            << std::endl;
//    }
//};


static const utility::string_t s_qtrade_key     = U("");
static const utility::string_t s_qtrade_secret  = U("");
static const utility::string_t s_auth_endpoint  = U("https://login.questrade.com/oauth2/authorize");
static const utility::string_t s_token_endpoint = U("https://login.questrade.com/oauth2/token");
static const utility::string_t s_redirect_uri   = U("http://localhost:8888/");

static const std::string api_server = "https://api01.iq.questrade.com/"; // FIXME This should be returned in our auth flow

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

// TODO move to separate header
struct Position {
    utility::string_t symbol;
    int symbol_id=0;
    double open_quantity=0;
    double cur_market_val=0;
    double cur_price=0;
    double avg_entry_price=0;
    double total_cost=0; // Total cost of position
};

utility::string_t get_userid() // Return user id
{
    const std::string api_url = api_server + "v1/accounts/";
    http_client api(uri(utility::conversions::to_string_t(api_url)), m_http_config);

    ucout << "Requesting account information:" << std::endl;

    const json::value & ret_json = api.request(methods::GET).get().extract_json().get();
    ucout << "Information: " << ret_json
        << std::endl;

    //auto array = ret_json.at(U("userId")).as_array();

    //std::cout << "\nparse json data here!";
    //const json::value& v = previousTask.get();

    //utility::string_t jsonval = v.serialize();

    //std::wcout << jsonval;

    //auto array = v.at(U("row")).as_array();
    //for (int i = 0; i < array.size(); i++) {
    //    auto id = array[i].at(U("id")).as_string();
    //    std::wcout << "\n" << id;
    //    auto key = array[i].at(U("key")).as_string();
    //    std::wcout << "\n" << key;
    //    auto array2 = array[i].at(U("value")).as_array();

    //    std::wcout << array2[0];
    //    std::wcout << array2[1];
    //}

    utility::string_t account_num = U("-1");
    if (ret_json.has_field(U("accounts"))) {
        auto accounts = ret_json.at(U("accounts")).as_array();
        for (int i = 0; i < accounts.size(); i++) {
            account_num= accounts[i].at(U("number")).as_string();
            // TODO maybe add some checking for account status? or account type?
            std::wcout << account_num<< std::endl;
        }
        //id = ret_json.at(U("userId")).as_integer();
        ucout << "UserId = " << account_num<< std::endl;
    }
    //if (ret_json.has_field(U("userId"))) {
    //    id = ret_json.at(U("userId")).as_integer();
    //    std::wcout << "UserId = " << id << std::endl;
    //}
    return account_num;
}

void get_ticker() // Return user id
{

    http_client api(U("https://api05.iq.questrade.com/v1/symbols/8049"), m_http_config);

    ucout << "Requesting AAPL ticker:" << std::endl;

    const json::value& ret_json = api.request(methods::GET).get().extract_json().get();
    //ucout << "Information: " << ret_json
        //<< std::endl;

    if (ret_json.has_field(U("symbols"))) {
        auto symbols = ret_json.at(U("symbols")).as_array();

        ucout << "Num of tickers returned =" << symbols.size() << std::endl;

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

int main(int argc, char* argv[])
{
    std::cout << "Running OAuth 2.0 client sample" << std::endl;    

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

    //qtrade_session_sample qtrade;
    //qtrade.run();

    utility::string_t userid = get_userid();
    
    std::vector<Position> positions;
    std::vector<Symbol> valid_symbols;
    get_current_pos(userid, positions);


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
        //fetch_option_data(valid_symbols[i].symbol_id);
    }

    std::cout << "Done" << std::endl;
    return 0;
}
// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
