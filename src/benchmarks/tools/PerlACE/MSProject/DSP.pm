# $Id: DSP.pm 14 2007-02-01 15:49:12Z mitza $

package PerlACE::MSProject::DSP;

use strict;
use PerlACE::MSProject;

our @ISA = ("PerlACE::MSProject");

###############################################################################

# Constructor

sub new
{
    my $proto = shift;
    my $class = ref ($proto) || $proto;
    my $self = $class->SUPER::new (@_);

    $self->{COMPILER} = "msdev.com";

    bless ($self, $class);
    return $self;
}

###############################################################################

1;
