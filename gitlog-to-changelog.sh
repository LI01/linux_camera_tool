#!/bin/sh
# ---------CHANGLOG.md auto generator from git log------------------# 
# How to:                                                           #
#   1. run this script to update CHANGELOG_TMP.md after commit      #
#   2. copy, paste the new log in CHANGELOG_TMP.md to CHANGELOG.md  #
#   3. edit the version number in CHANGELOG.md                      #
#   4. re-run this script to update the "gitversion" number         #
#   5. re-compile the program to update version number in software  #
#   6. commit the change to git with "git commit --amend --no-edit" #
#   7. if you want to release the lastest commit:                   #
#           $ git tag -a v0.x.x -m "messages"                       #
#           $ git push -f --tags                                    #
# Author: Danyu L                                                   #
# Last Edit: 2019/09                                                #
# ------------------------------------------------------------------#

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
git log --date=iso                                               |\
grep -vwE "(commit|Danyu|danyu9394|Merge)"                       |\
sed 's/Date://g'                                                 |\
sed 's/[-+]0[[:digit:]]00//g'                                    |\
sed "s/^[ \t]*//"                                                |\
sed '/^$/d'                                                      |\
sed 's/[[:digit:]]\{2\}\:[[:digit:]]\{2\}\:[[:digit:]]\{2\}//g'  |\
awk '!seen[$0]++'                                                |\
sed 's/^[a-zA-Z]/- &/g'                                          |\
sed 's/^- Author:/\n**Added:**/g'                                |\
sed 's/^[[:digit:]]\{4\}\-[[:digit:]]\{2\}/## v0.0.0 - &/g'       \
> CHANGELOG_TMP.md

# put "const char *gitversion = xxxx; " inside the header file
# get line contains Unreleased and one line afterwards
# remove ## in the from
# remove the first line: [Unreleased]
# remove the leading and trailing whitespace 
echo "const char *gitversion = \"$(   \
    grep -A1 Unreleased CHANGELOG.md |\
    sed 's/## //g'                   |\
    sed -n '1!p'                     |\
    awk '{$1=$1};1'                   \
    )\";"                             \
> includes/gitversion.h

# display all the version number
# awk '{ if($2 ~ /v[0-9]./) print $2}' CHANGELOG.md