// #pragma once
#include "avatarmk.hpp"
#include "atomicassets.hpp"
namespace avatarmk {
    //
    //system core token transfer handler
    //
    void avatarmk_c::notify_transfer(eosio::name from, eosio::name to, const eosio::asset& quantity, std::string memo)
    {
        //dispatcher accepts tokens from all contracts!
        if (from == get_self() || to != get_self()) return;
        if (from == "eosio"_n || from == "eosio.rex"_n || from == "eosio.stake"_n) return;

        if (memo == std::string("deposit")) {
            eosio::check(quantity.amount > 0, "Transfer amount must be positive");
            eosio::extended_asset extended_quantity(quantity, get_first_receiver());
            add_balance(from, extended_quantity);  //
        }
    }

    //
    //atomic asset handlers
    //
    void avatarmk_c::notify_logtransfer(eosio::name collection_name, eosio::name from, eosio::name to, std::vector<uint64_t> asset_ids, std::string memo)
    {
        if (get_first_receiver() != atomic_contract) return;
        config_table2 _config(get_self(), get_self().value);
        const auto cfg = _config.get_or_create(get_self(), config2());
        if (collection_name != cfg.collection_name) return;

        //incoming transfers
        if (to == get_self()) {
            if (memo.substr(0, 8) == "assemble") {
                //validate_assemble_set will assert when not a valid set
                eosio::check(is_whitelisted(from, cfg), "Only whitelisted accounts can assemble parts");
                check_contract_is_frozen(cfg);
                auto assemble_set = validate_assemble_set(asset_ids, cfg);
                assemble_set.creator = from;
                assemble_set.avatar_name = eosio::name(memo.substr(9));  //assemble:avatarname

                editions_table _editions(get_self(), get_self().value);
                auto edition = _editions.get(assemble_set.scope.value, "edition doesn't exists");
                if (edition.avatar_template_price.amount > 0 && from != cfg.moderator) {
                    eosio::extended_asset atp(edition.avatar_template_price, cfg.payment_token.get_contract());
                    sub_balance(from, atp);
                }

                const auto data = std::make_tuple(assemble_set);
                eosio::action(eosio::permission_level{get_self(), "active"_n}, get_self(), "assemble"_n, data).send();  //can be a function call instead of action
                burn_nfts(asset_ids);
            }
            else if (memo == std::string("potion")) {
                //check if received nfts are 1x avatar + potion nfts
                //trigger inline "log" action that the server can intercept to render new nft
                //burn received assets
            }
            else if (memo == std::string("unpack")) {
                eosio::check(is_whitelisted(from, cfg), "Only whitelisted accounts can unpack packs");
                check_contract_is_frozen(cfg);
                eosio::check(asset_ids.size() == 1, "Only can unpack 1 pack at the same time.");
                //get pack info
                auto asset_id = asset_ids[0];
                auto pack_data = validate_pack(asset_id, cfg);

                unpack_table _unpack(get_self(), get_self().value);
                _unpack.emplace(get_self(), [&](auto& n) {
                    n.pack_asset_id = asset_id;
                    n.owner = from;
                    n.pack_data = pack_data;
                    n.inserted = eosio::time_point_sec(eosio::current_time_point());
                });

                const auto tx_id = get_trx_id();
                uint64_t signing_value;
                memcpy(&signing_value, tx_id.data(), sizeof(signing_value));
                const auto data = std::make_tuple(asset_id, signing_value, get_self());
                eosio::action({get_self(), "active"_n}, cfg.rng, "requestrand"_n, data).send();
                burn_nfts(asset_ids);
            }
        }
    }

    void avatarmk_c::notify_lognewtempl(int32_t template_id,
                                        eosio::name authorized_creator,
                                        eosio::name collection_name,
                                        eosio::name schema_name,
                                        bool transferable,
                                        bool burnable,
                                        uint32_t max_supply,
                                        atomicassets::ATTRIBUTE_MAP immutable_data)
    {
        if (get_first_receiver() != atomic_contract) return;

        config_table2 _config(get_self(), get_self().value);
        const auto cfg = _config.get_or_create(get_self(), config2());
        if (collection_name != cfg.collection_name) return;

        if (schema_name == cfg.avatar_schema) {
            //avatar template creation//
            ////////////////////////////
            auto template_ids = std::get<UINT32_VEC>(immutable_data["avatarparts"]);
            auto identifier = calculateIdentifier(template_ids);

            auto scope_str = std::get<std::string>(immutable_data["edition"]);

            avatars_table _avatars(get_self(), eosio::name(scope_str).value);
            auto a_idx = _avatars.get_index<eosio::name("byidf")>();
            auto existing = a_idx.require_find(identifier, "Identifier not found in scoped avatars table during lognewtempl notify handler.");

            //update template_id
            a_idx.modify(existing, eosio::same_payer, [&](auto& n) {
                n.template_id = template_id;
                n.modified = eosio::time_point_sec(eosio::current_time_point());
            });
        }

        else if (schema_name == cfg.parts_schema) {
            //populate table for packs logic
            auto edition_name = eosio::name(std::get<std::string>(immutable_data["edition"]));
            auto rarity_score = std::get<uint8_t>(immutable_data["rarityScore"]);
            eosio::check(rarity_score <= 5 && rarity_score > 0, "rarityscore out of bounds 1-5");
            //template_id
            register_part(edition_name, template_id, rarity_score);
        }

        //
    }

}  // namespace avatarmk
