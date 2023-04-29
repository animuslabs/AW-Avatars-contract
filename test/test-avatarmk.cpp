#include <eosio/tester.hpp>
#include <token/token.hpp>

#include "avatarmk.hpp"

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

using namespace eosio;

// Setup function to install contract to the test_chain t
void setup_installMyContract(test_chain& t)
{
    t.create_code_account("avatarmk"_n);
    t.set_code("avatarmk"_n, "artifacts/avatarmk.wasm");
}

// Setup function to add some accounts to the chain
void setup_createAccounts(test_chain& t)
{
    for (auto user : {"alice"_n, "bob"_n, "jane"_n, "joe"_n}) {
        t.create_account(user);
    }
}

// Sample setup function to set up the eosio.token contract
void setup_token(test_chain& t)
{
    // This creates the eosio.token account with the following authorities:
    // * owner:
    //    * threshold = 1
    //    * public keys: [default_pub_key (EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV)]
    //    * accounts: []
    // * active:
    //    * threshold = 1
    //    * public keys: [default_pub_key]
    //    * accounts: [eosio.token@eosio.code]
    //
    // There are several overloads of create_account() and create_code_account() which cover common
    // cases plus provide full flexibility when needed.
    t.create_code_account("eosio.token"_n);

    // Set the code
    t.set_code("eosio.token"_n, CLSDK_CONTRACTS_DIR "token.wasm");

    // Create and issue tokens.
    //
    // * as("eosio.token"_n) means use eosio.token's active authority. The tester uses
    //   default_priv_key (5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3) to sign.
    // * act<...>(...) sends a single transaction and verifies it was successful
    // * token::actions::* is defined by EOSIO_ACTIONS(...) in token/token.hpp
    // * s2a (string-to-asset) is a shortcut for constructing assets

    t.as("eosio.token"_n).act<token::actions::create>("eosio"_n, s2a("1000000.0000 EOS"));
    // t.as("eosio.token"_n).act<token::actions::create>("eosio"_n, s2a("1000000.0000 OTHER"));
    t.as("eosio"_n).act<token::actions::issue>("eosio"_n, s2a("1000000.0000 EOS"), "");
    // t.as("eosio"_n).act<token::actions::issue>("eosio"_n, s2a("1000000.0000 OTHER"), "");

    // Uh-oh, someone's up to no good
    // One of the arguments to EOSIO_ACTIONS is a default contract. EOSIO_ACTIONS places the default
    // contract in the action wrappers. act<...> and trace<...> normally send the action to the
    // default. with_code() overrides that.

    t.create_code_account("hacker.token"_n);
    t.set_code("hacker.token"_n, CLSDK_CONTRACTS_DIR "token.wasm");
    t.as("hacker.token"_n).with_code("hacker.token"_n).act<token::actions::create>("hacker.token"_n, s2a("1000000.0000 EOS"));
    t.as("hacker.token"_n).with_code("hacker.token"_n).act<token::actions::issue>("hacker.token"_n, s2a("1000000.0000 EOS"), "");
}

// Sample setup function to fund some users
void setup_fundUsers(test_chain& t)
{
    for (auto user : {"alice"_n, "bob"_n, "jane"_n, "joe"_n}) {
        t.as("eosio"_n).act<token::actions::transfer>("eosio"_n, user, s2a("10000.0000 EOS"), "");
        t.as("eosio"_n).act<token::actions::transfer>("eosio"_n, user, s2a("10000.0000 OTHER"), "");
        t.as("hacker.token"_n).with_code("hacker.token"_n).act<token::actions::transfer>("hacker.token"_n, user, s2a("10000.0000 EOS"), "");
    }
}

TEST_CASE("Short description of first test", "[test]")
{
    // This starts a single-producer chain
    test_chain t;
    setup_installMyContract(t);
    setup_createAccounts(t);

    // some shortcuts
    auto alice = t.as("alice"_n);
    auto bob = t.as("bob"_n);

    // * trace<...>(...) sends a single transaction, but does not verify success. Instead it returns
    //   a transaction trace.
    // * expect(trace, msg)
    //    * if msg == nullptr, verifies the transaction was successful
    //    * if msg != nullptr, verifies the transaction failed, and the error message contains msg
    //      within it (partial match)

    // expect(alice.trace<avatarmk::actions::assemble>("alice"_n, "blabla"), nullptr);
    // alice.trace<avatarmk::actions::test>();

}  // First test
