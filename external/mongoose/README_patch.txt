Patches
=======
* fixendings_osx.patch makes the following mods:
    1. Run dos2unix on Makefile, mongoose.h, and mongoose.c

* yahoo_all.patch makes the following mods:
    1. Change makefile to generate static libs for windows and osx.
    2. Add mg_create() and mg_destroy() methods.  
       Change existing mg_start()/stop() to only start and stop the server, 
       not create/destroy it.  This allows a client to do a bind prior 
       to starting server.
    3. Change mg_open_listening_port() to only use INADDR_LOOPBACK.
    4. Change set_ports_option() to return actual port bound (useful 
       for ephemeral).


Note: The source files in mongoose-2.8.tgz have DOS line endings.  
      We use fixendings_osx.patch to correct them on darwin.
      Without this the patch program will have troubles.
      Note we depend on alpha order to ensure fixendings_osx is applied before 
      yahoo_all.
