use strict;
use warnings;

use ExtUtils::MakeMaker;

use Cwd;
use File::Spec;

use Gtk2::CodeGen;
use Glib::MakeHelper;
use ExtUtils::Depends;
use ExtUtils::PkgConfig;

my $DEPENDS;
my @XS_FILES = ();

exit main();

sub main {

	my %deps = (
			'Gtk2::Clutter' => '0.100',
			'Champlain'     => '0.09',
	);
	
	# Create the build folder used by the code generation utilities
	mkdir 'build', 0777;
	
	# Find libchamplain-gtk
	my %pkgconfig;
	eval {
		%pkgconfig = ExtUtils::PkgConfig->find("champlain-gtk-0.4");
		push @XS_FILES, <xs/Gtk2Champlain*.xs>;
	};
	if (my $error = $@) {
		warn "FAIL: ", $error;
		return;
	}
	
	$DEPENDS = ExtUtils::Depends->new('Gtk2::Champlain', keys %deps);
	
	$DEPENDS->add_pm(
		File::Spec->catfile('lib', 'Gtk2', 'Champlain.pm'),
		File::Spec->catfile('$(INST_LIBDIR)', 'Champlain.pm'),
	);

	# Code generation
	Gtk2::CodeGen->parse_maps('champlain-gtk', input => [ 'maps' ]);
	Gtk2::CodeGen->write_boot(
		ignore   => qr/^Gtk2::Champlain$/,
		xs_files => [ @XS_FILES ]
	);
	
	
	$DEPENDS->set_inc($pkgconfig{cflags} . ' -I./build');
	$DEPENDS->set_libs($pkgconfig{libs});
	$DEPENDS->add_xs(@XS_FILES);
	$DEPENDS->add_typemaps(
		File::Spec->catfile(cwd(), 'build', 'champlain-gtk.typemap')
	);
	$DEPENDS->install(
		File::Spec->catfile('build', 'champlain-gtk-autogen.h'),
		'champlain-gtk-perl.h',
	);
	$DEPENDS->save_config(File::Spec->catfile('build', 'IFiles.pm'));
	

	# If Gtk2::Clutter isn't loaded then the following warning will be printed:
	#   GLib-GObject-WARNING **: cannot register existing type `GConnectFlags'
	require Gtk2::Clutter;

	# Create the Makefile
	WriteMakefile(
		AUTHOR        => 'Emmanuel Rodriguez <potyl@cpan.org>',
		NAME          => 'Gtk2::Champlain',
		VERSION_FROM  => File::Spec->catfile('lib', 'Gtk2', 'Champlain.pm'),
		ABSTRACT_FROM => File::Spec->catfile('lib', 'Gtk2', 'Champlain.pm'),
		LICENSE       => 'perl, lgpl',

		PREREQ_PM     => \%deps,
		PREREQ_FATAL  => 1,
		
		XSPROTOARG    => '-noprototypes ',
		MAN3PODS      => {
			Glib::MakeHelper->do_pod_files(@XS_FILES),
		},

		$DEPENDS->get_makefile_vars(),
		
		# Remove the build folder when doing "make clean"
		clean => {
			FILES => 'build',
		},
	);
	
	return 0;
}


sub MY::postamble {
	
	my $postamble = Glib::MakeHelper->postamble_clean();
	$postamble .= Glib::MakeHelper->postamble_docs_full(
		DEPENDS   => $DEPENDS,
		XS_FILES  => [ @XS_FILES ],
		COPYRIGHT => 'Copyright (C) 2009 by Emmanuel Rodriguez'
	);

	return $postamble;
}
