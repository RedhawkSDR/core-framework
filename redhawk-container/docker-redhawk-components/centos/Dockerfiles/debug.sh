sleep $1
str=""
for x in ${@:2}; do
	str=${str}" "${x}
done
echo ${str}
${str}
