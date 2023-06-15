#include "atomicassets.hpp"
#include "avatarmk.hpp"
#include "randomness_provider.hpp"

namespace avatarmk {

    void avatarmk_c::whitelistadd(const eosio::name& account)
    {
        config_table2 _config(get_self(), get_self().value);
        auto const cfg = _config.get_or_create(get_self(), config2());

        require_privileged_account(cfg);

        eosio::check(cfg.whitelist_enabled, "Whitelist is disabled");

        whitelist_table _whitelist(get_self(), get_self().value);
        auto itr = _whitelist.find(account.value);
        eosio::check(itr == _whitelist.end(), "Account already added to whitelist");
        _whitelist.emplace(get_self(), [&](auto& n) { n.account = account; });
    }

    void avatarmk_c::whitelistdel(const eosio::name& account)
    {
        config_table2 _config(get_self(), get_self().value);
        auto const cfg = _config.get_or_create(get_self(), config2());

        require_privileged_account(cfg);

        whitelist_table _whitelist(get_self(), get_self().value);
        auto itr = _whitelist.require_find(account.value, "Account not found in whitelist");
        _whitelist.erase(itr);
    };

    void avatarmk_c::editionset(eosio::name& edition_scope, eosio::asset& avatar_floor_mint_price, eosio::asset& avatar_template_price)
    {
        //warning no input validation!
        // require_auth(get_self());
        config_table2 _config(get_self(), get_self().value);
        auto const cfg = _config.get_or_create(get_self(), config2());

        require_privileged_account(cfg);

        editions_table _editions(get_self(), get_self().value);
        auto itr = _editions.find(edition_scope.value);

        if (itr == _editions.end()) {
            //new edition
            _editions.emplace(get_self(), [&](auto& n) {
                n.edition_scope = edition_scope;
                n.avatar_template_price = avatar_template_price;
                n.avatar_floor_mint_price = avatar_floor_mint_price;
            });
        }
        else {
            //existing edition
            _editions.modify(itr, eosio::same_payer, [&](auto& n) {
                n.avatar_template_price = avatar_template_price;
                n.avatar_floor_mint_price = avatar_floor_mint_price;
            });
        }
    }
    void avatarmk_c::editiondel(eosio::name& edition_scope)
    {
        config_table2 _config(get_self(), get_self().value);
        auto const cfg = _config.get_or_create(get_self(), config2());

        require_privileged_account(cfg);

        editions_table _editions(get_self(), get_self().value);
        auto itr = _editions.require_find(edition_scope.value, "Edition with this scope not registered");
        _editions.erase(itr);
    }

    void avatarmk_c::setconfig(std::optional<config2> cfg)
    {
        config_table2 _config(get_self(), get_self().value);
        if (_config.exists()) {
            auto const existing_cfg = _config.get();
            require_privileged_account(existing_cfg);
        }
        else {
            require_auth(get_self());
        }
        cfg ? _config.set(cfg.value(), get_self()) : _config.remove();
    }

    void avatarmk_c::clrconfig()
    {
        config_table2 _config(get_self(), get_self().value);
        require_auth(get_self());
        _config.remove();
    }

