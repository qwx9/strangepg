#!/bin/awk -f
# plaintext index wide to long
# d u v0 v1 v2 … → u v0 \n u v1 \n u v2, …
{
	for(i=3; i<=NF; i++)
		print $2, $i
}
