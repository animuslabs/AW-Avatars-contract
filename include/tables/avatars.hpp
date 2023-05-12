#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

namespace avatarmk {
    //scoped by edition
    struct avatars {
        eosio::name avatar_name;
        uint32_t template_id;           //atomic assets template_id
        eosio::name creator;            //creator of the template
        eosio::checksum256 identifier;  //checksum from sorted vector containing all part_template_ids
        uint8_t rarity;
        uint32_t mint;                   //how many are minted
        uint32_t max_mint;               //added for convenience
        eosio::time_point_sec modified;  //timestamp that gets updated each time the row gets modified (assemble, finalize, mint)
        eosio::asset base_price;
        std::vector<uint32_t> avatarparts;

        uint64_t primary_key() const { return avatar_name.value; }
        uint64_t by_creator() const { return creator.value; }
        eosio::checksum256 by_idf() const { return identifier; }
    };
    EOSIO_REFLECT(avatars, avatar_name, template_id, creator, identifier, rarity, mint, max_mint, modified, base_price, avatarparts)
    // clang-format off
    typedef eosio::multi_index<"avatars"_n, avatars,
    eosio::indexed_by<"bycreator"_n, eosio::const_mem_fun<avatars, uint64_t, &avatars::by_creator>>,
    eosio::indexed_by<"byidf"_n, eosio::const_mem_fun<avatars, eosio::checksum256, &avatars::by_idf>>
    >avatars_table;
    // clang-format on
}  // namespace avatarmk
