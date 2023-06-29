#pragma once
// #include "atomicassets.hpp"
#include <cmath>
#include <eosio/asset.hpp>
#include <eosio/crypto.hpp>
#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <eosio/transaction.hpp>
#include <numeric>
#include <token/token.hpp>

#include "tables/avatars.hpp"
#include "tables/config.hpp"
#include "tables/deposits.hpp"
#include "tables/editions.hpp"
#include "tables/packs.hpp"
#include "tables/queue.hpp"
#include "tables/unpack.hpp"
#include "tables/whitelist.hpp"
#define DEBUG
//for some reason it's not possible to include atomicassets.hpp in avatarmk.hpp.
//hence the typedefs for ATTRIBUTE_MAP here.
typedef std::vector<int8_t> INT8_VEC;
typedef std::vector<int16_t> INT16_VEC;
typedef std::vector<int32_t> INT32_VEC;
typedef std::vector<int64_t> INT64_VEC;
typedef std::vector<uint8_t> UINT8_VEC;
typedef std::vector<uint16_t> UINT16_VEC;
typedef std::vector<uint32_t> UINT32_VEC;
typedef std::vector<uint64_t> UINT64_VEC;
typedef std::vector<float> FLOAT_VEC;
typedef std::vector<double> DOUBLE_VEC;
typedef std::vector<std::string> STRING_VEC;

typedef std::variant<int8_t,
                     int16_t,
                     int32_t,
                     int64_t,
                     uint8_t,
                     uint16_t,
                     uint32_t,
                     uint64_t,
                     float,
                     double,
                     std::string,
                     INT8_VEC,
                     INT16_VEC,
                     INT32_VEC,
                     INT64_VEC,
                     UINT8_VEC,
                     UINT16_VEC,
                     UINT32_VEC,
                     UINT64_VEC,
                     FLOAT_VEC,
                     DOUBLE_VEC,
                     STRING_VEC>
    ATOMIC_ATTRIBUTE;

typedef std::map<std::string, ATOMIC_ATTRIBUTE> ATTRIBUTE_MAP;
namespace avatarmk {

    inline constexpr auto atomic_contract = "atomicassets"_n;
    inline constexpr int day_sec = 86400;

    //used as return value by mint price calculation
    struct avatar_mint_price {
        eosio::extended_asset price;
        eosio::asset next_base_price;
    };

    struct avatarmk_c : public eosio::contract {
        using eosio::contract::contract;

#if defined(DEBUG)
        void clravatars(eosio::name& scope);
        void clrqueue();
        void clrunpack();
        void test(const uint64_t id);
        void clrwhitelist();
#endif

        //actions
        void whitelistadd(const eosio::name& account);
        void whitelistdel(const eosio::name& account);

        void setparts(const eosio::name& edition_scope, const std::vector<uint32_t> template_ids, std::vector<uint8_t>& rarity_scores);
        void receiverand(uint64_t& assoc_id, const eosio::checksum256& random_value);

        void setconfig(std::optional<config2> cfg);
        void clrconfig();
        void packadd(eosio::name& edition_scope, uint64_t& template_id, eosio::asset& base_price, eosio::asset& floor_price, std::string& pack_name);
        void packdel(eosio::name& edition_scope, uint64_t& template_id);
        void avatardel(eosio::name& edition_scope, eosio::name& avatar_template_name);
        void editionset(eosio::name& edition_scope, eosio::asset& avatar_floor_mint_price, eosio::asset& avatar_template_price);
        void editiondel(eosio::name& edition_scope);
        void withdraw(const eosio::name& owner, const eosio::extended_asset& value);
        void selfwithdraw(const eosio::name& destination, const eosio::extended_asset& value);
        void open(const eosio::name& owner, eosio::extended_symbol& token, const eosio::name& ram_payer);