    void avatarmk_c::mintavatar(eosio::name& minter, eosio::name& avatar_name, eosio::name& scope, uint64_t holding_tool_id)
    {
        require_auth(minter);
        avatars_table _avatars(get_self(), scope.value);
        auto itr = _avatars.require_find(avatar_name.value, "Avatar with this name doesn't exist.");

        config_table2 _config(get_self(), get_self().value);
        auto const cfg = _config.get_or_create(get_self(), config2());

        check_contract_is_frozen(cfg);

        eosio::time_point_sec now(eosio::current_time_point());

        if (itr->mint == 0 && now < itr->modified + 3600) {
            eosio::check(itr->creator == minter, "Only the creator can mint the first avatar within the first hour the template is finalized");
        }

        editions_table _editions(get_self(), get_self().value);
        editions edition_cfg = _editions.get(scope.value, "Scope is not a valid edition");
        auto collection = eosio::name("alien.worlds");
        auto schema = eosio::name("tool.worlds");
        auto templates = atomicassets::get_templates(collection);
        auto collection_schemas = atomicassets::get_schemas(collection);
        auto tool_schema = collection_schemas.get(schema.value, "Schema with name not found in atomicassets contract");
        auto user_assets = atomicassets::get_assets(minter);
        auto user_asset = user_assets.get(holding_tool_id, "user not holding provided tool asset");
        auto t = templates.get(user_asset.template_id, "Template not found");
        auto des_data = atomicassets::deserialize(t.immutable_serialized_data, tool_schema.format);
        auto tool_rarity = std::get<std::string>(des_data["rarity"]);
        check(rarity_string_from_score(itr->rarity) == tool_rarity, "tool rarity does not match avatar rarity");

        //billing logic
        avatar_mint_price amp;
        if (minter != cfg.moderator) {
            amp = calculate_mint_price(*itr, edition_cfg.avatar_floor_mint_price);
            sub_balance(minter, amp.price);
            double pct_cut = 0.75;
            auto contract_share = eosio::extended_asset((uint64_t)(amp.price.quantity.amount * pct_cut), amp.price.get_extended_symbol());
            auto user_reward = amp.price - contract_share;
            add_balance(itr->creator, user_reward, get_self());
            add_balance(get_self(), contract_share, get_self());
        }
        //atomic mint action
        uint32_t new_mint_number = itr->mint + 1;
        const auto mutable_data = atomicassets::ATTRIBUTE_MAP{};
        auto immutable_data = atomicassets::ATTRIBUTE_MAP{};
        immutable_data["mint"] = new_mint_number;
        std::string name_string = itr->avatar_name.to_string();
        name_string[0] = std::toupper(name_string[0]);
        immutable_data["name"] = name_string + " #" + std::to_string(new_mint_number);

        _avatars.modify(itr, eosio::same_payer, [&](auto& n) {
            n.mint = new_mint_number;
            if (minter != cfg.moderator) {
                n.modified = now;
                n.base_price = amp.next_base_price;
            }
        });

        const std::vector<eosio::asset> tokens_to_back;
        const auto data = make_tuple(get_self(), cfg.collection_name, cfg.avatar_schema, itr->template_id, minter, immutable_data, mutable_data, tokens_to_back);
        eosio::action(eosio::permission_level{get_self(), "active"_n}, atomic_contract, "mintasset"_n, data).send();
    }

    void avatarmk_c::setowner(eosio::name& owner, eosio::name& new_owner, eosio::name& avatar_name, eosio::name& scope)
    {
        require_auth(owner);
        avatars_table _avatars(get_self(), scope.value);
        auto itr = _avatars.require_find(avatar_name.value, "Avatar with this name doesn't exist in this scope.");
        eosio::check(itr->creator == owner, "this avatar isn't owned by owner.");
        eosio::check(eosio::is_account(new_owner), "new_owner isn't a valid account.");
        _avatars.modify(itr, eosio::same_payer, [&](auto& n) { n.creator = new_owner; });
    }

