all: python-pdf python-web perl-pdf perl-web c-pdf c-web

python-pdf:
	xsltproc --xinclude --stringparam profile.condition "python" -o python/champlain-python-tutorial.fo /usr/share/xml/docbook/stylesheet/nwalsh/fo/profile-docbook.xsl champlain-tutorial.docbook
	fop python/champlain-python-tutorial.fo -pdf python/champlain-python-tutorial.pdf

python-web:
	xsltproc --xinclude --stringparam profile.condition "python" -o python/index.html /usr/share/xml/docbook/stylesheet/nwalsh/xhtml/profile-docbook.xsl champlain-tutorial.docbook

perl-pdf:
	xsltproc --xinclude --stringparam profile.condition "perl" -o perl/champlain-perl-tutorial.fo /usr/share/xml/docbook/stylesheet/nwalsh/fo/profile-docbook.xsl champlain-tutorial.docbook
	fop perl/champlain-perl-tutorial.fo -pdf perl/champlain-perl-tutorial.pdf

perl-web:
	xsltproc --xinclude --stringparam profile.condition "perl" -o perl/index.html /usr/share/xml/docbook/stylesheet/nwalsh/xhtml/profile-docbook.xsl champlain-tutorial.docbook

c-pdf:
	xsltproc --xinclude --stringparam profile.condition "c" -o c/champlain-c-tutorial.fo /usr/share/xml/docbook/stylesheet/nwalsh/fo/profile-docbook.xsl champlain-tutorial.docbook
	fop c/champlain-c-tutorial.fo -pdf c/champlain-c-tutorial.pdf

c-web:
	xsltproc --xinclude --stringparam profile.condition "c" -o c/index.html /usr/share/xml/docbook/stylesheet/nwalsh/xhtml/profile-docbook.xsl champlain-tutorial.docbook

clean:
	rm -r python perl c

