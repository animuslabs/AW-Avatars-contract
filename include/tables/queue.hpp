
#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

namespace avatarmk {
    struct namepair {
        std::string bodypart;
        std::string name;
    };
    EOSIO_REFLECT(namepair, bodypart, name)

    struct assemble_set {
        eosio::name creator;
        eosio::name avatar_name;
        std::vector<uint32_t> template_ids;
        uint8_t rarity_score;
        eosio::checksum256 identifier;
        uint32_t max_mint;
        std::vector<namepair> bodypart_names;  ///////////////////
        eosio::name scope;
        eosio::asset base_price;
    };
    EOSIO_REFLECT(assemble_set, creator, avatar_name, template_ids, rarity_score, identifier, max_mint, bodypart_names, scope, base_price)

    struct queue {
        eosio::name avatar_name;
        eosio::checksum256 identifier;   //checksum from sorted vector containing all part_template_ids
        eosio::name work;                //[assemble, potion, etc]
        eosio::name scope;               //which pack
        assemble_set set_data;           //can make this a variant for future work types with different payload
        eosio::time_point_sec inserted;  //timestamp that gets updated each time the row gets modified (assemble, finalize, mint)

        uint64_t primary_key() const { return avatar_name.value; }
        eosio::checksum256 by_idf() const { return identifier; }
        uint64_t by_scope() const { return scope.value; }
    };
    EOSIO_REFLECT(queue, avatar_name, identifier, work, scope, set_data, inserted);
    // clang-format off
    typedef eosio::multi_index<"queue"_n, queue,
    eosio::indexed_by<"byidf"_n, eosio::const_mem_fun<queue, eosio::checksum256, &queue::by_idf>>,
    eosio::indexed_by<"byscope"_n, eosio::const_mem_fun<queue, uint64_t, &queue::by_scope>>
    >queue_table;
    // clang-format on
}  // namespace avatarmk