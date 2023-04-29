#include "avatarmk.hpp"

#include "functions.cpp"
#include "notification_handlers.cpp"
#include "deposit_spend.cpp"
#include "actions.cpp"

#if defined(DEBUG)
#include "debug_actions.cpp"
#endif

EOSIO_ACTION_DISPATCHER(avatarmk::actions)

EOSIO_ABIGEN(
    // Include the contract actions in the ABI
    actions(avatarmk::actions),
    table("whitelist"_n, avatarmk::whitelist),
    table("avatars"_n, avatarmk::avatars),
    table("queue"_n, avatarmk::queue),
    table("deposits"_n, avatarmk::deposits),
    table("config"_n, avatarmk::config),
    table("editions"_n, avatarmk::editions),
    table("packs"_n, avatarmk::packs),
    table("unpack"_n, avatarmk::unpack))