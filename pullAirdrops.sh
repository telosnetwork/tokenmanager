for i in `curl https://raw.githubusercontent.com/eoscafe/eos-airdrops/master/tokens.json | jq -c '.[] | select(.chain == "telos")'`; do
  name="$(jq -r '.name' <<< ${i})"
  echo $name
done
