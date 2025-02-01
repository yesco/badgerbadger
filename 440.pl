while (<>) {
    s/ \d+(\s*)/ 440$1/g;
    print;
}
