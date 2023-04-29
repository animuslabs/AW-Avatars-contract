const fakelish = require('fakelish')
const minLen = 4
const maxLen = 12;

(async () => {
  // Generate 20 fake words
  for (let i = 0; i < 12; i++) {
    // Generate a fake word
    const fakeWord = await fakelish.generateFakeWord(minLen, maxLen)
    // Print the capitalized fake word
    // console.log(capitalize(fakeWord));

  }
})()

// Capitalize string
function capitalize(str) {
  return str.charAt(0).toUpperCase() + str.substring(1)
}
