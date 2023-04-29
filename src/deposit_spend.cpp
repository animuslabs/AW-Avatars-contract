
#include "avatarmk.hpp"

namespace avatarmk {

    void avatarmk_c::sub_balance(const eosio::name& owner, const eosio::extended_asset& value)
    {
        deposits_table _deposits(get_self(), owner.value);
        auto idx = _deposits.get_index<"bycontrsym"_n>();
        const uint128_t composite_id = (uint128_t{value.contract.value} << 64) | value.quantity.symbol.raw();
        const auto& itr = idx.get(composite_id, "No balance with this symbol and contract.");
        eosio::check(itr.balance >= value, "Overdrawn balance. " + owner.to_string() + " only owns " + itr.balance.to_string());

        if (owner != get_self() && itr.balance == value) {
            //auto close account if balance is zero
            _deposits.erase(itr);
            return;
        }

        _deposits.modify(itr, eosio::same_payer, [&](auto& n) {
            n.balance -= value;  //
        });
    }

    void avatarmk_c::add_balance(const eosio::name& owner, const eosio::extended_asset& value, const eosio::name& ram_payer)
    {
        deposits_table _deposits(get_self(), owner.value);
        auto idx = _deposits.get_index<"bycontrsym"_n>();
        const uint128_t composite_id = (uint128_t{value.contract.value} << 64) | value.quantity.symbol.raw();
        auto itr = idx.find(composite_id);

        if (itr == idx.end()) {
            //new table entry
            eosio::check(ram_payer.value, owner.to_string() + " doesn't have a deposit account for this token yet. Use open action first.");
            _deposits.emplace(ram_payer, [&](auto& n) {
                n.id = _deposits.available_primary_key();
                n.balance = value;
            });
        }
        else {
            idx.modify(itr, eosio::same_payer, [&](auto& n) {
                n.balance += value;  //
            });
        }
    }

    void avatarmk_c::open(const eosio::name& owner, eosio::extended_symbol& token, const eosio::name& ram_payer)
    {
        //can make ram_payer optional
        require_auth(ram_payer);
        eosio::check(is_account(owner), "Owner isn't associated with an on-chain account.");

        deposits_table _deposits(get_self(), owner.value);
        auto idx = _deposits.get_index<"bycontrsym"_n>();
        uint128_t composite_id = (uint128_t{token.contract.value} << 64) | token.get_symbol().raw();
        auto itr = idx.find(composite_id);
        eosio::check(itr == idx.end(), "Owner already has an open account for token");

        _deposits.emplace(ram_payer, [&](auto& n) {
            n.id = _deposits.available_primary_key();
            n.balance = eosio::extended_asset(0, token);
        });
    }
    void avatarmk_c::withdraw(const eosio::name& owner, const eosio::extended_asset& value)
    {
        require_auth(owner);
        eosio::check(value.quantity.amount > 0, "Withdraw amount must be possitive.");
        sub_balance(owner, value);
        token::actions::transfer{value.contract, get_self()}.send(get_self(), owner, value.quantity, "withdraw");
    }
}  // namespace avatarmk
