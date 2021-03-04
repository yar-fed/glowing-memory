#!/bin/bash
# utilities used: convert (from package imagemagick)
#                 unum (wget http://www.fourmilab.ch/webtools/unum/download/unum.tar.gz)
[[ ! -v PIXEL_FONT ]] && PIXEL_FONT="~/Downloads/PixelMplus10-Regular.ttf"

PARSE_COMMAND="{s/.*\"(.)\"\W*(.*)/\1:\2/;/^ (.*)/d;s/[-+ ]/_/g}"
UNICODE_RANGES=($(unum l="cjk.*punctuation" l=hiragana l="katakana$" | sed -E "$PARSE_COMMAND"))
CHARS_NUMBER=${#UNICODE_RANGES[*]}

echo -e "#include \"jp_hi_ka_10.h\""  >jp_hi_ka_ranges.h
echo -e "#include \"jp_hi_ka_12.h\"" >>jp_hi_ka_ranges.h
echo -e "#include \"jp_hi_ka_14.h\"" >>jp_hi_ka_ranges.h
echo -e "" >>jp_hi_ka_ranges.h
echo -e "struct char_ranges {" >>jp_hi_ka_ranges.h
echo -e "\tunsigned int offset;" >>jp_hi_ka_ranges.h
echo -e "\tunsigned int upper_boundary;" >>jp_hi_ka_ranges.h
echo -e "\tstruct {" >>jp_hi_ka_ranges.h
echo -e "\t\tchar *b10[$CHARS_NUMBER];" >>jp_hi_ka_ranges.h
echo -e "\t\tchar *b12[$CHARS_NUMBER];" >>jp_hi_ka_ranges.h
echo -e "\t\tchar *b14[$CHARS_NUMBER];" >>jp_hi_ka_ranges.h
echo -e "\t} bmps;" >>jp_hi_ka_ranges.h
echo -e "} jp_hi_ka_ranges = {" >>jp_hi_ka_ranges.h
echo -e "\t.offset = 0x3000," >>jp_hi_ka_ranges.h
echo -e "\t.upper_boundary = 0x3100," >>jp_hi_ka_ranges.h
echo -e "\t.bmps = {" >>jp_hi_ka_ranges.h
echo -e "\t\t.b10 = {" >>jp_hi_ka_ranges.h
for i in ${UNICODE_RANGES[*]}; do
	echo -e "\t\t\t${i##*:}_xbm10," >>jp_hi_ka_ranges.h
done
echo -e "\t\t}," >>jp_hi_ka_ranges.h
echo -e "\t\t.b12 = {" >>jp_hi_ka_ranges.h
for i in ${UNICODE_RANGES[*]}; do
	echo -e "\t\t\t${i##*:}_xbm12," >>jp_hi_ka_ranges.h
done
echo -e "\t\t}," >>jp_hi_ka_ranges.h
echo -e "\t\t.b14 = {" >>jp_hi_ka_ranges.h
for i in ${UNICODE_RANGES[*]}; do
	echo -e "\t\t\t${i##*:}_xbm14," >>jp_hi_ka_ranges.h
done
echo -e "\t\t}" >>jp_hi_ka_ranges.h
echo -e "\t}" >>jp_hi_ka_ranges.h
echo -e "};" >>jp_hi_ka_ranges.h
echo -e "" >>jp_hi_ka_ranges.h
OFFSET=0x3000
for i in ${UNICODE_RANGES[*]}; do
	printf "#define ${i##*:} %#X\n" $((OFFSET++)) >>jp_hi_ka_ranges.h
done

rm jp_hi_ka_10.h jp_hi_ka_12.h jp_hi_ka_14.h
for i in ${UNICODE_RANGES[*]}; do
	# parse i "<char>:<name>"
	CHAR=${i%%:*}
	NAME=${i##*:}
	# get 10x10 xbm
	[[ ! -f "${NAME}10.xbm" ]] && convert -extent 10x10 -font "$PIXEL_FONT" -pointsize 10 label:${CHAR} "${NAME}10.xbm"
	# add 1 pixel white border
	[[ ! -f "${NAME}12.xbm" ]] && convert "${NAME}10.xbm" -border 1x1 "${NAME}12.xbm"
	# add 1 pixel black border
	[[ ! -f "${NAME}14.xbm" ]] && convert "${NAME}12.xbm" -bordercolor black -border 1x1 "${NAME}14.xbm"
	# yield generated xbms to headers
	cat "${NAME}10.xbm" | sed -E "/^#/d;s/\w+\[]/${NAME}_xbm10\[]/" >>jp_hi_ka_10.h
	cat "${NAME}12.xbm" | sed -E "/^#/d;s/\w+\[]/${NAME}_xbm12\[]/" >>jp_hi_ka_12.h
	cat "${NAME}14.xbm" | sed -E "/^#/d;s/\w+\[]/${NAME}_xbm14\[]/" >>jp_hi_ka_14.h
done

