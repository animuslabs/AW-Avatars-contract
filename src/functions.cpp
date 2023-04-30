// #pragma once
#include "atomicassets.hpp"
#include "avatarmk.hpp"

namespace avatarmk {

    avatar_mint_price avatarmk_c::calculate_mint_price(const avatars& avatar, const eosio::asset& avatar_floor_mint_price)
    {
        avatar_mint_price result;

        const auto sec_passed = (eosio::current_time_point() - avatar.modified).to_seconds();
        const auto days_passed = std::floor(sec_passed / day_sec);

        config_table _config(get_self(), get_self().value);
        const auto cfg = _config.get_or_create(get_self(), config());

        //calculate next base price
        if (days_passed == 0) {
            //base price increases by 10% each time a mint happens before a day passes from the previous mint
            double nbp = avatar.base_price.amount * 1.10;
            result.next_base_price = {static_cast<int64_t>(nbp), cfg.payment_token.get_symbol()};
        }
        else {
            result.next_base_price = avatar_floor_mint_price * avatar.rarity;
        }
        //calculate mint price based on current base price
        const uint64_t pv = avatar.base_price.amount;
        const double r = 0.01 * (5 / avatar.rarity);                     //rare avatars will decay slower
        const auto decay_step = days_passed <= 7 ? 0 : days_passed - 7;  //start decay after 1 week
        const uint64_t p = (uint64_t)(pv * pow(1 - r, decay_step));
        eosio::asset mint_price{static_cast<int64_t>(p), cfg.payment_token.get_symbol()};
        mint_price = std::max(mint_price, avatar_floor_mint_price);  //mint price can't be smaller than floor
        result.price = {mint_price, cfg.payment_token.get_contract()};

        return result;
    }

    eosio::checksum256 avatarmk_c::calculateIdentifier(std::vector<uint32_t>& template_ids)
    {
        sort(template_ids.begin(), template_ids.end());
        auto packed_parts = eosio::pack(template_ids);
        std::string sdata = std::string(packed_parts.begin(), packed_parts.end());
        return eosio::sha256(sdata.c_str(), sdata.length());
    }

    pack_data avatarmk_c::validate_pack(const uint64_t& asset_id, const config& cfg)
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
        result.edition = eosio::name(std::get<std::string>(des_data["edition"]));
        editions_table _editions(get_self(), get_self().value);
        _editions.get(result.edition.value, "Pack received with unkown edition.");

        packs_table _packs(get_self(), result.edition.value);
        auto p_itr = _packs.require_find(asset.template_id, "Pack with this template_id not found in packs table.");
        result.rarity_distribution = p_itr->rarity_distribution;

        return result;
    }

    assemble_set avatarmk_c::validate_assemble_set(std::vector<uint64_t> asset_ids, config cfg)
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

            auto body_type = std::get<std::string>(des_data["bodypart"]);  //type

            if (std::find(test_types.begin(), test_types.end(), body_type) != test_types.end()) {
                eosio::check(false, "Duplicate body part type " + body_type);
            }
            else {
                test_types.push_back(body_type);
                result.template_ids.push_back(asset.template_id);
                rarities.push_back(get<uint8_t>(des_data["rarityScore"]));
                //add body part name to set_data
                result.bodypart_names.push_back({body_type, get<std::string>(des_data["name"])});
            }
        }

        //there must be 8 unique body part types
        eosio::check(result.template_ids.size() == 8, "there must be 8 unique body part types");

        result.identifier = calculateIdentifier(result.template_ids);
        result.rarity_score = std::floor(std::accumulate(rarities.begin(), rarities.end(), 0) / 8);
        //result.rarity_score = *std::max_element(rarities.begin(), rarities.end());
        result.max_mint = 10 * pow(6 - result.rarity_score, 2);
        result.base_price = edition_cfg.avatar_floor_mint_price * pow(result.rarity_score, 3);

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

    bool avatarmk_c::is_whitelisted(const eosio::name& account, const config& cfg)
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

    void avatarmk_c::check_contract_is_frozen(const config& cfg)
    {
        if (!has_auth(get_self()) || !has_auth(cfg.moderator)) {
            eosio::check(!cfg.freeze, "Contract is in maintenance. Please try again later.");
        }
    }

    void avatarmk_c::require_privileged_account(const config& cfg)
    {
        if (is_account(cfg.moderator)) {
            eosio::check(eosio::has_auth(get_self()) || eosio::has_auth(cfg.moderator), "Need authorization of moderator or contract");
        }
        else {
            require_auth(get_self());
        }
    }

}  // namespace avatarmk
