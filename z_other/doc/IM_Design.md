# IM Design

## What it do
Client
* Chat with other user
* Chat history [Optional]
* Friends list [Optional]

Server  
* Help user to chat with other user
* Help user to find out friends [Optional]

## Message transport design
* All chat and control messages
  use only two queues to receive and send respectively.
* A pair of request and response use a same and
  unique id, and get the context by this id.
  
## Problems
* Is the message sent successfully?
* How to plan the queues?
* How to plan the threads?  
  If it's an independent worker, it likely need a
  independent thread.
* What is need to wait?  
  Waiting one thing should block not other
  things in principle.