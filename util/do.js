const conf = require('./eosioConfig')
const env = require('./.env.js')
const utils = require('@deltalabs/eos-utils')
const { api, tapos, doAction } = require('./lib/eosjs')()
const activeChain = process.env.CHAIN || env.defaultChain
const contractAccount = conf.accountName[activeChain]

const methods = {
  async clrcfg(param1, param2) {
    await doAction('setconfig', {})
  },
  async clrqueue() {
    await doAction('clrqueue')
  },
  async setconfig() {
    await doAction('setconfig', { cfg:
      { freeze: false,
        moderator:"nft.boid",
        auto_claim_packs: false,
        whitelist_enabled: false,
        collection_name: "avatar.boid",
        parts_schema: "avatarparts",
        avatar_schema: "boidavatars",
        pack_schema: "partpacks",
        rng:"rng.boid",
        payment_token:{contract:"boidcomtoken",sym:"4,BOID"}
      } })
  },
  async clravatars(scope) {
    await doAction('clravatars', { scope })
  },
  async editionset() {
    await doAction('editionset', {
      edition_scope:"cartoon",
      avatar_floor_mint_price:"1000.0000 BOID",
      avatar_template_price:"100000.0000 BOID"
    })
  },
  async clrunpack() {
    await doAction('clrunpack', { })
  },
  async whitelistadd(account) {
    await doAction('whitelistadd', { account })
  },
  async packdel(edition_scope, template_id) {
    await doAction('packdel', { edition_scope, template_id })
  },
  async clearpacks(edition_scope) {
    const allPacks = await api.rpc.get_table_rows({code:contractAccount,table:'packs',limit:1000,scope:edition_scope})
    console.log(allPacks.rows.length)
    for(const packData of allPacks.rows) {
      await doAction('packdel', { edition_scope, template_id:packData.template_id })
    }
  },
  async packadd(edition_scope, template_id, base_price, floor_price, pack_name, rarity_distribution) {
    await doAction('packadd', { edition_scope, template_id: parseInt(template_id), base_price, floor_price, pack_name, rarity_distribution: JSON.parse(rarity_distribution) })
  },
  async clearparts(edition_scope) {
    template_ids = []
    rarity_scores = []
    await doAction('setparts', { edition_scope, template_ids, rarity_scores })
  }
}

if (require.main == module) {
  if (Object.keys(methods).find(el => el === process.argv[2])) {
    console.log('Starting:', process.argv[2])
    methods[process.argv[2]](...process.argv.slice(3)).catch(error => console.error(error))
      .then(result => console.log('Finished'))
  } else {
    console.log('Available Commands:')
    console.log(JSON.stringify(Object.keys(methods), null, 2))
  }
}
module.exports = methods
