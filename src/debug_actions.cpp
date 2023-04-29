#include "avatarmk.hpp"

namespace avatarmk {

    template <typename T>
    void cleanTable(eosio::name code, uint64_t account, const uint32_t batchSize)
    {
        T db(code, account);
        uint32_t counter = 0;
        auto itr = db.begin();
        eosio::check(itr != db.end(), "Table already empty");
        while (itr != db.end() && counter++ < batchSize) {
            itr = db.erase(itr);
        }
    }
    void avatarmk_c::clravatars(eosio::name& scope)
    {
        require_auth(get_self());
        cleanTable<avatars_table>(get_self(), scope.value, 100);
    }
    void avatarmk_c::clrwhitelist()
    {
        require_auth(get_self());
        cleanTable<whitelist_table>(get_self(), get_self().value, 100);
    }
    void avatarmk_c::clrqueue()
    {
        require_auth(get_self());
        cleanTable<queue_table>(get_self(), get_self().value, 100);
    }
    void avatarmk_c::clrunpack()
    {
        require_auth(get_self());
        cleanTable<unpack_table>(get_self(), get_self().value, 100);
    }
    void avatarmk_c::test(const uint64_t id)
    {
        // require_auth(get_self());
        // const auto tx_id = get_trx_id();
        // uint64_t signing_value;
        // memcpy(&signing_value, tx_id.data(), sizeof(signing_value));
        // const auto data = std::make_tuple(id, signing_value, get_self());
        // eosio::action({get_self(), "active"_n}, cfg.r, "requestrand"_n, data).send();
    }

}  // namespace avatarmk