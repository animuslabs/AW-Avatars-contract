#pragma once
#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>

namespace avatarmk {
    struct config2 {
        bool freeze = false;
        eosio::name moderator = "piecesnbitss"_n;
        bool auto_claim_packs = false;
        bool whitelist_enabled = true;
        eosio::extended_symbol payment_token = {{"WAX", 8}, "eosio.token"_n};
        eosio::name rng = "orng.wax"_n;
        eosio::name collection_name = "boidavatars4"_n;
        eosio::name parts_schema = "avatarparts"_n;
        eosio::name avatar_schema = "boidavatars"_n;
        eosio::name pack_schema = "partpacks"_n;
        float avatar_mint_pct_increase = 0.10;
    };
    EOSIO_REFLECT(config2,
                  freeze,
                  moderator,
                  auto_claim_packs,
                  whitelist_enabled,
                  payment_token,
                  rng,
                  collection_name,
                  parts_schema,
                  avatar_schema,
                  pack_schema,
                  avatar_mint_pct_increase)
    typedef eosio::singleton<"config2"_n, config2> config_table2;
}  // namespace avatarmk
