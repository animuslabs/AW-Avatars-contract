const conf = require('./eosioConfig')
const env = require('./.env.js')
const activeChain = process.env.CHAIN || env.defaultChain
const { api, tapos, doAction, getFullTable } = require('./lib/eosjs')()
const contractAccount = conf.accountName[activeChain]
const contractActions = require('./do.js')

const methods = {

}

if (require.main == module) {
  if (Object.keys(methods).find(el => el === process.argv[2])) {
    console.log("Starting:", process.argv[2])
    methods[process.argv[2]](...process.argv.slice(3)).catch((error) => console.error(error))
      .then((result) => console.log('Finished'))
  } else {
    console.log("Available Commands:")
    console.log(JSON.stringify(Object.keys(methods), null, 2))
  }
}
module.exports = methods
