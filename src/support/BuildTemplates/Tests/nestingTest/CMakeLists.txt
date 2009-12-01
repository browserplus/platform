# this is a test of "nested" projects.  Meaning several stand alone
# projects that are configured in the same cmake invocation.
# the reason for doing this is to generate uber build files that
# know how to reach around inter-library to build dependencies.
#
# a feature that is very useful when you're hacking on multiple projects
# at once.

# XXX: order is important.  should it be?
SUBDIRS(dependee depender)
