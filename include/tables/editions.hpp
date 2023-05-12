#pragma once
#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>

namespace avatarmk {
    struct editions {
        eosio::name edition_scope;             //primary key, must be unique and function as identifier of different part groups (scope)
        eosio::asset avatar_floor_mint_price;  // min price to mint an avatar from this edition
        eosio::asset avatar_template_price;
        // float pct_increase_template_price; // increase the template creation price this percent for each rarity
        uint64_t avatar_template_count;
        uint8_t num_avatarparts = 6;
        std::vector<std::vector<uint32_t>> part_template_ids = {{}, {}, {}, {}, {}};  //index matches rarityscore -1

        uint64_t primary_key() const { return edition_scope.value; }
    };

    EOSIO_REFLECT(editions, edition_scope, avatar_floor_mint_price, avatar_template_price, avatar_template_count, num_avatarparts, part_template_ids)
    // clang-format off
    typedef eosio::multi_index<"editions"_n, editions >editions_table;
    // clang-format on
}  // namespace avatarmk
