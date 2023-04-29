var spawn = require('child-process-promise').spawn
const conf = require('./eosioConfig')
const fs = require('fs-extra')

/**
 * @param {string} exec native exec
 * @param {string[]} params parameters to pass to executable
 */
async function runCommand(exec, params) {

  var promise = spawn(exec, params, { cwd: '../' })

  var childProcess = promise.childProcess

  console.log('[spawn] childProcess.pid: ', childProcess.pid)

  childProcess.stdout.on('data', function (data) {
    console.log('stdout: ', data.toString())
  })
  childProcess.stderr.on('data', function (data) {
    let e_str = data.toString()
    if (e_str.includes('warning: abigen warning (Action <')) {

    }
    else {
      console.log('stderr: ', e_str)
    }
  })

  return promise
}

const includes = [
  './include'
]

const initPrams = (chain) => {
  let params = [`./src/${conf.cppName}.cpp`, '-abigen']
  includes.forEach(el => {
    params = params.concat(['-I', el])
  })
  params = params.concat(['-O', '3'])
  return params
}

const methods = {
  async debug(data) {
    try {
      try {
        const stats = await fs.stat(`../build/${conf.contractName}.wasm`)
        console.log('wasm size preBuild:', stats.size / 1000, 'kb')
      } catch (error) {
        console.log('no existing wasm file to check size.')
      }
      fs.ensureDirSync('../build')
      const params = initPrams('debug')
        .concat([
          '-o',
          `./build/${conf.contractName}.wasm`])
      // console.log("Building with params:")
      // console.log(params)
      console.log("Building...")
      await runCommand('eosio-cpp', params)
      const stats2 = await fs.stat(`../build/${conf.contractName}.wasm`)
      console.log('wasm size postBuild:', stats2.size / 1000, 'kb')

    } catch (error) {
      console.error(error.toString())
    }

  },
  async prod() {
    console.error('not implemented')
  }
}


if (require.main == module) {
  const param = process.argv[2] || "debug"
  if (Object.keys(methods).find(el => el === param)) {
    console.log("Starting:", param)
    methods[param](...process.argv.slice(3)).catch((error) => console.error(error))
      .then((result) => {
        console.log('Finished')
      })
  } else {
    console.log("Available Commands:")
    console.log(JSON.stringify(Object.keys(methods), null, 2))
  }
}
