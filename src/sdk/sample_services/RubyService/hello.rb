# Copyright 2006-2008, Yahoo!
#  
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in
#     the documentation and/or other materials provided with the
#     distribution.
#  3. Neither the name of Yahoo! nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
# 
#  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
#  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#  DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
#  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
#   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
#  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
#  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
#  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.

# A Ruby class that serves as the "implementation" of the service
# the name of the class and functions supported by this class are
# extracted from the 'rubyServiceDefinition' below. 

require 'pp'

class HelloRuby
  # At the time the first function is invoked on the service, initialize
  # will be called.  Arguments to initialize are documented online at
  # http://browserplus.yahoo.com/developer/services/ruby/
  #
  # notice the usage of bp_log() in this function.  A small number of
  # functions are mapped into the ruby execution environment which
  # are documented at the same link (above).
  def initialize(args)
    bp_log('info', "ruby init called with #{args.pretty_inspect}")
  end
  
  def howdy(trans, args)
    bp_log('info', "howdy function invoked")

    # in ruby, arguments are passed in a hash, where the keys are strings
    # defined in your service definition (below), and values are whatever
    # data javascript passed in, mapped into the ruby environment.
    Thread.new(trans, args['callback']) do |trans,callback |
      (0..10).each do |x|
        str = "howdy there, for the #{x}th time"
        bp_log('info', "invoking callback")
        # here we demonstrate invoking callback arguments to services.
        # In ruby, these arguments are passed as standalone objects
        # with an #invoke method.  That method may be called as many
        # times as you like throughout the life of your "transaction"
        # after bp.complete or bp.error is called, your transaction will
        # be done and callbacks may no longer be invoked.
        callback.invoke(str)
        sleep 0.4
      end
      # when a transaction is complete, you can call #complete on
      # your transaction object.  If an error occurs, you could end the
      # transaction and pass an error up to javascript by using the
      # #error method.
      trans.complete("Hello world from my great service instance!")
    end
  end
end

# The definition of your service interface, ruby style.
# 'rubyServiceDefinition' is a required variable that the ruby runtime will
# pluck out and traverse to interpret your service definition.
#
# This definition will be used to automatically express a callable
# javascript interface to the web.  More documentation is available online at
# http://browserplus.yahoo.com/developer/services/ruby/
rubyServiceDefinition = {
  'class' => "HelloRuby",
  'name' => "HelloRuby",
  'major_version' => 1,
  'minor_version' => 0,
  'micro_version' => 0,
  'documentation' => 'A service that tests callbacks from ruby.',
  'functions' =>
  [
    {
      'name' => 'howdy',
      'documentation' => "Say \"hello\" to the world",
      'arguments' =>
      [
        {
          'name' => 'callback',
          'type' => 'callback',
          'required' => true,
          'documentation' => 'the callback to send a hello message to'
        }
      ]
    }  
  ] 
}