        void buypack(eosio::name& buyer, eosio::name& edition_scope, uint64_t& template_id);
        void claimpack(eosio::name& owner, uint64_t& pack_asset_id);
        void assemble(assemble_set& set_data);
        void finalize(eosio::checksum256& identifier, std::string& ipfs_hash);
        void mintavatar(eosio::name& minter, eosio::name& avatar_name, eosio::name& scope, uint64_t holding_tool_id);
        void setowner(eosio::name& owner, eosio::name& new_owner, eosio::name& avatar_name, eosio::name& scope);
        using assemble_action = eosio::action_wrapper<"assemble"_n, &avatarmk_c::assemble>;

        //notifications
        void notify_transfer(eosio::name from, eosio::name to, const eosio::asset& quantity, std::string memo);
        void notify_logtransfer(eosio::name collection_name, eosio::name from, eosio::name to, std::vector<uint64_t> asset_ids, std::string memo);
        void notify_lognewtempl(int32_t template_id,
                                eosio::name authorized_creator,
                                eosio::name collection_name,
                                eosio::name schema_name,
                                bool transferable,
                                bool burnable,
                                uint32_t max_supply,
                                ATTRIBUTE_MAP immutable_data);  //,atomicassets::ATTRIBUTE_MAP immutable_data

       private:
        pack_data validate_pack(const uint64_t& asset_id, const config2& cfg);
        assemble_set validate_assemble_set(std::vector<uint64_t> asset_ids, config2 cfg);
        eosio::checksum256 calculateIdentifier(std::vector<uint32_t>& template_ids);
        avatar_mint_price calculate_mint_price(const avatars& avatar, const eosio::asset& avatar_floor_mint_price);

        //internal accounting
        void add_balance(const eosio::name& owner, const eosio::extended_asset& value, const eosio::name& ram_payer = eosio::name(0));
        void sub_balance(const eosio::name& owner, const eosio::extended_asset& value);

        bool is_whitelisted(const eosio::name& account, const config2& cfg);
        void check_contract_is_frozen(const config2& cfg);
        void require_privileged_account(const config2& cfg);
        std::string rarity_string_from_score(uint8_t rarity_score);
        void burn_nfts(std::vector<uint64_t>& asset_ids);
        eosio::checksum256 get_trx_id();
        void register_part(const eosio::name& edition_scope, const uint32_t& template_id, const uint8_t& rarity_score);
        bool in_range(unsigned low, unsigned high, unsigned x)
        {
            if (low > high) return false;
            return ((x - low) <= (high - low));
        }
    };

    // clang-format off
    EOSIO_ACTIONS(
                avatarmk_c,
                "avatarmk"_n,
                action(packadd, edition_scope, template_id, base_price, floor_price, pack_name),
                action(packdel, edition_scope, template_id),
                action(avatardel, edition_scope, avatar_template_name),
                action(buypack, buyer, edition_scope, template_id),
                action(claimpack, owner, pack_asset_id),
                action(editionset, edition_scope, avatar_floor_mint_price, avatar_template_price),
                action(editiondel, edition_scope),
                action(setparts, edition_scope, template_ids, rarity_scores),
                action(setconfig, cfg),
                action(clrconfig),
                action(withdraw, owner, value),
                action(selfwithdraw, destination, value),
                action(open, owner, token, ram_payer),
                action(assemble, set_data),
                action(finalize, identifier, ipfs_hash),
                action(mintavatar, minter, avatar_name, scope, holding_tool_id),
                action(receiverand, assoc_id, random_value),
                action(setowner, owner, new_owner, avatar_name, scope),
                action(whitelistadd, account),
                action(whitelistdel, account),

                #if defined(DEBUG)
                    action(test, id),
                    action(clravatars, scope),
                    action(clrqueue),
                    action(clrunpack),
                    action(clrwhitelist),
                #endif
                notify("alien.worlds"_n, transfer),
                // notify("eosio.token"_n, transfer),
                notify(atomic_contract, logtransfer),
                notify(atomic_contract, lognewtempl)
    )
    // clang-format on

}  // namespace avatarmk
