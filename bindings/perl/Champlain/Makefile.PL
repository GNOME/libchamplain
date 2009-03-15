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
	
	# Create the build folder used by the code generation utilities
	mkdir 'build', 0777;
	
	$DEPENDS = ExtUtils::Depends->new('Champlain', 'Clutter');
	
	# The version of libchamplain (and friends) for which this module can built against
	my $champlain_version = '0.3';
	
	
	# Find libchamplain
	my @typemaps = ();
	my %pkgconfig;
	eval {
		%pkgconfig = ExtUtils::PkgConfig->find("champlain-$champlain_version");
	};
	if (my $error = $@) {
		warn "FAIL: ", $error;
		return;
	}
	
	# All XS files that start with Champlain are for libchamplain
	push @XS_FILES, <xs/Champlain*.xs>;
	push @typemaps, 'maps';
	$DEPENDS->add_pm(
		File::Spec->catfile('lib', 'Champlain.pm'),
		File::Spec->catfile('$(INST_LIBDIR)', 'Champlain.pm'),
	);
	
	
	# If libchamplain-gtk is installed the GTK bindings will be generated
	eval {
		my %conf = ExtUtils::PkgConfig->find("champlain-gtk-$champlain_version");
		$pkgconfig{cflags} .= $conf{cflags} . '-DCHAMPLAINPERL_GTK';
		$pkgconfig{libs} .= $conf{libs};
		
		# All XS files that start with Gtk2Champlain are for libchamplain-gtk
		push @XS_FILES, <xs/Gtk2Champlain*.xs>;
		push @typemaps, 'maps-gtk';
	};
	if (my $error = $@) {
		warn "Ignoring the Gtk2 bindings ($error)";
	}
	
	
	# Code generation
	Gtk2::CodeGen->parse_maps('champlain', input => [ @typemaps ]);
	Gtk2::CodeGen->write_boot(xs_files => [ @XS_FILES ]);
	
	
	$DEPENDS->set_inc($pkgconfig{cflags});
	$DEPENDS->set_libs($pkgconfig{libs});
	$DEPENDS->add_xs(@XS_FILES);
	$DEPENDS->add_typemaps(
		File::Spec->catfile(cwd(), 'build', 'champlain.typemap')
	);
	$DEPENDS->install(File::Spec->catfile('build', 'champlain-autogen.h'));
	$DEPENDS->save_config(File::Spec->catfile('build', 'IFiles.pm'));
	

	# Create the Makefile
	WriteMakefile(
		AUTHOR        => 'Emmanuel Rodriguez <potyl@cpan.org>',
		NAME          => 'Champlain',
		VERSION_FROM  => File::Spec->catfile('lib', 'Champlain.pm'),
    ABSTRACT_FROM => File::Spec->catfile('lib', 'Champlain.pm'),
		LICENSE       => 'perl, lgpl',

		PREREQ_PM     => {
			'Clutter' => '0.8000',
		},
		PREREQ_FATAL  => 1, # Clutter is mandatory otherwise the Makefile can't be created
		
		XSPROTOARG    => '-noprototypes ',
		MAN3PODS      => {
			Glib::MakeHelper->do_pod_files(@XS_FILES),
		},

		$DEPENDS->get_makefile_vars(),
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
