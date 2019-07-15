#!/bin/sh
# ---------CHANGLOG.md auto generator from git log--------------- 
# How to: copy and paste the new log in CHANGELOG_TMP.md and edit 
#         the version number after you run this script
# Author: Danyu L
# Last Edit: 2019/07
# ---------------------------------------------------------------
# get git log with iso date style 
# remove line has "commit", "Danyu", "danyu9394", "merge"
# remove word "Date:"
# remove word for all time zone, e.g. +0800 / -0700
# remove all tabs in the beginning of the line
# remove empty lines
# remove time of the day, e.g. 11:11:11
# remove duplicate line(for a day with multiple commits)
# add a dash in the commit message line
# remove the dash in front of other author line replace it with \n**Added:**
# add ## v0.0.0 in front of date for markdown placeholder

git log --date=iso | \
grep -vwE "(commit|Danyu|danyu9394|Merge)" | \
sed 's/Date://g'| \
sed 's/[-+]0[[:digit:]]00//g' | \
sed "s/^[ \t]*//" | \
sed '/^$/d' | \
sed 's/[[:digit:]]\{2\}\:[[:digit:]]\{2\}\:[[:digit:]]\{2\}//g'  | \
awk '!seen[$0]++' | \
sed 's/^[a-zA-Z]/- &/g' | \
sed 's/^- Author:/\n**Added:**/g' | \
sed 's/^[[:digit:]]\{4\}\-[[:digit:]]\{2\}/## v0.0.0 - &/g'  \
> CHANGELOG_TMP.md