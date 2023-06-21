// #pragma once
#include "atomicassets.hpp"
#include "avatarmk.hpp"

namespace avatarmk {

    avatar_mint_price avatarmk_c::calculate_mint_price(const avatars& avatar, const eosio::asset& avatar_floor_mint_price)
    {
        avatar_mint_price result;

        const auto sec_passed = (eosio::current_time_point() - avatar.modified).to_seconds();
        const auto days_passed = std::floor(sec_passed / day_sec);

        config_table2 _config(get_self(), get_self().value);
        const auto cfg = _config.get();
        double nbp = avatar.base_price.amount * cfg.avatar_mint_pct_increase;
        result.next_base_price = {static_cast<int64_t>(avatar.base_price.amount + nbp), cfg.payment_token.get_symbol()};

        //calculate mint price based on current base price
        const uint64_t pv = avatar.base_price.amount;
        const double r = 0.01 * (5.0 / double(avatar.rarity));  // rare avatars will decay slower
        // check(false,std::to_string(r));
        const auto decay_step = days_passed;
        const uint64_t p = (uint64_t)(pv * pow(1 - r, decay_step));
        // check(false,std::to_string(p));
        eosio::asset mint_price{static_cast<int64_t>(p), cfg.payment_token.get_symbol()};
        mint_price = std::max(mint_price, avatar_floor_mint_price);
        result.price = {mint_price, cfg.payment_token.get_contract()};
        // check(false,mint_price.to_string());
        return result;
    }

    eosio::checksum256 avatarmk_c::calculateIdentifier(std::vector<uint32_t>& template_ids)
    {
        sort(template_ids.begin(), template_ids.end());
        auto packed_parts = eosio::pack(template_ids);
        std::string sdata = std::string(packed_parts.begin(), packed_parts.end());
        return eosio::sha256(sdata.c_str(), sdata.length());
    }

    pack_data avatarmk_c::validate_pack(const uint64_t& asset_id, const config2& cfg)
    {
        pack_data result;

        auto received_assets = atomicassets::get_assets(get_self());
        auto asset = received_assets.get(asset_id, "Asset not received in contract");
        eosio::check(cfg.pack_schema == asset.schema_name, "asset doesn't have the correct pack_schema");
        auto templates = atomicassets::get_templates(cfg.collection_name);
        auto collection_schemas = atomicassets::get_schemas(cfg.collection_name);
        auto pack_schema = collection_schemas.get(cfg.pack_schema.value, "Schema with name not found in atomicassets contract");
        auto t = templates.get(asset.template_id, "Template not found");
        auto des_data = atomicassets::deserialize(t.immutable_serialized_data, pack_schema.format);
        result.pack_size = std::get<uint8_t>(des_data["size"]);
        std::string editionString = std::get<std::string>(des_data["edition"]);
        editionString[0] = std::tolower(editionString[0]);
        result.edition = eosio::name(editionString);
        result.rarity_distribution = std::get<std::vector<uint8_t>>(des_data["rarities"]);
        editions_table _editions(get_self(), get_self().value);
        _editions.get(result.edition.value, "Pack received with unkown edition.");

        packs_table _packs(get_self(), result.edition.value);
        auto p_itr = _packs.require_find(asset.template_id, "Pack with this template_id not found in packs table.");
        // result.rarity_distribution = p_itr->rarity_distribution;

        return result;
    }