    void avatarmk_c::assemble(assemble_set& set_data)
    {
        //assemble adds work to the queue
        require_auth(get_self());
        avatars_table _avatars(get_self(), set_data.scope.value);
        auto a_idx = _avatars.get_index<eosio::name("byidf")>();
        eosio::check(a_idx.find(set_data.identifier) == a_idx.end(), "Avatar with these body parts already available to mint.");

        queue_table _queue(get_self(), get_self().value);
        auto q_idx = _queue.get_index<eosio::name("byidf")>();
        eosio::check(q_idx.find(set_data.identifier) == q_idx.end(), "Avatar with these body parts already in queue.");

        auto q_name = _queue.find(set_data.avatar_name.value);
        eosio::check(q_name == _queue.end(), "avatar with this name already in queue");

        auto a_name = _avatars.find(set_data.avatar_name.value);
        eosio::check(a_name == _avatars.end(), "avatar with this name already exists in edition");

        //add to queue
        _queue.emplace(get_self(), [&](auto& n) {
            n.avatar_name = set_data.avatar_name;
            n.scope = set_data.scope;
            n.identifier = set_data.identifier;
            n.work = eosio::name("assemble");
            n.set_data = set_data;
            n.inserted = eosio::time_point_sec(eosio::current_time_point());
        });
    }
    void avatarmk_c::finalize(eosio::checksum256& identifier, std::string& ipfs_hash)
    {
        //finalize will remove from queue
        config_table2 _config(get_self(), get_self().value);
        auto const cfg = _config.get_or_create(get_self(), config2());

        require_privileged_account(cfg);

        eosio::check(ipfs_hash.size() > 0, "ipfs_hash required");  //this validation can be done better

        queue_table _queue(get_self(), get_self().value);
        auto q_idx = _queue.get_index<eosio::name("byidf")>();
        auto queue_entry = q_idx.require_find(identifier, "Avatar with this identifier not in the queue.");

        eosio::name scope = queue_entry->scope;  // = edition

        avatars_table _avatars(get_self(), scope.value);
        auto a_idx = _avatars.get_index<eosio::name("byidf")>();
        eosio::check(a_idx.find(identifier) == a_idx.end(), "Avatar with this identifier already finalized.");

        //create template inline action
        const eosio::name authorized_creator = get_self();
        const eosio::name collection_name = cfg.collection_name;
        const eosio::name schema_name = cfg.avatar_schema;
        const bool transferable = true;
        const bool burnable = true;
        const uint32_t max_supply = queue_entry->set_data.max_mint;
        auto immutable_data = atomicassets::ATTRIBUTE_MAP{};
        //must match avatar schema!!!!!!!!
        //immutable_data["name"] = queue_entry->set_data.avatar_name.to_string();
        immutable_data["edition"] = scope.to_string();
        immutable_data["img"] = ipfs_hash;
        immutable_data["rarityScore"] = queue_entry->set_data.rarity_score;
        immutable_data["avatarparts"] = queue_entry->set_data.template_ids;  //vector template_ids
        immutable_data["rarity"] = rarity_string_from_score(queue_entry->set_data.rarity_score);
        //add part names to immutable_data
        for (auto p : queue_entry->set_data.avatarpart_names) {
            immutable_data[p.avatarpart] = p.name;
        }

        const auto data = make_tuple(authorized_creator, collection_name, schema_name, transferable, burnable, max_supply, immutable_data);
        eosio::action(eosio::permission_level{get_self(), "active"_n}, atomic_contract, "createtempl"_n, data).send();

        //destructure set_data in avatar table scope + complete with finalize args (ipfs_hash)..
        _avatars.emplace(get_self(), [&](auto& n) {
            n.avatar_name = queue_entry->set_data.avatar_name;
            n.rarity = queue_entry->set_data.rarity_score;
            n.creator = queue_entry->set_data.creator;
            n.identifier = queue_entry->identifier;
            n.base_price = queue_entry->set_data.base_price;
            n.modified = eosio::time_point_sec(eosio::current_time_point());
            n.avatarparts = queue_entry->set_data.template_ids;
            n.max_mint = queue_entry->set_data.max_mint;
        });
        //delete queue entry, not needed anymore.
        q_idx.erase(queue_entry);

        //update template counter in editions table
        editions_table _editions(get_self(), get_self().value);
        auto edition_itr = _editions.require_find(scope.value, "Scope is not a valid edition");
        _editions.modify(edition_itr, eosio::same_payer, [&](auto& n) { n.avatar_template_count += 1; });
    }

