eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
    & eval 'exec perl -S $0 $argv:q'
    if 0;

# $Id: boost_test.pl 2927 2010-01-21 19:08:30Z buschd $
# -*- perl -*-

#
# Launches a DCPSInfoRepo, 1 Boost-based Publisher process, and
# and N Boost-based Subscriber processes
#
# Usage: boost_test.pl  \
#              <Boost Test Prefix > [<message-size>] <num-messages> \
#              -num-subs <num subscribers>
#
#                 Don't provide <message-size> for "Typed" tests
#
#  e.g. boost_test.pl Boost 1000 10000 -num-subs 5
#       boost_test.pl BoostTypedDotnet 10000 -num-subs 2
#       boost_test.pl BoostTyped 10000 -num-subs 2
#

use Cwd 'abs_path';
use File::Basename;
use lib dirname( abs_path $0 );
#use Env (DDS_ROOT);
#use lib "$DDS_ROOT/bin";
#use Env (ACE_ROOT);
#use lib "$ACE_ROOT/bin";
#use Env (MNB_ROOT);

use PerlDDS::Run_Test;
use File::Which qw(which where);  # exports which() and where()

# Get the local host and IP address
use Socket;
use Sys::Hostname;
my $host = hostname();
my $ipaddr = inet_ntoa(scalar gethostbyname($host || 'localhost'));

print STDOUT "IP address = $ipaddr\n";

$status = 0;
$num_subs = 1;
@REMAINING_ARGV = ();
while (@ARGV) {
    $arg = $ARGV[0];

    if ($arg eq '-num-subs') {
        $num_subs = $ARGV[1];
        print STDOUT "$num_subs subscribers\n";
        shift @ARGV;
        shift @ARGV;
    } else {
        $REMAINING_ARGV[$#REMAINING_ARGV+1] = $arg;
        shift @ARGV;
    }
}

$boost_test_prefix = $REMAINING_ARGV[0];
$publisher_exe = which("$boost_test_prefix" . "Publisher");
$subscriber_exe = which("$boost_test_prefix" . "Subscriber");
$sub_port = 54421;

$message_size = "";
if (not $boost_test_prefix =~ /Typed/) {
    $message_size = $REMAINING_ARGV[1];
    shift @REMAINING_ARGV;
}
$num_messages = $REMAINING_ARGV[1];

$pub_opts = "$message_size $num_messages";
$sub_opts = "$message_size $num_messages";

@Subscribers = ();
for ($i = 0; $i < $num_subs; $i++) {
    $pub_opts = "$pub_opts $ipaddr $sub_port";
    $Subscribers[$i] = PerlDDS::create_process ("$subscriber_exe", " $sub_opts $sub_port");
    $sub_port += 1;
}
$Publisher = PerlDDS::create_process ("$publisher_exe", " $pub_opts");

print $Subscribers[0]->CommandLine() . "\n";
foreach $sub (@Subscribers) {
  $sub->Spawn ();
}


print $Publisher->CommandLine() . "\n";
sleep ($num_subs + 2);
$Publisher->Spawn ();



$PublisherResult = $Publisher->WaitKill (300);
if ($PublisherResult != 0) {
    print STDERR "ERROR: publisher returned $PublisherResult \n";
    $status = 1;
}

foreach $sub (@Subscribers) {
  $SubscriberResult = $sub->WaitKill (100);
  if ($SubscriberResult != 0) {
      print STDERR "ERROR: subscriber returned $SubscriberResult \n";
      $status = 1;
  }
}

if ($status == 0) {
  print "test PASSED.\n";
} else {
  print STDERR "test FAILED.\n";
}

exit $status;
