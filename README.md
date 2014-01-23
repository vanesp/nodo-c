# Nodo demo... 

Using socketstream and node.js talk to a NodoSmall via the serial port
of a Raspberry PI. Create a live user interface and allow control of 
lights via the KAKU (Klik Aan-Klik Uit) commands.

## Redesign

The Node.js application just acts as a monitor program... it does not interact with the
serial port anymore. The reason for this is because the node.js application does not 
work in the background if no browser is opened for http://rpi1.local:3333

This has to be written in C with:

- libevent (http://software.schmorp.de/pkg/libev.html)
- non-blocking serial I/O
- HiRedis (https://github.com/redis/hiredis)
- Jansson for dealing with json (http://www.digip.org/jansson/ or https://github.com/akheron/jansson)

- libevent is installed: sudo apt-get install libev4 libev-dev

Luckily the stuff is straightforward with little modification of the message content...

So the code consists of two programs:

    publish_nodo
     read serial
        get message
        json message
        publish message
        
    cmdreceiver    
     redis_subscribe
        get message event
        unpack json
        write to serial port


## Workings

The client receives, via Redis Pub/Sub, two kinds of messages. These looks as follows to conform to the 
Socketstream specification:

    "publish" "ss:event" "{\"t\":\"all\",\"e\":\"newMessage\",\"p\":[\"\\u0013\\u0000Error in command.\"]}"
    {
       "t" : "all",
       "e" : "newMessage",
       "p" : [ "param1", "param2" ]
    }

 The parameters for the messages we send look as follows:
 
    type        = Switch (to control a switch setting) or Motion (for a motion event)
    location    = 3 for Woonkamer, and 2 for Study. For a Switch these may contain values
                  1..4 or any other named switches if these switches are defined in the code
    quantity    = for a Switch this contains the body of the string to be sent to the Nodo
    value       = is 1 for a motion event, and a true for On, or false for Off


    
Motion events are just show in the log. Switch events cause the app to send a command to
the Nodo which switches the corresponding switch via rs-232 interface to the Nodo device.

* Motion events are captured on the Portux server in the rcvsend.php process (such that they
are transmitted immediately).
* Switch control events are generated in the controller.php process on the Portux server.

When the nodo is ultimately connected directly to the Portux, the redis database will
reside there and this code is no longer required.

## Requirements

Redis is installed on the Portux and invoked through socketstream as follows:

    # host changed to the Portux machine
    ss.session.store.use 'redis' 
        host: 'portux.local'




