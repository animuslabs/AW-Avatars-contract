module.exports = {
  chains: ['eos', 'kylin', 'wax', 'jungle', 'telosTest', 'waxTest'],
  endpoints: {
    eos: ['https://eos.eosn.io','https://eos.api.animus.is',  'https://eos.dfuse.eosnation.io/'],
    kylin: ['https://kylin.eosn.io', 'https://kylin.eossweden.org'],
    jungle: ['https://jungle3.cryptolions.io', 'https://jungle3.greymass.com'],
    telosTest: ['https://testnet.telos.caleos.io'],
    waxTest: ['https://testnet.waxsweden.org']
  },
  accountName: {
    telosTest: "avatar.boid",
    jungle: "avatar.boid",
    eos: "avatar.boid",
    waxTest: "waxcontract1"
  },
  explorer: {
    waxTest: "https://wax-test.bloks.io"
  },
  contractName: 'avatarmk',
  cppName: 'avatarmk'
}
