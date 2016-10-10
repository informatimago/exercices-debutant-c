#
# Try it out with:
#   bash
#   source characterOfSeason.bash
#   test_characterForSeason
#   c="$(characterForSeason $(date +%m) $(date +%d))"
#   echo "$c"
#

function characterForSeason(){
    local month="${1?Missing month as first argument of $FUNCNAME}"
    local day="${2?Missing day as second argument of $FUNCNAME}"
    case "$month" in
        (01) charOfToday="*" ;;
        (02) charOfToday="*" ;;
        (03) if [ "$day" -le 20 ] ; then
                 charOfToday="*"
             else
                 charOfToday="&"
             fi
            ;;
        (04) charOfToday="&" ;;
        (05) charOfToday="&" ;;
        (06) if [ "$day" -le 20 ] ; then
                 charOfToday="&"
             else
                 charOfToday="@"
             fi
            ;;
        (07) charOfToday="@" ;;
        (08) charOfToday="@" ;;
        (09) if [ "$day" -le 20 ] ; then
                 charOfToday="@"
             else
                 charOfToday="%"
             fi
            ;;
        (10) charOfToday="%" ;;
        (11) charOfToday="%" ;;
        (12) if [ "$day" -le 20 ] ; then
                 charOfToday="@"
             elif [ "$day" -eq 25 ] ; then
                 charOfToday="+"
             else
                 charOfToday="%"
             fi
            ;;
    esac
    echo "$charOfToday"
}

function assertEqual(){
    local result="$1"
    local expected="$2"
    local expression="$3"
    if [[ "$result" != "$expected" ]] ; then
       printf 'FAILURE:  %s\n' "$expression"
       printf '   returned: %s\n' "$result"
       printf '   expected: %s\n' "$expected"
       return 1
    fi
}

function test_characterForSeason(){
   assertEqual "$(characterForSeason 03 10)" '*' 'characterForSeason 03 10'
   assertEqual "$(characterForSeason 03 30)" '&' 'characterForSeason 03 30'
   assertEqual "$(characterForSeason 08 10)" '@' 'characterForSeason 08 10'
   assertEqual "$(characterForSeason 08 30)" '@' 'characterForSeason 08 30'
   assertEqual "$(characterForSeason 09 22)" '%' 'characterForSeason 09 22'
}
