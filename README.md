 

# Linux Shell Design - Project Description

### The repository for the project is available at https://github.com/frediff/Linux-Shell

## Linux-Shell 

This project is about implementing a shell that will run as an application program on top of the Linux kernel. The shell will accept user commands (one line at a time) and execute the same. We have implemented the following features:

> ###### Run an external command
>
> The external commands execute files that are executables. They are executed by spawning a child process and invoking execlp() or similar system calls. Example user commands:
>
> ```bash
> ./a.out myprog.c
> 
> cc –o myprog myprog.c
> 
> ls -l
> ```

> ###### Run an external command by redirecting standard input from a file
>
> The symbol "<" is used for input redirection, where the input will be read from the specified file and not from the keyboard. We use a system call like dup() or dup2() to carry out the redirection. Example user command:
>
> ```bash
> ./a.out < infile.txt
> 
> sort < somefile.txt
> ```

> ###### Run an external command by redirecting standard output to a file
>
> The symbol ">" is used for output redirection, where the output will be written to the specified file and not to the screen. We use a system call like dup() or dup2() to carry out the redirection. Example user commands:
>
> ```bash
> ./a.out > outfile.txt
> 
> ls > abc
> ```

> ###### Combination of input and output redirection
>
> Here we use both "<" and ">" to specify both types of redirection. Example user command:
>
> ```bash
> ./a.out < infile.txt > outfile.txt
> ```

> ###### Run an external command in the background with possible input and output redirections
>
> We use the symbol "&" to specify running a command in the background. The shell prompt will appear, and the user can type the next command while the said command executes in the background. Example user commands:
>
> ```bash
> ./a.out &
> 
> ./myprog < in.txt > out.txt &
> ```

> ###### Run several external commands in the pipe mode
>
> The symbol "|" is used to indicate pipe mode of execution. Here, one command's standard output redirects to the next command's standard input in sequence. We use the pipe() system call to implement this feature. Example user commands:
>
> ```bash
> ls | more
> 
> cat abc.c | sort | more
> ```

> ###### Interrupting commands running in the shell (using signal call)
>
> - Halt a command running in the user's shell during runtime. For instance, if the user presses "Ctrl - c" while a program executes, the program stops executing, and the shell prompt reappears. Note that the shell does not stop if the user presses "Ctrl - c."
> - Move a command in execution to the background. If the user presses "Ctrl - z" while a program is executing, the program execution moves to the background, and the shell prompt will reappear.

> ###### Implementing cd and pwd
>
> Implement the "cd" command, which changes the shell's current working directory, and the "pwd" command, which prints the name of the present working directory.

> ###### Handling wildcards in commands ('*' and '?')
>
> *Wildcards* are a feature that allows a single command to perform operations on multiple files matching the wildcard. The '*' character matches 0 or more characters. The '?' character matches a single character. Examples: Suppose there are files named file1.txt, file2.txt, and file3.txt in directories named dir1, dir2, and dir3, respectively. The following commands open all three files.
>
> ```bash
> gedit dir?/file?.txt
> ```
>
> ```bash
> gedit dir1/file1.txt dir2/file2.txt dir3/file3.txt
> ```
>
> ```bash
> gedit d*/f*
> ```
>
> The following commands concatenate the three files and sort them.
>
> ```bash
> sort dir?/file?.txt
> ```
>
> ```bash
> sort dir1/file1.txt dir2/file2.txt dir3/file3.txt
> ```
>
> ```bash
> sort d*/f*
> ```

> ###### Implementing searching through history using up/down arrow keys
>
> Maintaining a history of the last 1000 commands run in the user's shell. Pressing the 'up' (arrow) key shows the previous command run in the shell. If no previous command exists, then nothing is done. When some previous command is being shown (since the user pressed the up arrow key), pressing the down arrow key will show the command run after the command that is currently shown (similar to a typical shell). When showing the current command, pressing the down arrow key will do nothing. An example: Suppose we run the following commands in the shell
>
> ```bash
> touch myfile.txt
> 
> cd foldername
> 
> <Current command>
> ```
>
> - Pressing the up key when we are at the current command will change the current command to "cd folder name." Pressing the down arrow key at the current command will do nothing.
> - Pressing the up key when "cd folder name" is being shown will change the command to "touch myfile.txt." Pressing the down key when "cd folder name" is being shown will show the current command (empty if the has typed nothing).
> - Pressing the up key when "touch myfile.txt" is being shown will do nothing. Pressing the down key when "touch myfile.txt" is being shown will change the command to "cd folder name."
> - If the shell shows the 1000th previous command, pressing the up key will do nothing. ("touch myfile.txt" is the second previous command in the example).

> ###### Command to detect a simple malware
>
> A process can spawn multiple processes in the system, a technique many malware use (e.g., trojans). Malware spawns multiple child processes and then goes to sleep, and the child processes wreak havoc in our system. So, of course, we can run the 'top' command, check which processes are using the most CPU, and kill them using signals, but it will do us no good. The sleeping malware parent process will wake up and spawn even more processes. To deal with this situation, we implement a command "sb" (short for "squash bug") in our shell that will create a child which does the job of detecting malware. This command starts from a given process id (the user identifies the id from the "top" command as a suspected process and supplies it as an argument). Then it displays the process's parent, grandparent, and further ancestors. Note that the actual malware can be any of the parents or grandparents of the given process or the given process itself. We keep a flag "-suggest" which will additionally, based on a heuristic, detect which process id is the root of all trouble Explanations and justification for the heuristic is present in assgn2.squashbug.heuristic.txt. We also wrote a test case for this command – a process P that sleeps for 2 minutes, then wakes up, spawns five processes, and again goes to sleep. Each of these five spawned processes will again spawn ten processes each and then run an infinite loop. If we run the "sb" command, it identifies the process P as the malware. This program is NOT be a part of the shell program but rather a separate program.

> ###### Command to check for file locks
>
> Some processes start reading/writing a file after locking (using Flock) and never release it, making it impossible to delete a file. Implement a command "deep" (short for delete with extreme prejudice), which takes a file path as an argument and spawns a child process. The child process will help list all the process pids with this file open and all the process pids holding a lock over the file. Then the parent process will show it to the user and ask permission to kill each process using a yes/no prompt. On putting yes, the command kills all the processes using the "kill" signal, and then the file will be deleted. We also wrote a test code that will spawn a process that locks a file and try to write to a file using an infinite loop.

> ###### Features to help edit commands
>
> In a shell, often after writing a long command, we need to return to the command's beginning. In our shell, we added a feature "ctrl + a" that will bring the cursor to the beginning of your typed command and a feature "ctrl + e" that will bring the cursor to the end of the typed command.

#### Contributors

- Subham Ghosh (Roll No.: 20CS10065)
- Anubhav Dhar (Roll No.: 20CS30004)
- Aritra Mitra (Roll No.: 20CS30006)
- Shiladitya De (Roll No.: 20CS30061)

# 
