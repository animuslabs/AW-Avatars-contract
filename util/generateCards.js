var gm = require('gm')
var fs = require('fs-extra')
const fakelish = require('fakelish')
let minLen = 4
let maxLen = 12
const range = (OldValue, OldMin, OldMax, NewMin, NewMax) => (((OldValue - OldMin) * (NewMax - NewMin)) / (OldMax - OldMin)) + NewMin
function capitalize(str) {
  return str.charAt(0).toUpperCase() + str.substring(1)
}
function randomInt(min, max) {
  return Math.floor(Math.random() * (max - min + 1) + min)
}
function generateRarity() {
  let rand = randomInt(1, 100)
  if (rand < 5) return 5
  else if (rand < 20) return 4
  else if (rand < 35) return 3
  else if (rand < 60) return 2
  else return 1
}

async function random(test) {
  try {
    let rawList
    if (test) rawList = require('./avatarparts.json').slice(test)
    else rawList = require('./avatarparts.json')
    await fs.ensureDir('./images/output')
    let partsList = []
    for (const rawPart of rawList) {
      let name
      if (test) name = capitalize('test')
      else name = await fakelish.generateFakeWord(minLen, maxLen)
      let rarity = generateRarity()
      const file = `./images/output/${name}.png`
      const rawArt = `./images/avatar-renders/${rawPart.filepath}`
      const tempArt = './images/temp.png'
      const backArt = `./images/templates/card-template-bg-${rarity}.png`
      const frontArt = `./images/templates/card-template-fg-${rarity}.png`
      await writeImage(name, rawArt, tempArt, file, rawPart.type, frontArt, backArt)
      rawPart['name'] = name.toLowerCase()
      rawPart.rarity = rarity
      console.log(rawPart)
      if (test) return
      partsList.push(rawPart)
    }
    if (test) return
    await fs.writeJSON('./images/partsList.json', partsList, { spaces: 2 })
  } catch (error) {
    console.error(error)
  }
}

async function configured(name) {
  try {
    if (!name) throw ('must provide name from partsList.json')
    const partsList = await fs.readJson('./images/partsList.json')
    const partConfig = partsList.find(el => el.name == name)
    if (!partConfig) throw ("can't find part with that value")

    const file = `./images/output/${name}.png`
    const rawArt = `./images/avatar-renders/${partConfig.filepath}`
    const tempArt = './images/temp.png'
    const backArt = `./images/templates/card-template-bg-${partConfig.rarity}.png`
    const frontArt = `./images/templates/card-template-fg-${partConfig.rarity}.png`
    await writeImage(name, rawArt, tempArt, file, partConfig.type, frontArt, backArt)


    // let rawList
    // if (test) rawList = require('./avatarparts.json').slice(test)
    // else rawList = require('./avatarparts.json')
    // await fs.ensureDir('./images/output')
    // let partsList = []
    // for (const rawPart of rawList) {
    //   let name
    //   if (test) name = capitalize('test')
    //   else name = await fakelish.generateFakeWord(minLen, maxLen)
    //   let rarity = generateRarity()
    //   const file = `./images/output/${name}.png`
    //   const rawArt = `./images/avatar-renders/${rawPart.filepath}`
    //   const tempArt = './images/temp.png'
    //   const backArt = `./images/templates/card-template-bg-${rarity}.png`
    //   const frontArt = `./images/templates/card-template-fg-${rarity}.png`
    //   await writeImage(name, rawArt, tempArt, file, rawPart.type, frontArt, backArt)
    //   rawPart['name'] = name.toLowerCase()
    //   rawPart.rarity = rarity
    //   console.log(rawPart);
    //   if (test) return
    //   partsList.push(rawPart)
    // }
    // if (test) return
    // await fs.writeJSON('./images/partsList.json', partsList, { spaces: 2 })
  } catch (error) {
    console.error(error)
  }
}

async function writeImage(name, rawArt, tempArt, file, type, frontArt, backArt) {
  return new Promise((res) => {
    gm(rawArt).geometry(determineSize(type)).gravity('North').write(tempArt, (err, el) => {
      gm(backArt).composite(tempArt).geometry(determinOffset(type)).gravity('North').write(file, (err, el) => {
        gm(file).composite(frontArt).write(file, (err, el) => {
          gm(file)
            .font('/Library/Fonts/Comfortaa-Bold.ttf', 100)
            .drawText(110, 160, capitalize(name), 'SouthWest')
            .font('/Library/Fonts/Comfortaa-Normal.ttf', 60)
            .drawText(120, 60, capitalize(type), 'SouthWest')
            // .mask('./images/templates/card-template-mask.jpg')
            .write(file, () => {
              gm('./images/templates/card-template-mask.png').compose('In').composite(file).write(file, res)
              // gm(file).mask('./images/templates/card-template-mask.png').colorize(2, 3, 2).drawText(320, 160, 'blam!', 'SouthWest').write(file, (err, el) => {
              //   if (err) throw (err)
              //   res()
              // })
            })
        })
      })
    })
  })
}

function determineSize(type) {
  if (type == 'eyes') return '80%'
  else if (type == 'top') return '80%'
  else if (type == 'bottom') return '90%'
  else if (type == 'small-wings') return '105%'
  else if (type == 'big-wings') return '70%'
  else if (type == 'hair') return '64%'
  else if (type == 'mouth') return '75%'
  else if (type == 'head') return '75%'

  else return '50%'
}
function determinOffset(type) {
  if (type == 'eyes') return '+100-80'
  else if (type == 'top') return '+100-700'
  else if (type == 'bottom') return '+100-1400'
  else if (type == 'small-wings') return '-500-1450'
  else if (type == 'big-wings') return '-700-250'
  else if (type == 'hair') return '+50+150'
  else if (type == 'mouth') return '+160-50'
  else if (type == 'head') return '+160-50'
  // let pct = range(height, 150, 2600, 800, 450)
  // console.log('offset', pct);
  // return '+0+' + (pct - (height / 2.3))
}

async function renderAllConfigured() {
  const partsList = await fs.readJson('./images/partsList.json')
  for (const part of partsList) {
    console.log(part)
    await configured(part.name)
  }
}


random()
// configured(process.argv[2])

// renderAllConfigured()

module.exports = { random, configured, renderAllConfigured }
