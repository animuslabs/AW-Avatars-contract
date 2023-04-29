#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

namespace avatarmk {
    struct editions {
        eosio::name edition_scope;             //primary key, must be unique and function as identifier of different part groups (scope)
        eosio::asset avatar_floor_mint_price;  // min price to mint an avatar from this edition
        eosio::asset avatar_template_price;
        uint64_t avatar_template_count;
        std::vector<std::vector<uint32_t>> part_template_ids = {{}, {}, {}, {}, {}};  //index matches rarityscore -1

        uint64_t primary_key() const { return edition_scope.value; }
    };
    EOSIO_REFLECT(editions, edition_scope, avatar_floor_mint_price, avatar_template_price, avatar_template_count, part_template_ids)
    // clang-format off
    typedef eosio::multi_index<"editions"_n, editions >editions_table;
    // clang-format on
}