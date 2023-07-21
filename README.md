# One-Time Pads
In this assignment, you will be creating five small programs that encrypt and decrypt information using a one-time pad-like system. These programs will combine the multi-processing code you have been learning with socket-based inter-process communication. Your programs will also be accessible from the command line using standard Unix features like input/output redirection, and job control. Finally, you will write a short compilation script.

## enc_server
This program is the encryption server and will run in the background as a daemon.

- Its function is to perform the actual encoding, as described above in the Wikipedia quote.
- This program will listen on a particular port/socket, assigned when it is first ran (see syntax below).
- Upon execution, enc_server must output an error if it cannot be run due to a network error, such as the ports being unavailable.
- When a connection is made, enc_server must call accept to generate the socket used for actual communication, and then use a separate process to handle the rest of the servicing for this client connection (see below), which will occur on the newly accepted socket.
- This child process of enc_server must first check to make sure it is communicating with enc_client (see enc_client, below).
- After verifying that the connection to enc_server is coming from enc_client, then this child receives plaintext and a key from enc_client via the connected socket.
- The enc_server child will then write back the ciphertext to the enc_client process that it is connected to via the same connected socket.
- Note that the key passed in must be at least as big as the plaintext.

## enc_client
This program connects to enc_server, and asks it to perform a one-time pad style encryption as detailed above. By itself, enc_client doesn’t do the encryption - enc_server does.

In this syntax, plaintext is the name of a file in the current directory that contains the plaintext you wish to encrypt. Similarly, key contains the encryption key you wish to use to encrypt the text. Finally, port is the port that enc_client should attempt to connect to enc_server on. When enc_client receives the ciphertext back from enc_server, it should output it to stdout

If enc_client receives key or plaintext files with ANY bad characters in them, or the key file is shorter than the plaintext, then it should terminate, send appropriate error text to stderr, and set the exit value to 1.

enc_client should NOT be able to connect to dec_server, even if it tries to connect on the correct port - you’ll need to have the programs reject each other. If this happens, enc_client should report the rejection to stderr and then terminate itself. In more detail: if enc_client cannot connect to the enc_server server, for any reason (including that it has accidentally tried to connect to the dec_server server), it should report this error to stderr with the attempted port, and set the exit value to 2. Otherwise, upon successfully running and terminating, enc_client should set the exit value to 0.

Again, any and all error text must be output to stderr (not into the plaintext or ciphertext files).

## dec_server
This program performs exactly like enc_server, in syntax and usage. In this case, however, dec_server will decrypt ciphertext it is given, using the passed-in ciphertext and key. Thus, it returns plaintext again to dec_client.

## dec_client
Similarly, this program will connect to dec_server and will ask it to decrypt ciphertext using a passed-in ciphertext and key, and otherwise performs exactly like enc_client, and must be runnable in the same three ways. dec_client should NOT be able to connect to enc_server, even if it tries to connect on the correct port - you’ll need to have the programs reject each other, as described in enc_client.

## keygen
This program creates a key file of specified length. The characters in the file generated will be any of the 27 allowed characters, generated using the standard Unix randomization methods. Do not create spaces every five characters, as has been historically done. Note that you specifically do not have to do any fancy random number generation: we’re not looking for cryptographically secure random number generation. rand()Links to an external site. is just fine. The last character keygen outputs should be a newline. Any error text must be output to stderr.
