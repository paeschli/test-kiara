my $remove_copyright=0;
while (<>) {
    my $fn = $_;
    if ($remove_copyright eq 0) {
        if (/\/\*  KIARA - Middleware for efficient and QoS\/Security-aware invocation of services and exchange of messages/) {
            $remove_copyright = 1;
            next;
        }
    } elsif ($remove_copyright eq 1) {
        if (/ \*\//) {
            $remove_copyright = 2;
            next;
        }
        next;
    }
    print;
}
