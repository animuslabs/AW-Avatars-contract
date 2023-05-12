const nftMetadata = {
  part: {
    schema: [
      { name: 'name', type: 'string' },
      { name: 'edition', type: 'string' },
      { name: 'rarityScore', type: 'uint8' },
      { name: 'avatarpart', type: 'string' },
      { name: 'info', type: 'string' },
      { name: 'url', type: 'string' },
      { name: 'img', type: 'image' }
    ]
  },
  avatar:{
    schema:[
      {name:'name', type:'string'},
      {name:'edition', type:'string'},
      {name:'rarityScore', type: 'uint8' },
      {name:'mint', type: 'uint32' },
      {name:'head', type:'string'},
      {name:'eyes', type:'string'},
      {name:'mouth', type:'string'},
      {name:'top', type:'string'},//old hair
      {name:'torso', type:'string'},//old top
      {name:'legs', type:'string'},//old bottom
      {name:'big-wings', type:'string'},
      {name:'small-wings', type:'string'},
      {name:'avatarparts', type:'uint32[]'},
      {name:'img', type:'image'}
    ]
  },
  pack: {
    schema: [
      { name: 'name', type: 'string' },
      { name: 'edition', type: 'string' },
      { name: 'size', type: 'uint8' },
      { name: 'info', type: 'string' },
      { name: 'chance1', type: 'uint8' },
      { name: 'chance2', type: 'uint8' },
      { name: 'chance3', type: 'uint8' },
      { name: 'chance4', type: 'uint8' },
      { name: 'chance5', type: 'uint8' },
      { name: 'url', type: 'string' },
      { name: 'img', type: 'image' }
    ],
    small: [
      { key: "name", value: ["string", "Small Pack"] },
      { key: "edition", value: ["string", "cartoon"] },
      { key: "size", value: ["uint8", 10] },
      {
        key: 'info', value: ["string",
          "This small pack contains 10 random Boid Avatar Part NFTs. When the pack is opened the random cards will be revealed. Chance1-5 corresponds to the percentage chance that a part card in this pack is of a particular rarity score.  Open this pack at avatar.boid.com/packs/open."]
      },
      { key: "chance1", value: ["uint8", 50] },
      { key: "chance2", value: ["uint8", 30] },
      { key: "chance3", value: ["uint8", 15] },
      { key: "chance4", value: ["uint8", 4] },
      { key: "chance5", value: ["uint8", 1] },
      { key: "url", value: ["string", "https://avatar.boid.com/packs/open"] },
      { key: "img", value: ["string", "QmQMeEY9tFxt4PBnnvbQqP8KWMGmBgtTVC4Y5UYLUbB3Fy"] },
    ],
    medium: [
      { key: "name", value: ["string", "Medium Pack"] },
      { key: "edition", value: ["string", "cartoon"] },
      { key: "size", value: ["uint8", 20] },
      {
        key: 'info', value: ["string",
          "This medium pack contains 20 random Boid Avatar Part NFTs. Chance1-5 corresponds to the percentage chance that a part card in this pack is of a particular rarity score.  Open this pack at avatar.boid.com/packs/open."]
      },
      { key: "chance1", value: ["uint8", 50] },
      { key: "chance2", value: ["uint8", 30] },
      { key: "chance3", value: ["uint8", 15] },
      { key: "chance4", value: ["uint8", 4] },
      { key: "chance5", value: ["uint8", 1] },
      { key: "url", value: ["string", "https://avatar.boid.com/packs/open"] },
      { key: "img", value: ["string", "QmPFWJcKQaXsHsewsYHQFo7HG5u98vawHf386SAovJrP1h"] },
    ],
    large: [
      { key: "name", value: ["string", "Large Pack"] },
      { key: "edition", value: ["string", "cartoon"] },
      { key: "size", value: ["uint8", 30] },
      {
        key: 'info', value: ["string",
          "This large pack contains 30 random Boid Avatar Part NFTs. Chance1-5 corresponds to the percentage chance that a part card in this pack is of a particular rarity score.  Open this pack at avatar.boid.com/packs/open."]
      },
      { key: "chance1", value: ["uint8", 50] },
      { key: "chance2", value: ["uint8", 30] },
      { key: "chance3", value: ["uint8", 15] },
      { key: "chance4", value: ["uint8", 4] },
      { key: "chance5", value: ["uint8", 1] },
      { key: "url", value: ["string", "https://avatar.boid.com/packs/open"] },
      { key: "img", value: ["string", "QmStJeCUYNX1PGYk9HdgLvwbz7gyx6BKWGPc5DnXg6K284"] },
    ],
    rare: [
      { key: "name", value: ["string", "Rare Pack"] },
      { key: "edition", value: ["string", "cartoon"] },
      { key: "size", value: ["uint8", 5] },
      {
        key: 'info', value: ["string",
          "This rare pack contains 5 random Boid Avatar part NFTs.  Chance1-5 corresponds to the percentage chance that a part card in this pack is of a particular rarity score.  Open this pack at avatar.boid.com/packs/open."]
      },
      { key: "chance1", value: ["uint8", 0] },
      { key: "chance2", value: ["uint8", 10] },
      { key: "chance3", value: ["uint8", 50] },
      { key: "chance4", value: ["uint8", 35] },
      { key: "chance5", value: ["uint8", 5] },
      { key: "url", value: ["string", "https://avatar.boid.com/packs/open"] },
      { key: "img", value: ["string", "QmXzmEFQ1CGcUpiKtp7jstoqwdkfzS3xeGM9Sjc8QEx5sk"] },
    ],
    ultrarare: [
      { key: "name", value: ["string", "Ultra Rare Pack"] },
      { key: "edition", value: ["string", "cartoon"] },
      { key: "size", value: ["uint8", 5] },
      {
        key: 'info', value: ["string",
          "This ultra rare pack contains 5 random Boid Avatar Part NFTs. Chance1-5 corresponds to the percentage chance that a part card in this pack is of a particular rarity score.  Open this pack at avatar.boid.com/packs/open."]
      },
      { key: "chance1", value: ["uint8", 0] },
      { key: "chance2", value: ["uint8", 0] },
      { key: "chance3", value: ["uint8", 50] },
      { key: "chance4", value: ["uint8", 40] },
      { key: "chance5", value: ["uint8", 10] },
      { key: "url", value: ["string", "https://avatar.boid.com/packs/open"] },
      { key: "img", value: ["string", "Qmd8ogTWzwjq5TTK7SEdWzkJNudYKvH6zYUYhYaWMraQzJ"] },
    ]
  },
}

module.exports = nftMetadata
