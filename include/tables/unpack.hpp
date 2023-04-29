#pragma once
#include <eosio/eosio.hpp>

namespace avatarmk {
    struct pack_data {
        eosio::name edition;
        uint8_t pack_size;  //number of nfts in pack
        std::vector<uint8_t> rarity_distribution;
    };
    EOSIO_REFLECT(pack_data, edition, pack_size, rarity_distribution)

    struct unpack {
        uint64_t pack_asset_id;
        eosio::name owner;
        pack_data pack_data;
        std::vector<uint32_t> claimable_template_ids;
        eosio::time_point_sec inserted;
        uint64_t primary_key() const { return pack_asset_id; }
        uint64_t by_owner() const { return owner.value; }
    };
    EOSIO_REFLECT(unpack, pack_asset_id, owner, pack_data, claimable_template_ids, inserted)
    // clang-format off
    typedef eosio::multi_index<"unpack"_n, unpack,
    eosio::indexed_by<"byowner"_n, eosio::const_mem_fun<unpack, uint64_t, &unpack::by_owner>>
    >unpack_table;
    // clang-format on
}  // namespace avatarmk