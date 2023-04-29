#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

namespace avatarmk {
    struct deposits {
        uint64_t id;
        eosio::extended_asset balance;
        uint64_t primary_key() const { return id; }
        uint128_t by_contr_sym() const { return (uint128_t{balance.contract.value} << 64) | balance.quantity.symbol.raw(); }
    };
    EOSIO_REFLECT(deposits, id, balance)
    // clang-format off
    typedef eosio::multi_index<"deposits"_n, deposits, 
    eosio::indexed_by<"bycontrsym"_n, eosio::const_mem_fun<deposits, uint128_t, &deposits::by_contr_sym>>
    > deposits_table;
    // clang-format on
}