To push new permissions to a distro server:

# cd <build_dir>
# dist/bin/bpkg pack -publicKey bpsdk_internal/signing/BrowserPlus.crt -privateKey bpsdk_internal/signing/BrowserPlus.pvk -in libs/permissions/permissions.json -out permissions.bpkg -password FreeYourBrowser

# scp permissions.bpkg <distro_server>:/tmp

on distro server
# mv /tmp/permissions.bpkg <browserplus path>/data/

