// Example contract that can create, update, and delete tasks.
//
// @author Awesome Developer Person
// @contract tokenmanager
// @version v1.1.0

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>
#include <eosio/action.hpp>

using namespace std;
using namespace eosio;

static constexpr eosio::name TOKEN_CONTRACT = name("testtoken321");

CONTRACT tokenmanager : public contract
{
public:
    tokenmanager(name self, name code, datastream<const char *> ds) : contract(self, code, ds){};
    ~tokenmanager(){};

    const symbol TLOS_SYM = symbol("TLOS", 4);

    ACTION init(asset create_price);

    ACTION openacct(name account_name);

    ACTION closeacct(name account_name);

    ACTION addtoken(string token_name, name token_owner, name contract_account, symbol token_symbol, string logo_sm, string logo_lg);

    ACTION setmeta(symbol token_symbol, string token_name, string logo_sm, string logo_lg);

    ACTION createtoken(name owner, string token_name, asset max_supply, string logo_sm, string logo_lg);

    void check_token(asset token) {
        auto sym = token.symbol;
        check( sym.is_valid(), "invalid symbol name" );
        check( token.is_valid(), "invalid supply");
        check( token.amount > 0, "max-supply must be positive");    
    }

    [[eosio::on_notify("eosio.token::transfer")]] void catch_transfer(name from, name to, asset quantity, string memo);

    TABLE config
    {
        asset create_price;

        EOSLIB_SERIALIZE(config, (create_price))
    };
    typedef singleton<name("config"), config> config_table;

    TABLE token
    {
        name contract_account;
        name token_owner;
        string token_name;
        string logo_sm;
        string logo_lg;
        symbol token_symbol;
        optional<map<string, string>> meta;

        uint64_t primary_key() const { return token_symbol.code().raw(); }
        EOSLIB_SERIALIZE(token, (contract_account)(token_owner)(token_name)(logo_sm)(logo_lg)(token_symbol)(meta))
    };
    typedef multi_index<name("tokens"), token> tokens_table;

    TABLE account
    {
        asset balance;

        uint64_t primary_key() const { return balance.symbol.code().raw(); }

        EOSLIB_SERIALIZE(account, (balance))
    };
    typedef multi_index<name("accounts"), account> accounts_table;
};