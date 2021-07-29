const https = require('https')
const { JsonRpc } = require('eosjs');
const fetch = require('node-fetch');
const rpc = new JsonRpc('https://telos.caleos.io/', { fetch });


/*
    name: 'Destiny Coin Telos',
    logo: 'https://raw.githubusercontent.com/eoscafe/eos-airdrops/master/logos/DECO.png',
    logo_lg: 'https://raw.githubusercontent.com/eoscafe/eos-airdrops/master/logos/DECO.png',
    symbol: 'DECO',
    account: 'destinytoken',
    chain: 'telos'
*/
// https://raw.githubusercontent.com/eoscafe/eos-airdrops/master/tokens.json

const options = {
  hostname: 'raw.githubusercontent.com',
  port: 443,
  path: '/eoscafe/eos-airdrops/master/tokens.json',
  method: 'GET'
}

const req = https.get('https://raw.githubusercontent.com/eoscafe/eos-airdrops/master/tokens.json', res => {
  console.log(`statusCode: ${res.statusCode}`)

  let body = ''
  res.on('data', d => {
    body += d
  })
  
  res.on('end', async () => {
    let drops = JSON.parse(body)
    let telosDrops = drops.filter(drop => drop.chain == 'telos')
    telosDrops.forEach(async t => {
     try {
      let stat = await rpc.get_table_rows({
        code: t.account,
        scope: t.symbol,
        table: 'stat',
        limit: 1,
        lower_bound: t.symbol,
      })
      if (!stat.rows.length) {
        return;
      }
      let symbol = `${stat.rows[0].supply.split(' ')[0].split('.')[1].length},${t.symbol}`;
      console.log(`cleos -u https://telos.caleos.io push action tokenmanager addtoken '["${t.name}","${t.account}","${t.account}","${symbol}","${t.logo}","${t.logo_lg}"]' -p tokenmanager@active`)
     } catch (e) {
     }
    })
  })
}).on("error", (err) => {
});

