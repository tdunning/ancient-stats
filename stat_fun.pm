#!/usr/local/bin/perl -w
use strict;

package stat_fun;

=doc
analyze contingency tables.

a contingency table is a two dimensional array.  this means that 

   my @table = ([100, 200], [200, 100]);
   print log_lr ( \@table ), "\n";
   print chi2 ( \@table ), "\n";

could be used to call either chi2 or log_lr

some examples of the correct output include:

chi2 ([100, 120], [200, 250]) = 0.0609700609700612
log_lr ([100, 120], [200, 250]) = 0.0609417625545348

chi2 ([1000, 100000, 2, 10]) = -1
log_lr ([1000, 100000, 2, 10]) = 7.84244805509004

Copyright 1996, 1995, 1993, Ted Dunning, ted@nmsu.edu
Redistribution permitted if I am notified, and if this copyright notice
is preserved.  Inclusion in other software is also allowed on the same
terms.  Other terms by request.

=cut


my ($mark_errors) = 0;

sub main::chi2 {
    my($table) = @_;
    my($i, $j);

    my($rows) = scalar @$table;
    my($cols) = scalar @{$table->[0]};

    my(@rowsums);
    $i = 0;
    for $i (0 .. $rows-1) {
	@rowsums[$i] = 0;
	for $j (0 .. $cols-1) {
	    @rowsums[$i] += $table->[$i][$j];
	}
	$i++;
    }

    my(@colsums);
    for $j (0 .. $cols-1) {
	@colsums[$j] = 0;
	for $i (0 .. $rows-1) {
	    @colsums[$j] += $table->[$i][$j];
	}
    }

    my($total) = 0;
    for $i (0 .. $rows-1) {
	$total += $rowsums[$i];
    }

    my($result) = 0;
    my($expected);
    for $i (0 .. $rows-1) {
	for $j (0 .. $cols-1) {
	    $expected = $colsums[$j] * $rowsums[$i]/$total;
	    if ($expected < 5) {
		if ($expected != 0) {
		    if ($mark_errors) {
			die "chi^2 inappropriate (table[", $i, "][", $j, "] = ",
			$table->[$i][$j], ") ", $expected, " expected < 5)\n";
		    }
		    else {
			return -1;
		    }
		}
	    }
	    else {
		$result += ($table->[$i][$j] - $expected)**2/$expected;
	    }
	}
    }
    return $result;
}

sub logl {
    my ($p, $total, $k, $cols, $m) = @_;

    my ($sum) = 0;
    my ($logtotal) = log($total);

    my($j);
    for $j (0 .. $cols-1) {
	if ($p->[$j] != 0) {
	    $sum += $k->[$j] * log($p->[$j] / $total);
	}
	elsif ($k->[$j] != 0) {
	    die "data error, column ", $j, " is completely zero\n";
	}
    }

    return $sum;
}

sub main::log_lr {
    my ($table) = @_;

    my($i, $j);

    my($rows) = scalar @$table;
    my($cols) = scalar @{$table->[0]};

    my(@rowsums);
    $i = 0;
    for $i (0 .. $rows-1) {
	@rowsums[$i] = 0;
	for $j (0 .. $cols-1) {
	    @rowsums[$i] += $table->[$i][$j];
	}
	$i++;
    }

    my(@colsums);
    for $j (0 .. $cols-1) {
	@colsums[$j] = 0;
	for $i (0 .. $rows-1) {
	    @colsums[$j] += $table->[$i][$j];
	}
    }

    my($total) = 0;
    for $i (0 .. $rows-1) {
	$total += $rowsums[$i];
    }

    my ($sum) = 0;
    for $i (0 .. $rows-1) {
	$sum +=
	    logl($table->[$i], @rowsums[$i], $table->[$i], $cols, $rows*$cols) -
		logl(\@colsums, $total, $table->[$i], $cols, $rows*$cols);
    }
    return 2*$sum;
}

