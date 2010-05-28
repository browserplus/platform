Test signing password is FreeYourBrowser

To sign a service with test certifate:
openssl smime -sign -passin pass:FreeYourBrowser -binary -nodetach -signer BrowserPlus.crt -inkey BrowserPlus.pvk -in yourService.zip -out yourService.smime
