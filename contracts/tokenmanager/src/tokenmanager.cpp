#include "../include/tokenmanager.hpp"

//======================== config actions ========================

ACTION tokenmanager::init(asset create_price)
{
    require_auth(get_self());

    config_table configs(get_self(), get_self().value);

    check(!configs.exists(), "config already initialized");

    config new_conf = {
        create_price};

    configs.set(new_conf, get_self());
}

ACTION tokenmanager::addtoken(string token_name, name token_owner, name contract_account, symbol token_symbol, string logo_sm, string logo_lg)
{
    require_auth(get_self());
    tokens_table tokens(get_self(), get_self().value);
    auto tkn = tokens.find(token_symbol.code().raw());
    check(tkn == tokens.end(), "Cannot create token, symbol already exists");
    tokens.emplace(get_self(), [&](auto &t)
                   {
                       t.token_name = token_name;
                       t.token_symbol = token_symbol;
                       t.token_owner = token_owner;
                       t.contract_account = contract_account;
                       t.logo_sm = logo_sm;
                       t.logo_lg = logo_lg;
                   });
}

ACTION tokenmanager::setmeta(symbol token_symbol, string token_name, string logo_sm, string logo_lg)
{
    tokens_table tokens(get_self(), get_self().value);
    auto tkn = tokens.find(token_symbol.code().raw());
    check(tkn != tokens.end(), "Token not found");
    require_auth(tkn->token_owner);
    tokens.modify(tkn, same_payer, [&](auto &t)
                  {
                      t.token_name = token_name;
                      t.logo_sm = logo_sm;
                      t.logo_lg = logo_lg;
                  });
}

ACTION tokenmanager::createtoken(name owner, string token_name, asset max_supply, string logo_sm, string logo_lg)
{
    require_auth(owner);
    check_token(max_supply);
    accounts_table accounts(get_self(), owner.value);
    auto &acct = accounts.get(TLOS_SYM.code().raw(), "Cannot create a token without an account balance");
    config_table configs(get_self(), get_self().value);
    check(configs.exists(), "Contract not yet initialized");
    auto conf = configs.get();
    check(acct.balance.amount >= conf.create_price.amount, "Insufficient balance");
    accounts.modify(acct, same_payer, [&](auto &a)
                    { a.balance -= conf.create_price; });

    tokens_table tokens(get_self(), get_self().value);
    auto tkn = tokens.find(max_supply.symbol.code().raw());
    check(tkn == tokens.end(), "Cannot create token, symbol already exists");
    tokens.emplace(owner, [&](auto &t)
                   {
                       t.token_name = token_name;
                       t.token_symbol = max_supply.symbol;
                       t.token_owner = owner;
                       t.contract_account = TOKEN_CONTRACT;
                       t.logo_sm = logo_sm;
                       t.logo_lg = logo_lg;
                   });

    action(permission_level{TOKEN_CONTRACT, name("create")},
           TOKEN_CONTRACT,
           name("create"),
           make_tuple(owner, max_supply))
        .send();
}

ACTION tokenmanager::openacct(name account_name)
{
    require_auth(account_name);

    accounts_table accounts(get_self(), account_name.value);
    auto acct = accounts.find(TLOS_SYM.code().raw());
    if (acct == accounts.end())
    {
        accounts.emplace(account_name, [&](auto &a)
                         { a.balance = asset(0, TLOS_SYM); });
    }
}

ACTION tokenmanager::closeacct(name account_name)
{
    require_auth(account_name);
    accounts_table accounts(get_self(), account_name.value);
    auto &acct = accounts.get(TLOS_SYM.code().raw(), "account not found");
    if (acct.balance.amount > 0)
    {
        action(permission_level{get_self(), name("active")},
               name("eosio.token"),
               name("transfer"),
               make_tuple(get_self(),
                          account_name,
                          acct.balance,
                          std::string("Close token manager account")))
            .send();
    }
    accounts.erase(acct);
}

void tokenmanager::catch_transfer(name from, name to, asset quantity, string memo)
{
    name rec = get_first_receiver();

    if (rec == name("eosio.token") && from != get_self() && to == get_self() && quantity.symbol == TLOS_SYM)
    {
        accounts_table accounts(get_self(), from.value);
        auto acct = accounts.find(TLOS_SYM.code().raw());

        check(acct != accounts.end(), "Must call openacct action first to create an account");
        accounts.modify(*acct, same_payer, [&](auto &a)
                        { a.balance += quantity; });
    }
}
