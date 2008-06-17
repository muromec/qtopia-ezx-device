#!/bin/sh

(cat headers | while read f
do
d=$(locate -e $f)
echo -I${d%$f}
done)|sort|uniq


(cat libs | while read f
do
d=$(locate -e $f)
echo -L${d%$f}
done)|sort|uniq

