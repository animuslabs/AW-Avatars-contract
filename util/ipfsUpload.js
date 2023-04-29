// import * as IPFS from 'ipfs-core'
const IPFS = require('ipfs-core')
const fs = require('fs-extra')

async function init() {
  try {
    const ipfs = IPFS.create()
    const imageList = await fs.readJson('./images/partsList.json')
    let newList = []
    for (const image of imageList) {
      const file = await fs.readFile('./images/output/' + image.name + '.png')
      console.log(file)
      const result = await ipfs.add(file)
      console.log(result)
      image.ipfsCard = result.cid
      newList.push(image)
    }
    await fs.writeJson('./images/partsList.json', newList, { spaces: 2 })
  } catch (error) {
    console.error(error)
  }
}

init()
