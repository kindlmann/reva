#!/bin/bash
set -o errexit
set -o nounset
JUNK=""
function junk {
  JUNK="$JUNK $@"
}
function cleanup {
  rm -rf $JUNK
}
trap cleanup err exit int term


## for making square-hex grid diagrams
#SZ=4001
#MIN=-2.2
#MAX=2.2
#MAXRR=1
## thickness in pixels of perimeter and dash
#THICK=4

SZ=4001
MIN=-3.2
MAX=3.2
MAXRR=1
# thickness in pixels of perimeter and dash
THICK=4


echo "$MIN $MAX $MIN $MAX" \
 | unu reshape -s 2 2 \
 | unu resample -s $SZ $SZ -k tent -c node -o xx
unu axinfo -i xx -a 0 -mm $MIN $MAX \
 | unu axinfo -a 1 -mm $MAX $MIN -o xx
unu swap -i xx -a 1 0 | unu flip -a 1 | unu inset -i xx -min 0 0 -s - -o yy
unu join -i xx yy -a 0 -incr \
 | unu project -a 0 -m l2 -o rr
junk xx yy rr

unu 2op atan2 yy xx \
 | unu 3op clamp 1.0472 - 1.55 \
 | unu gamma -g 0.7 \
 | unu imap -m isobow3.txt -r \
 | unu quantize -b 8 -min -0.1 -max 0.97 -o bow.png
# cp bow.png bow-A.png
unu flip -i bow.png -a 1 -o bow-flip.png
junk bow-flip.png
unu crop -i bow-flip.png -min 0 0 0 -max M $[$SZ/2] M \
 | unu inset -i bow.png -s - -min 0 0 0 -o bow.png
# cp bow.png bow-B.png
echo "128 128 128" > gray.txt
junk gray.txt
unu join -i xx yy -a 0 -incr \
 | unu project -a 0 -m l2 \
 | unu 3op clamp 0 - $MAXRR \
 | unu 2op x - 2 \
 | unu gamma -g 0.7 \
 | unu axinsert -a 0 \
 | unu quantize -b 16 \
 | unu unquantize -i - -min 0 -max 1 \
 | unu pad -min 0 0 0 -max 2 M M \
 | unu 3op lerp - gray.txt bow.png \
 | unu convert -t uchar -o bow.png
# cp bow.png bow-C.png

if false; then ##########
unu 2op x rr 3.14159 \
 | unu 2op x - 10 \
 | unu 1op sin \
 | unu 1op abs \
 | unu 2op gt - 0.08 -o bands.nrrd
junk bands.nrrd
unu 3op clamp 0.05 rr $MAXRR \
 | unu rmap -m bow.txt \
 | unu quantize -b 8 -min 0 -max 0.7 -o bow0.png
unu axinsert -a 0 -i bands.nrrd \
 | unu pad -min 0 0 0 -max 2 M M \
 | unu 2op x bow.png - -o bow1.png
unu axinsert -a 0 -i bands.nrrd \
 | unu pad -min 0 0 0 -max 2 M M \
 | unu 2op - 1 - \
 | unu 2op x bow0.png - -o bow0.png
unu 2op + bow0.png bow1.png -o bow.png
# cp bow.png bow-D.png
fi #######################

if false; then ############
echo lattfold xx
reva fold -i xx -back -o xxb
echo lattfold yy
reva fold -i yy -back -o yyb
junk xxb yyb
unu join -i xxb xx xxb -a 0 -incr \
 | unu project -a 0 -m var \
 | unu gamma -g 10 -o xm
junk xm
unu join -i yyb yy yyb -a 0 -incr \
 | unu project -a 0 -m var \
 | unu gamma -g 10 -o ym
junk ym
unu 2op max xm ym \
 | unu 2op = - 0 -o mask.nrrd
junk mask.nrrd
unu axinsert -a 0 -i mask.nrrd \
 | unu pad -min 0 0 0 -max 2 M M \
 | unu 2op x bow.png - -o bow.png
fi #########################

reva fold -i xx -back -zf -o - \
 | unu 1op exists -o perim.nrrd
unu resample -i perim.nrrd -s x1 x1 -k tent:$THICK \
 | unu 2op neq 0 - -o perim1.nrrd
unu resample -i perim.nrrd -s x1 x1 -k tent:$THICK \
 | unu 2op = 1 - \
 | unu 2op x 2 - \
 | unu 2op - - perim1.nrrd \
 | unu axinsert -a 0 \
 | unu pad -min 0 0 0 -max 2 M M -o perim.nrrd
junk perim.nrrd perim1.nrrd perim0.nrrd
unu 2op - bow.png 128 -t float \
 | unu 2op x perim.nrrd - \
 | unu 2op + - 128 \
 | unu quantize -b 8 -min 0 -max 255 -o bow.png
# cp bow.png bow-E.png

unu 2op gt yy 0 \
 | unu axinsert -a 0 \
 | unu pad -min 0 0 0 -max 2 M M \
 | unu 3op lerp - 128 bow.png -w 2 -o bow.png
# cp bow.png bow-F.png

unu 2op gt xx 0 \
 | unu resample -s x1 = -k tent:$THICK \
 | unu 3op in_op 0 - 1 -o dash0.nrrd
unu 2op x yy 100 \
 | unu 1op sin \
 | unu 2op x - dash0.nrrd \
 | unu 2op gt - 0 \
 | unu 2op x - -2 \
 | unu 2op + - 1 \
 | unu axinsert -a 0 \
 | unu pad -min 0 0 0 -max 2 M M -o dash.nrrd
junk dash.nrrd dash0.nrrd
unu 2op - bow.png 128 -t float \
 | unu 2op x dash.nrrd - \
 | unu 2op + - 128 \
 | unu quantize -b 8 -min 0 -max 255 -o bow.png

reva fold -i bow.png -back -o - \
 | unu quantize -b 8 -min 0 -max 255 -o bow-unfold.png

echo "255 255 255" > white.txt; junk white.txt
unu 2op eq bow.png 128 \
 | unu project -a 0 -m min \
 | unu axinsert -a 0 -s 3 \
 | unu 3op lerp - bow.png white.txt -w 1 -o bow-white.png

unu 3op lerp 0.5 bow-white.png bow-unfold.png -o bow-blend.png
unu resample -i bow-blend.png -s = 800 800 -o bow-sm.png