    void avatarmk_c::packadd(eosio::name& edition_scope, uint64_t& template_id, eosio::asset& base_price, eosio::asset& floor_price, std::string& pack_name)
    {
        config_table2 _config(get_self(), get_self().value);
        auto const cfg = _config.get_or_create(get_self(), config2());

        require_privileged_account(cfg);

        packs_table _packs(get_self(), edition_scope.value);
        auto p_itr = _packs.find(template_id);
        eosio::check(p_itr == _packs.end(), "Pack with this template_id already in table");

        auto templates = atomicassets::get_templates(cfg.collection_name);
        auto collection_schemas = atomicassets::get_schemas(cfg.collection_name);
        auto pack_schema = collection_schemas.get(cfg.pack_schema.value, "Schema with name not found in atomicassets contract");
        auto t = templates.get(template_id, "Template not found");
        auto des_data = atomicassets::deserialize(t.immutable_serialized_data, pack_schema.format);
        auto rarity_distribution = std::get<std::vector<uint8_t>>(des_data["rarities"]);
        auto total_rarity = std::accumulate(rarity_distribution.begin(), rarity_distribution.end(), 0);
        eosio::check(total_rarity == 100, "Sum of rarity distribitions must equal 100 " + std::to_string(total_rarity) + " " + pack_name);
        eosio::check(rarity_distribution.size() == 5, "There must be a rarity chance specified for each rarity score 1-5");
        _packs.emplace(get_self(), [&](auto& n) {
            n.template_id = template_id;
            n.base_price = base_price;
            n.floor_price = floor_price;
            n.pack_name = pack_name;
        });
    }
    void avatarmk_c::packdel(eosio::name& edition_scope, uint64_t& template_id)
    {
        config_table2 _config(get_self(), get_self().value);
        auto const cfg = _config.get_or_create(get_self(), config2());

        require_privileged_account(cfg);

        packs_table _packs(get_self(), edition_scope.value);
        auto p_itr = _packs.find(template_id);
        eosio::check(p_itr != _packs.end(), "Pack with this template_id not found");
        _packs.erase(p_itr);
    }
    void avatarmk_c::avatardel(eosio::name& edition_scope, eosio::name& avatar_template_name)
    {
        config_table2 _config(get_self(), get_self().value);
        auto const cfg = _config.get_or_create(get_self(), config2());

        require_privileged_account(cfg);

        avatars_table _avatars(get_self(), edition_scope.value);
        auto p_itr = _avatars.find(avatar_template_name.value);
        eosio::check(p_itr != _avatars.end(), "Avatar with this name not found");
        _avatars.erase(p_itr);
    }

    void avatarmk_c::buypack(eosio::name& buyer, eosio::name& edition_scope, uint64_t& template_id)
    {
        require_auth(buyer);

        packs_table _packs(get_self(), edition_scope.value);
        auto p_itr = _packs.require_find(template_id, "Pack with this template_id not found for this scope");

        config_table2 _config(get_self(), get_self().value);
        auto const cfg = _config.get_or_create(get_self(), config2());

        check_contract_is_frozen(cfg);

        eosio::check(is_whitelisted(buyer, cfg), "Only whitelisted accounts can buy packs");

        //calculate price
        if (buyer != cfg.moderator) {
            eosio::extended_asset p = {p_itr->base_price, cfg.payment_token.get_contract()};
            sub_balance(buyer, p);
            add_balance(get_self(), p, get_self());
            _packs.modify(p_itr, eosio::same_payer, [&](auto& n) { n.last_sold = eosio::time_point_sec(eosio::current_time_point()); });
        }

        const auto mutable_data = atomicassets::ATTRIBUTE_MAP{};
        auto immutable_data = atomicassets::ATTRIBUTE_MAP{};
        const std::vector<eosio::asset> tokens_to_back;

        const auto data = make_tuple(get_self(), cfg.collection_name, cfg.pack_schema, (uint32_t)template_id, buyer, immutable_data, mutable_data, tokens_to_back);

        eosio::action(eosio::permission_level{get_self(), "active"_n}, atomic_contract, "mintasset"_n, data).send();
    }

    void avatarmk_c::claimpack(eosio::name& owner, uint64_t& pack_asset_id)
    {
        config_table2 _config(get_self(), get_self().value);
        auto const cfg = _config.get_or_create(get_self(), config2());

        eosio::check(has_auth(get_self()) || has_auth(owner) || has_auth(cfg.moderator), "Need authorization of owner, contract or moderator");

        unpack_table _unpack(get_self(), get_self().value);

        auto itr = _unpack.find(pack_asset_id);
        eosio::check(itr != _unpack.end(), "pack with this id not in the unpack table");
        eosio::check(itr->owner == owner, "you are not the owner of this pack");
        eosio::check(itr->claimable_template_ids.size() != 0, "pack not ready to claim yet. waiting for oracle to draw random cards");
        eosio::check(itr->claimable_template_ids.size() == itr->pack_data.pack_size, "Claimable templates count doesn't equal pack_size");

        check_contract_is_frozen(cfg);

        eosio::check(is_whitelisted(owner, cfg), "Only whitelisted accounts can claim packs");

        const auto mutable_data = atomicassets::ATTRIBUTE_MAP{};
        const auto immutable_data = atomicassets::ATTRIBUTE_MAP{};
        const std::vector<eosio::asset> tokens_to_back;
        for (const auto template_id : itr->claimable_template_ids) {
            const auto data = make_tuple(get_self(), cfg.collection_name, cfg.parts_schema, template_id, owner, immutable_data, mutable_data, tokens_to_back);
            eosio::action(eosio::permission_level{get_self(), "active"_n}, atomic_contract, "mintasset"_n, data).send();
        }
        _unpack.erase(itr);
    }

