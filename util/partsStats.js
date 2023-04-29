const partsList = require('./images/partsList.json')
async function init() {
  try {
    let stats = {
      total: partsList.length,
      total1: 0,
      total2: 0,
      total3: 0,
      total4: 0,
      total5: 0
    }
    partsList.forEach(el => {
      if (el.rarity == 1) stats.total1++
      if (el.rarity == 2) stats.total2++
      if (el.rarity == 3) stats.total3++
      if (el.rarity == 4) stats.total4++
      if (el.rarity == 5) stats.total5++
    })
    console.log(stats)
  } catch (error) {
    console.error(error)
  }
}

init()
