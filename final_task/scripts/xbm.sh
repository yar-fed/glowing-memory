#!/bin/bash
# utilities used: 
#	convert (from package imagemagick)
#	unum (wget http://www.fourmilab.ch/webtools/unum/download/unum.tar.gz)
# font can be downloaded here:
#	http://sourceforge.jp/projects/mix-mplus-ipa/downloads/58930/PixelMplus-20130602.zip/
[[ ! -v PIXEL_FONT ]] && PIXEL_FONT="$HOME/Downloads/PixelMplus10-Regular.ttf"
[[ ! -f "$PIXEL_FONT" ]] && echo "Font not found" && exit 1
if [[ ! -v TARGET_DIR ]]; then
	echo "TARGET_DIR not specified"
	if ! mkdir -p /tmp/xbm_script  2>/dev/null; then
		echo "Couldn't create /tmp/xbm_script"
		echo "Falling back to current directory"
		TARGET_DIR="$(pwd)"
		echo "TARGET_DIR is set to $TARGET_DIR"
	else
		echo "Created /tmp/xbm_script"
		TARGET_DIR="/tmp/xbm_script"
		echo "TARGET_DIR is set to $TARGET_DIR"
	fi
fi

PARSE_COMMAND="{s/.*\"(.)\"\W*(.*)/\1:\2/;/^ (.*)/d;s/[-+ ]/_/g}"
UNICODE_RANGES=($(unum l="cjk.*punctuation" l=hiragana l="katakana$" | sed -E "$PARSE_COMMAND"))
CHARS_NUMBER=${#UNICODE_RANGES[*]}

echo -e "#include \"jp_hi_ka_10.h\""  >${TARGET_DIR}/jp_hi_ka_ranges.h
echo -e "#include \"jp_hi_ka_12.h\"" >>${TARGET_DIR}/jp_hi_ka_ranges.h
echo -e "#include \"jp_hi_ka_14.h\"" >>${TARGET_DIR}/jp_hi_ka_ranges.h
echo -e "" >>${TARGET_DIR}/jp_hi_ka_ranges.h
echo -e "struct char_ranges {" >>${TARGET_DIR}/jp_hi_ka_ranges.h
echo -e "\tunsigned int offset;" >>${TARGET_DIR}/jp_hi_ka_ranges.h
echo -e "\tunsigned int upper_boundary;" >>${TARGET_DIR}/jp_hi_ka_ranges.h
echo -e "\tstruct {" >>${TARGET_DIR}/jp_hi_ka_ranges.h
echo -e "\t\tchar *b10[$CHARS_NUMBER];" >>${TARGET_DIR}/jp_hi_ka_ranges.h
echo -e "\t\tchar *b12[$CHARS_NUMBER];" >>${TARGET_DIR}/jp_hi_ka_ranges.h
echo -e "\t\tchar *b14[$CHARS_NUMBER];" >>${TARGET_DIR}/jp_hi_ka_ranges.h
echo -e "\t} bmps;" >>${TARGET_DIR}/jp_hi_ka_ranges.h
echo -e "} jp_hi_ka_ranges = {" >>${TARGET_DIR}/jp_hi_ka_ranges.h
echo -e "\t.offset = 0x3000," >>${TARGET_DIR}/jp_hi_ka_ranges.h
echo -e "\t.upper_boundary = 0x3100," >>${TARGET_DIR}/jp_hi_ka_ranges.h
echo -e "\t.bmps = {" >>${TARGET_DIR}/jp_hi_ka_ranges.h
echo -e "\t\t.b10 = {" >>${TARGET_DIR}/jp_hi_ka_ranges.h
for i in ${UNICODE_RANGES[*]}; do
	echo -e "\t\t\t${i##*:}_xbm10," >>${TARGET_DIR}/jp_hi_ka_ranges.h
done
echo -e "\t\t}," >>${TARGET_DIR}/jp_hi_ka_ranges.h
echo -e "\t\t.b12 = {" >>${TARGET_DIR}/jp_hi_ka_ranges.h
for i in ${UNICODE_RANGES[*]}; do
	echo -e "\t\t\t${i##*:}_xbm12," >>${TARGET_DIR}/jp_hi_ka_ranges.h
done
echo -e "\t\t}," >>${TARGET_DIR}/jp_hi_ka_ranges.h
echo -e "\t\t.b14 = {" >>${TARGET_DIR}/jp_hi_ka_ranges.h
for i in ${UNICODE_RANGES[*]}; do
	echo -e "\t\t\t${i##*:}_xbm14," >>${TARGET_DIR}/jp_hi_ka_ranges.h
done
echo -e "\t\t}" >>${TARGET_DIR}/jp_hi_ka_ranges.h
echo -e "\t}" >>${TARGET_DIR}/jp_hi_ka_ranges.h
echo -e "};" >>${TARGET_DIR}/jp_hi_ka_ranges.h
echo -e "" >>${TARGET_DIR}/jp_hi_ka_ranges.h
OFFSET=0x3000
for i in ${UNICODE_RANGES[*]}; do
	printf "#define ${i##*:} %#X\n" $((OFFSET++)) >>${TARGET_DIR}/jp_hi_ka_ranges.h
done

rm ${TARGET_DIR}/jp_hi_ka_10.h ${TARGET_DIR}/jp_hi_ka_12.h ${TARGET_DIR}/jp_hi_ka_14.h 2>/dev/null
echo "Please, wait. This may take a few minutes."
for i in ${UNICODE_RANGES[*]}; do
	# parse i "<char>:<name>"
	CHAR=${i%%:*}
	NAME=${i##*:}
	# get 10x10 xbm
	[[ ! -f "${TARGET_DIR}/${NAME}10.xbm" ]] && convert -extent 10x10 -font "$PIXEL_FONT" -pointsize 10 label:${CHAR} "${TARGET_DIR}/${NAME}10.xbm"
	# add 1 pixel white border
	[[ ! -f "${TARGET_DIR}/${NAME}12.xbm" ]] && convert "${TARGET_DIR}/${NAME}10.xbm" -border 1x1 "${TARGET_DIR}/${NAME}12.xbm"
	# add 1 pixel black border
	[[ ! -f "${TARGET_DIR}/${NAME}14.xbm" ]] && convert "${TARGET_DIR}/${NAME}12.xbm" -bordercolor black -border 1x1 "${TARGET_DIR}/${NAME}14.xbm"
	# yield generated xbms to headers
	cat "${TARGET_DIR}/${NAME}10.xbm" | sed -E "/^#/d;s/\w+\[]/${NAME}_xbm10\[]/" >>${TARGET_DIR}/jp_hi_ka_10.h
	cat "${TARGET_DIR}/${NAME}12.xbm" | sed -E "/^#/d;s/\w+\[]/${NAME}_xbm12\[]/" >>${TARGET_DIR}/jp_hi_ka_12.h
	cat "${TARGET_DIR}/${NAME}14.xbm" | sed -E "/^#/d;s/\w+\[]/${NAME}_xbm14\[]/" >>${TARGET_DIR}/jp_hi_ka_14.h
done