    assemble_set avatarmk_c::validate_assemble_set(std::vector<uint64_t> asset_ids, config2 cfg)
    {
        //result to return only if valid, else assert.

        assemble_set result;
        //temp containers
        std::vector<uint8_t> rarities;
        std::vector<std::string> test_types;

        auto received_assets = atomicassets::get_assets(get_self());
        auto templates = atomicassets::get_templates(cfg.collection_name);
        auto collection_schemas = atomicassets::get_schemas(cfg.collection_name);
        auto schema = collection_schemas.get(cfg.parts_schema.value, "Schema with name not found in atomicassets contract");

        editions_table _editions(get_self(), get_self().value);
        editions edition_cfg;
        // check(false, "beforeloop");
        for (uint64_t asset_id : asset_ids) {
            auto asset = received_assets.get(asset_id, "Asset not received in contract");

            auto t = templates.get(asset.template_id, "Template not found");

            auto des_data = atomicassets::deserialize(t.immutable_serialized_data, schema.format);

            auto scope = eosio::name(std::get<std::string>(des_data["edition"]));

            if (result.scope.value == 0) {
                edition_cfg = _editions.get(scope.value, "Edition not found in editions table");
                result.scope = scope;
            }
            else {
                eosio::check(scope == result.scope, "body parts from different editions/scope received.");
            }

            // check(false, "inloop");
            auto body_type = std::get<std::string>(des_data["avatarpart"]);  //type

            if (std::find(test_types.begin(), test_types.end(), body_type) != test_types.end()) {
                eosio::check(false, "Duplicate body part type " + body_type);
            }
            else {
                test_types.push_back(body_type);
                result.template_ids.push_back(asset.template_id);
                rarities.push_back(get<uint8_t>(des_data["rarityScore"]));
                //add body part name to set_data
                result.avatarpart_names.push_back({body_type, get<std::string>(des_data["name"])});
            }
        }

        eosio::check(result.template_ids.size() == edition_cfg.num_avatarparts, "there must be correct number of unique body part types");

        result.identifier = calculateIdentifier(result.template_ids);
        result.rarity_score = std::floor(std::accumulate(rarities.begin(), rarities.end(), 0) / edition_cfg.num_avatarparts);
        result.max_mint = std::floor(5 * pow(6 - result.rarity_score, 1));
        result.base_price = edition_cfg.avatar_floor_mint_price * pow(result.rarity_score, 2);

        return result;
    };

    //
    void avatarmk_c::burn_nfts(std::vector<uint64_t>& asset_ids)
    {
        for (auto asset_id : asset_ids) {
            const auto data = std::make_tuple(get_self(), asset_id);
            eosio::action(eosio::permission_level{get_self(), "active"_n}, atomic_contract, "burnasset"_n, data).send();
        }
    };

    eosio::checksum256 avatarmk_c::get_trx_id()
    {
        auto size = transaction_size();
        char* buffer = (char*)(512 < size ? malloc(size) : alloca(size));
        uint32_t read = read_transaction(buffer, size);
        eosio::check(size == read, "read_transaction failed");
        return sha256(buffer, read);
    }

    void avatarmk_c::register_part(const eosio::name& edition_scope, const uint32_t& template_id, const uint8_t& rarity_score)
    {
        editions_table _editions(get_self(), get_self().value);
        auto itr = _editions.find(edition_scope.value);
        eosio::check(itr != _editions.end(), "configure edition before creating new part templates");

        auto rarity_index = rarity_score - 1;

        _editions.modify(itr, eosio::same_payer, [&](auto& n) { n.part_template_ids[rarity_index].push_back(template_id); });
    }

    bool avatarmk_c::is_whitelisted(const eosio::name& account, const config2& cfg)
    {
        if (cfg.whitelist_enabled) {
            whitelist_table _whitelist(get_self(), get_self().value);
            auto itr = _whitelist.find(account.value);
            return itr != _whitelist.end();
        }
        else {
            return true;
        }
    }

    void avatarmk_c::check_contract_is_frozen(const config2& cfg)
    {
        if (!has_auth(get_self()) || !has_auth(cfg.moderator)) {
            eosio::check(!cfg.freeze, "Contract is in maintenance. Please try again later.");
        }
    }

    void avatarmk_c::require_privileged_account(const config2& cfg)
    {
        if (is_account(cfg.moderator)) {
            eosio::check(eosio::has_auth(get_self()) || eosio::has_auth(cfg.moderator), "Need authorization of moderator or contract");
        }
        else {
            require_auth(get_self());
        }
    }
    std::string avatarmk_c::rarity_string_from_score(uint8_t rarity_score)
    {
        if (rarity_score == 1) return "Common";
        else if (rarity_score == 2)
            return "Rare";
        else if (rarity_score == 3)
            return "Epic";
        else if (rarity_score == 4)
            return "Legendary";
        else if (rarity_score == 5)
            return "Mythical";
        else {
            // check(false, "invalid rarity_score");
            return "Mythical";
        }
    }

}  // namespace avatarmk

// if (rarNum === 1) return 'common'
// else if (rarNum === 2) return 'rare'
// else if (rarNum === 3) return 'epic'
// else if (rarNum === 4) return 'legendary'
// else if (rarNum === 5) return 'mythical'
