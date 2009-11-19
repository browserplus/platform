To push new permissions to a distro server:

#../../build/Archive/dist/bin/Debug/bpkg pack -publicKey ../../signing/devel/BrowserPlus.crt -privateKey ../../signing/devel/BrowserPlus.pvk -in permissions.json -out permissions.bpkg -password FreeYourBrowser

# scp permissions.bpkg <distro_server>:/tmp

on distro server
# mv /tmp/permissions.bpkg <browserplus path>/data/

