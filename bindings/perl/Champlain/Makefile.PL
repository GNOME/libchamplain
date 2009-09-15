use strict;
use warnings;

use ExtUtils::MakeMaker;

use Cwd;
use File::Spec;

use Glib::CodeGen;
use Glib::MakeHelper;
use ExtUtils::Depends;
use ExtUtils::PkgConfig;

my $DEPENDS;
my @XS_FILES = ();

exit main();

sub main {
	
	# Create the build folder used by the code generation utilities
	mkdir 'build', 0777;
	
	# The version of libchamplain (and friends) for which this module can built against
	my $champlain_version = '0.4';
	
	# Find libchamplain
	my @typemaps = ();
	my @deps = ('Glib');
	my %pkgconfig;

	eval {
		%pkgconfig = ExtUtils::PkgConfig->find("champlain-$champlain_version");
		# All XS files that start with Champlain are for libchamplain
		push @XS_FILES, <xs/Champlain*.xs>;
		push @typemaps, 'maps';
		push @deps, 'Clutter';
	};
	if (my $error = $@) {
		warn "FAIL: ", $error;
		return;
	}
	
	$DEPENDS = ExtUtils::Depends->new('Champlain', @deps);
	
	$DEPENDS->add_pm(
		File::Spec->catfile('lib', 'Champlain.pm'),
		File::Spec->catfile('$(INST_LIBDIR)', 'Champlain.pm'),
	);

	# Code generation
	Glib::CodeGen->parse_maps('champlain', input => [ @typemaps ]);
	Glib::CodeGen->write_boot(xs_files => [ @XS_FILES ]);
	
	
	$DEPENDS->set_inc($pkgconfig{cflags} . ' -I./build');
	$DEPENDS->set_libs($pkgconfig{libs});
	$DEPENDS->add_xs(@XS_FILES);
	$DEPENDS->add_typemaps(
		File::Spec->catfile(cwd(), 'build', 'champlain.typemap')
	);
	$DEPENDS->install(
		File::Spec->catfile('build', 'champlain-autogen.h'),
		'champlain-perl.h',
	);
	$DEPENDS->save_config(File::Spec->catfile('build', 'IFiles.pm'));
	

	# Create the Makefile
	WriteMakefile(
		AUTHOR        => 'Emmanuel Rodriguez <potyl@cpan.org>',
		NAME          => 'Champlain',
		VERSION_FROM  => File::Spec->catfile('lib', 'Champlain.pm'),
		ABSTRACT_FROM => File::Spec->catfile('lib', 'Champlain.pm'),
		LICENSE       => 'perl, lgpl',

		PREREQ_PM     => {
			'Clutter' => '1.0000',
		},
		PREREQ_FATAL  => 1, # Clutter is mandatory otherwise the Makefile can't be created
		
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
