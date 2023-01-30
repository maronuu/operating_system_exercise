BEGIN {
    FS="[<>]"
    print "title\tusername\ttimestamp"
}
/<title>/ {
    title=$3;
}
/<timestamp>/ {
    timestamp=$3;
}
/<username>/ {
    username=$3;
    printf("%s\t%s\t%s\n", title, username, timestamp)
}