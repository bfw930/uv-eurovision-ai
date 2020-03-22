{
    name = $1
    numMatches = sub( $1, "", $0 )
    numMatches = gsub( / /, "", $0 )
    numMatches = gsub(/0/, "4", $0 )
    print name, $0
}

