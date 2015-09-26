# Nodo demo... 

Talk to a NodoSmall via the serial port
of a Raspberry PI. Collect messages from redis using pubsub and control
lights via the KAKU (Klik Aan-Klik Uit) commands.

## Redesign

This program is written in C with:

- libevent (http://software.schmorp.de/pkg/libev.html)
- non-blocking serial I/O
- HiRedis (https://github.com/redis/hiredis) using git clone https://github.com/redis/hiredis.git. Then build etc
- Jansson for dealing with json (http://www.digip.org/jansson/ or https://github.com/akheron/jansson) using
git clone https://github.com/akheron/jansson.git
- libevent is installed: sudo apt-get install libev4 libev-dev

Luckily the stuff is straightforward with little modification of the message content...

So the code consists of two programs:

    publishnodo
     read serial
        get message
        json message
        publish message
        
    cmdreceiver    
     redis subscribe
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

