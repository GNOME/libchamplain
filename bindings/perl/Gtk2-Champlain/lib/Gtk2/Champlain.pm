package Gtk2::Champlain;

=head1 NAME

Gtk2::Champlain - Gtk2 map rendering widget

=head1 SYNOPSIS

	use Gtk2::Clutter ':init';
	use Gtk2::Champlain;
	
	my $window = Gtk2::Window->new('toplevel');
	$window->signal_connect(destroy => sub { Gtk2->main_quit(); });
	$window->set_title('Free maps');
	$window->set_default_size(800, 600);
	
	# Embeddable map widget
	my $embed = Gtk2::Champlain::Embed->new();
	$window->add($embed);
	
	# Configure the map view
	my $map = $embed->get_view();
	$map->set_zoom_level(7);
	$map->center_on(45.466, -73.75);
	
	# Show all widgets and start the main loop
	$window->show_all();
	Gtk2->main();

=head1 DESCRIPTION

Gtk2::Champlain is a Gtk2 widget that allows Champlain maps to be embeded in any
Gtk2 application.

This makes all maps available to Champlain available to Gtk2 application. At the
moment various free map sources such as I<OpenStreetMap>, I<OpenAerialMap> and
I<Maps for free> can be easily displayed.

For more information about libchamplain-gtk see:
L<http://projects.gnome.org/libchamplain/>.

=head1 BUGS

The library libchamplain is quite young and its API is changing as the code
gains maturity. These bindings try to provide as much coverage from the C
library as possible. Don't be surprised if the API changes within the next
releases this is normal as B<libchamplain IS NOT yet API nor ABI frozen>.

It's quite probable that bugs will be exposed, please try to report all bugs
found through GNOME's Bugzilla
L<http://bugzilla.gnome.org/simple-bug-guide.cgi?product=champlain> (when
prompted for a component simply choose I<bindings>). GNOME's bug tracking tool
is preferred over RT because the bugs found in the library could impact
libchamplain or the other bindings. Of course all bugs entered through RT will
be acknowledged and addressed.

=head1 AUTHORS

Emmanuel Rodriguez E<lt>potyl@cpan.orgE<gt>.

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2009 by Emmanuel Rodriguez.

This library is free software; you can redistribute it and/or modify
it under the same terms of:

=over 4

=item the GNU Lesser General Public License, version 2.1; or

=item the Artistic License, version 2.0.

=back

This module is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

You should have received a copy of the GNU Library General Public
License along with this module; if not, see L<http://www.gnu.org/licenses/>.

For the terms of The Artistic License, see L<perlartistic>.

=cut

use warnings;
use strict;

our $VERSION = '0.01';

use base 'DynaLoader';
use Exporter 'import';

use Gtk2::Clutter;
use Champlain;

sub dl_load_flags { $^O eq 'darwin' ? 0x00 : 0x01 }

__PACKAGE__->bootstrap($VERSION);

1;

