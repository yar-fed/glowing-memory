#!/bin/bash

if [[ "$1" != "-C" ]]; then
	RED='\033[1;31m'
	GREEN='\033[1;32m'
	NC='\033[0m' # No Color
fi

function clear_buffer {
	echo 1 > /sys/class/t6/clear
}
function send_message {
	TO=$1
	shift
	echo "$TO: $*" > /dev/t6 2>/dev/null
	echo -e "\n>from $MSG_USER:\n   $*"
}

function chusr {
	echo -n $1 > /proc/t6/user
	MSG_USER=$1
}

function getusr {
	cat /proc/t6/user
}

function echo_g {
	echo -e "${GREEN}${*}${NC}"
}

function echo_r {
	echo -e "${RED}${*}${NC}"
}

rmmod task06
insmod task06.ko major=248

if [[ `getusr` != "monkey1" ]]; then
	chusr monkey1
else
	MSG_USER=monkey1
fi

if diff <(cat /proc/t6/usage) <(echo -e "used: 0\n size: 1024\n msgs_n: 0"); then
	echo_g "Reading empty usage succeded"
else
	echo_r "Reading empty usage failed"
fi

MSGS2=`send_message "monkey2" "hello!"`

chusr monkey2
if [[ `getusr` == "monkey2" ]]; then
	echo_g "Changing user succeded"
else
	echo_r "Changing user failed"
fi

if diff <(cat /dev/t6) <(echo "$MSGS2"); then
	echo_g "Sending 1 message succeded"
else
	echo_r "Sending 1 message failed"
fi

MSGS3="$(send_message 'donkey' 'Hello, donkey!')"
MSGS1=`send_message "monkey1" "Hello, monkey1!"`
MSGS3=`echo -e "${MSGS3}\n$(send_message 'donkey' 'How are you?')"`

chusr monkey1
if diff <(cat /dev/t6) <(echo "$MSGS1"); then
	
	echo_g "Reading message in the middle succeded"
else
	echo_r "Reading message in the middle failed"
fi

MSGS3=`echo -e "${MSGS3}\n$(send_message 'donkey' 'How are you?')"`

chusr donkey
if diff <(cat /dev/t6) <(echo -e "$MSGS3"); then
	echo_g "Reading multiple messages succeded"
else
	echo_r "Reading multiple messages failed"
fi

send_message "monkey1" "Hello, monkey1!" >/dev/null
send_message "monkey1" "Hello, monkey1!" >/dev/null
send_message "monkey1" "Hello, monkey1!" >/dev/null

if diff <(cat /proc/t6/usage) <(echo -e "used: 189\n size: 1024\n msgs_n: 3"); then
	echo_g "Reading usage succeded"
else
	echo_r "Reading usage failed"
fi

chusr monkey1
clear_buffer
if [[ "$(cat /dev/t6)" ]]; then
	echo_r "Clearing buffer failed"
else
	echo_g "Clearing buffer succeded"
fi

