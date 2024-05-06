OBJECTIVE:
Develop a peer-to-peer chat application that enables two users to communicate directly with each other. The application should also facilitate the transfer of various types of files between the users.


IMPLEMENTATION:
1. Alice sets up her application with two components: a Server thread for sending messages and files, and a Client thread for receiving them.
2. Initially, Alice activates her server on a specific port.
3. Alice's server remains idle, waiting for any incoming connections to her port.
4. Bob also prepares his system, setting his server to listen for connections on his own designated port.
5. To establish a link, Alice and Bob input each other's port numbers into their respective systems.
6. Once their connection is successfully established, both can send messages and transfer files.
7. Alice's Server thread handles writing and sending messages or files to Bob, while Bob receives them using his Client thread, and the roles reverse for Bob sending to Alice.


CODE EXECUTION:
1. Make two copies of the code ChatBot.cpp. One for Alice and one for Bob in directories X and Y.
2. Use the below command to start the server of Alice in directory X.
`g++ -std=c++11 -o chat_bot ChatBot.cpp`
Give the username as Alice to specify that the server of Alice has to start.
3. Similarly start the  server of Bob in directory Y.
`g++ -std=c++11 -o chat_bot ChatBot.cpp`
Give the username as Alice to specify that the server of Alice has to start.
4. In the terminal that has Alice's server enter Bob's port number to connect.
5. In the terminal that has Bobs' server enter Alice's port number to connect.
6. Once the connection is established, we can send messages to each other by typing the messages in their respective terminals.
7. To transfer file from ALice to Bob, type the below command in Alice's server terminal.
`transfer <file_name>` and the received file is saved as received_filename in Bob folder (Y folder)
8. To transfer file from Bob to Alice, type the below command in Bob's server terminal.
`transfer <file_name>` and the received file is saved as received_filename in Alice folder (X folder)

NOTe: make you the file exists in the folder for transfer 