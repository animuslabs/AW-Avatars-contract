#pragma once
#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>

namespace avatarmk {
    //scoped by edition
    struct packs {
        uint64_t template_id;
        eosio::asset base_price;
        eosio::asset floor_price;
        eosio::time_point_sec last_sold;
        std::string pack_name;
        // std::vector<uint8_t> rarity_distribution;  // TODO can be removed, reading from NFT template now
        uint64_t primary_key() const { return template_id; }
    };
    EOSIO_REFLECT(packs, template_id, base_price, floor_price, last_sold, pack_name)
    // clang-format off
    typedef eosio::multi_index<"packs"_n, packs >packs_table;
    // clang-format on
}  // namespace avatarmk
