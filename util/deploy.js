const { setAbiAction, setCodeAction } = require('./lib/encodeContractData')
const conf = require('./eosioConfig')
const fs = require('fs-extra')
const env = require('./.env.js')
const activeChain = process.env.CHAIN
const sleep = (ms) => new Promise((resolve) => setTimeout(resolve, ms))
const tapos = { blocksBehind: 20, expireSeconds: 60 }
async function pushAbi(chain, eosjs, data) {
  await eosjs.api.transact(data, tapos)
    .then(result => console.log('ABI:', eosjs.formatBloksTransaction(chain, result.transaction_id)))
    .catch(err => console.log('ABI Error:', err))
}
async function pushWasm(chain, eosjs, data) {
  eosjs.api.transact(data, tapos)
    .then(result => console.log('WASM:', eosjs.formatBloksTransaction(chain, result.transaction_id)))
    .catch(err => console.log('WASM Error:', err.toString()))
}

const methods = {
  async default(chain, target) {
    const eosjs = require('./lib/eosjs')(env.keys[chain], conf.endpoints[chain][0])
    const authorization = [{ actor: conf.accountName[chain], permission: 'active' }]
    console.log('Deploying to:', chain)
    let abiData = { actions: [setAbiAction(`../build/artifacts/${conf.contractName}.abi`, authorization)] }
    let wasmData = { actions: [setCodeAction(`../build/artifacts/${conf.contractName}.wasm`, authorization)] }
    if (target == 'abi') return pushAbi(chain, eosjs, abiData)
    else if (target == 'wasm') return pushWasm(chain, eosjs, wasmData)
    else {
      await pushAbi(chain, eosjs, abiData)
      await sleep(2000)
      await pushWasm(chain, eosjs, wasmData)
    }
    console.log(`${conf.explorer[chain]}/account/${conf.accountName[chain]}`)
  }
}

if (require.main == module) {
  const chain = process.argv[2] || activeChain
  console.log(chain)
  let target = process.argv[3]
  if (!methods[chain]) {
    if (conf.chains.find(el => el == chain)) {
      methods.default(chain, target)
        .catch((error) => console.error(error.toString()))
    } else console.error('invalid chain, must be \n', conf.chains, '\n set the chain using CHAIN=eos or "node deploy eos"', '\n configure chains in eosioConfig.json and .env.js')
  } else {
    methods[chain]()
      .catch((error) => console.error(error.toString()))
  }
}