    void avatarmk_c::receiverand(uint64_t& assoc_id, const eosio::checksum256& random_value)
    {
        config_table2 _config(get_self(), get_self().value);
        auto const cfg = _config.get_or_create(get_self(), config2());

        eosio::check(has_auth(get_self()) || has_auth(cfg.rng), "Need authorization of owner or contract");

        std::vector<uint32_t> result;
        unpack_table _unpack(get_self(), get_self().value);
        auto itr = _unpack.require_find(assoc_id, "error");

        editions_table _editions(get_self(), get_self().value);
        editions edition_cfg = _editions.get(itr->pack_data.edition.value, "Edition not found");

        RandomnessProvider RP(random_value);
        //draw cards
        //std::vector<uint8_t> rd = {5, 10, 20, 25, 40};  //rarity_distribution -> sum must be equal to 100
        std::vector<uint8_t> rd = itr->pack_data.rarity_distribution;

        uint8_t rd1 = rd[0];
        uint8_t rd2 = rd[0] + rd[1];
        uint8_t rd3 = rd[0] + rd[1] + rd[2];
        uint8_t rd4 = rd[0] + rd[1] + rd[2] + rd[3];

        for (int i = 0; i < itr->pack_data.pack_size; i++) {
            int rarity_index = 1;
            uint32_t r = RP.get_rand_exclude_zero(100);
            if (in_range(1, rd1, r)) {
                rarity_index = 5;
            }
            else if (in_range(rd1 + 1, rd2, r)) {
                rarity_index = 4;
            }
            else if (in_range(rd2 + 1, rd3, r)) {
                rarity_index = 3;
            }
            else if (in_range(rd3 + 1, rd4, r)) {
                rarity_index = 2;
            }
            else if (in_range(rd4 + 1, 100, r)) {
                rarity_index = 1;
            }
            auto r2 = RP.get_rand(edition_cfg.part_template_ids[rarity_index - 1].size());
            result.push_back(edition_cfg.part_template_ids[rarity_index - 1][r2]);
        }

        _unpack.modify(itr, eosio::same_payer, [&](auto& n) {
            n.claimable_template_ids = result;
            n.inserted = eosio::time_point_sec(eosio::current_time_point());
        });
        if (cfg.auto_claim_packs) {
            //untested
            auto data = std::make_tuple(itr->owner, assoc_id);
            eosio::action(eosio::permission_level{get_self(), "active"_n}, get_self(), "claimpack"_n, data).send();
        }
    }

    void avatarmk_c::setparts(const eosio::name& edition_scope, const std::vector<uint32_t> template_ids, std::vector<uint8_t>& rarity_scores)
    {
        config_table2 _config(get_self(), get_self().value);
        auto const cfg = _config.get_or_create(get_self(), config2());

        require_privileged_account(cfg);

        editions_table _editions(get_self(), get_self().value);
        auto itr = _editions.find(edition_scope.value);
        eosio::check(itr != _editions.end(), "configure edition before creating new part templates.");
        eosio::check(template_ids.size() == rarity_scores.size(), "invalid input. vectors must be of equal size.");

        if (template_ids.size() == 0) {
            _editions.modify(itr, eosio::same_payer, [&](auto& n) { n.part_template_ids = {{}, {}, {}, {}, {}}; });
            return;
        }

        _editions.modify(itr, eosio::same_payer, [&](auto& n) {
            for (unsigned i = 0; i < template_ids.size(); ++i) {
                auto rarity_index = rarity_scores[i] - 1;
                if (template_ids.size() == 1) {
                    auto existing = std::find(n.part_template_ids[rarity_index].begin(), n.part_template_ids[rarity_index].end(), template_ids[i]);
                    if (existing != n.part_template_ids[rarity_index].end()) {
                        n.part_template_ids[rarity_index].erase(existing);
                    }
                    else {
                        n.part_template_ids[rarity_index].push_back(template_ids[i]);
                    }
                }
                else {
                    n.part_template_ids[rarity_index].push_back(template_ids[i]);
                }
            }
        });
    }

}  // namespace avatarmk
