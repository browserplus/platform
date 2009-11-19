Test signing password is FreeYourBrowser

To sign a corelet with test certifate:
openssl smime -sign -passin pass:FreeYourBrowser -binary -nodetach -signer BrowserPlus.crt -inkey BrowserPlus.pvk -in yourCorelet.zip -out yourCorelet.smime
