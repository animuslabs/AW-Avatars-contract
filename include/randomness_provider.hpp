#pragma once
#include <eosio/eosio.hpp>
#include <eosio/crypto.hpp>

class RandomnessProvider {
   public:
    RandomnessProvider(eosio::checksum256 random_seed)
    {
        raw_values = random_seed.extract_as_byte_array();
        offset = 0;
    }

    uint64_t get_uint64()
    {
        if (offset > 24) {
            regenerate_raw_values();
        }

        uint64_t value = 0;
        for (int i = 0; i < 8; i++) {
            value = (value << 8) + raw_values[offset];
            offset++;
        }
        return value;
    }

    uint32_t get_rand_exclude_zero(uint32_t max_value)
    {
        uint32_t res = 0;
        while (res == 0) {
            //exclude zero as return value
            res = get_uint64() % ((uint64_t)max_value);
        }
        return res;
    }

    uint32_t get_rand(uint32_t max_value) { return get_uint64() % ((uint64_t)max_value); }

   private:
    void regenerate_raw_values()
    {
        eosio::checksum256 new_hash = eosio::sha256((char*)raw_values.data(), 32);
        raw_values = new_hash.extract_as_byte_array();
        offset = 0;
    }

    std::array<uint8_t, 32> raw_values;
    int offset;
};