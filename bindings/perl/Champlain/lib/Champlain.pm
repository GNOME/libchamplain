package Champlain;

=head1 NAME

Champlain - Map rendering canvas

=head1 SYNOPSIS

	use Clutter '-init';
	use Champlain;
	
	# Standard clutter canvas
	my $stage = Clutter::Stage->get_default();
	$stage->set_size(800, 600);
	
	# Create the map view and set some properties
	my $map = Champlain::View->new();
	$map->set('scroll-mode', 'kinetic');
	$map->set_size(800, 600);
	$map->set_property('zoom-level', 7);
	$map->center_on(45.466, -73.75);
	
	# Pack the actors	
	$stage->add($map);
	$stage->show_all();
	
	# Main loop
	Clutter->main();

=head1 DESCRIPTION

Champlain is a Perl binding for the C library libchamplain which provides a
canvas widget based on Clutter that displays maps from various free map sources
such as I<OpenStreetMap>, I<OpenArialMap> and I<Maps for free>.

If the C library is compiled with GTK support then the map widget can also be
embedded in any GTK application.

For more information about libchamplain see:
L<http://projects.gnome.org/libchamplain/>.

=head1 EXPORTS

The library makes the following constants available:

=over

=item MIN_LAT

=item MAX_LAT

=item MIN_LONG

=item MAX_LONG

=back

The tag I<coords> can be used for importing the constants providing the minimal
and maximal values for (latitude, longitude) coordinates:

	use Champlain ':coords';

=head1 BUGS

The library libchamplain is quite young and its API is changing as the code
gains maturity. These bindings try to provide as much coverage from the C
library as possible. Don't be surprised if the API changes within the next
releases this is normal as B<libchamplain IS NOT yet API nor ABI frozen>.

It's quite probable that bugs will be exposed, please report all bugs found.

=head1 AUTHORS

Emmanuel Rodriguez E<lt>potyl@cpan.orgE<gt>.

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2009 by Emmanuel Rodriguez.

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself, either Perl version 5.8.8 or,
at your option, any later version of Perl 5 you may have available.

=cut

use warnings;
use strict;

our $VERSION = '0.01';

use base 'DynaLoader';
use Exporter 'import';


use constant {
	MIN_LAT  => -90,
	MAX_LAT  =>  90,
	MIN_LONG => -180,
	MAX_LONG =>  180,
};


our %EXPORT_TAGS = (
	coords => [qw(MIN_LAT MAX_LAT MIN_LONG MAX_LONG)],
);

our @EXPORT_OK = map { @{ $_ } } values %EXPORT_TAGS;


sub dl_load_flags { 0x01 }

__PACKAGE__->bootstrap($VERSION);

1;

