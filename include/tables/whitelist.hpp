#pragma once
#include <eosio/eosio.hpp>

namespace avatarmk {
    struct whitelist {
        eosio::name account;
        uint64_t primary_key() const { return account.value; }
    };
    EOSIO_REFLECT(whitelist, account)
    // clang-format off
    typedef eosio::multi_index<"whitelist"_n, whitelist> whitelist_table;
    // clang-format on
}