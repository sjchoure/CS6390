# Overview
1. Unidirectional channels (links)
2. Building routing protocol
3. Simulation done using Unix Process correspond to a node in the network, and files correspond to channels in the network.
4. Processes will be running concurrently.
5. Normal Node: 10, Special Node: 1 (Controller Node)
6. Controller node will be needed because of the fact that we will use files to represent channels.
7. All regular node will have the same code, and should be named as node.cc
8. Controller node will be called as controller.cc
9. Nodes will send data to each other, some may not send data at all. All nodes should be able to receive data messages from others (and also route data messages in the direction of the destination)

## Command to Send data to a Node
```bash
$ node ID duration dest "this is a message" &
```
Where:   
1. "ID" is the identifier of the process from 0 to 9
2. "duration" is the number of seconds before the node kills itself
3. "dest" is the node to which the data message will be sent (again from 0 to 9)
4. "this is the message" is the actual message that will be sent over the channel.

Note that putting an "&" at the end of the command-line puts the process in the "background" (batch mode)

```sh
$ node 9 100 5 "Example" &
```
This will create a node with ID 9, which will last for 100 seconds (then kill itself automatically), it will attempt to send the message "Example" to node 5.

Controller is simply executed as:
```sh
$ controller duration &
```
## Channels, Processes, and Files

Scenario One,

1. For a channel y to x (y->x), we say
    * y is an incoming neighbor of x.
    * x is an outgoing neighbor of y.

2. Each node will broadcast a hello message. When y sends a hello message, x will receive it, and x will add y to its list of known incoming neighbors.

3. y is still not aware that it has an outgoing neighbor x since it cannot receive messages from x.

Scenario Two, 
1. If we have two channels, y->x and x->y, both x and y will learn that the other process is an incoming neighbor of them (from hello messages), but they do not know that the other process is also an outgoing neighbor. Eg. y knows it can receive messages from x but it does not know x can receive messages from y.

### File structure
1. Each node x has a single output file, called output_x, where x is from 0 to 9.
2. Each node x will also have a single incoming file, input_x, where x is from 0 to 9.
3. The input and output files of a process consist of a sequence of messages, each message will be in a separate line in the file.
4. Each message sent by a node is heard (received) by all its outgoing neighbors, but a node is not aware of who its outgoing neighbors are.
5. That's why we need a "controller" node.
6. This node will implement the fact that not all nodes can reach all other nodes in one hop.
7. The controller will have an input file that contains the network topology, it contains the info about which node is the neighbor and which are not. This file is known ONLY to the controller, not to the nodes.
8. If node y wants to send some data it will output it in the output_y file from where the controller will read and pass it on to all other nodes.
9. Same as a wireless network.
10. Only a single node can access a particular file at a time.
11. Reading EOF doesn't mean that no new data is there, we need to check for new data continously.
12. The topology file contains some of the Unidirectional channel, like:    
0 1   
1 2    
2 0    
13. If there is a bidirectional topolgy then we have sometimes like:   
0 1   
1 0   
14. Each node x will open a file calles x_received where x is the node's ID (0 to 9). Whenever x receives a data message from a node z, it will write this string to this file. Eg. if it receives the data message "z is sending this to x", then x will write in x_received:
```
message from z: z is sending this to x
```
The example script could be as follows:
```sh
/bin/rm input_? output_? ?_received
controller 100 &
node 0 100 2 "this is a message from 0" &
node 1 100 2 "this is a message from 1" &
node 2 100 -1 &
```
## Hello Protocol
1. Every 5 seconds, each node will send out a hello message with the following format
```txt
hello ID
```
If they don't receive any message within 30 seconds the neighbor will assume that the node is dead.

## Routing Protocol
1. The nodes find the path to themselves from the other nodes.
### In-tree Protocol
1. The nodes take the intree message from other node and contructs the intree tree.
2. The format of a intree message is as follows:
```sh
intree D (A D) (C D) (E C) (B A)
```
## Routing Data Messages
1. All the nodes will use Source routing protocol to send the message to the destination. The format of which will be like this:
```txt
data src dst i1 i2 .. begin the actual text message
```
2. The nodes will send data every 15 seconds, the dst is not -1.

