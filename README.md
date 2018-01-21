# Personal Shell Implementation
### Kyeongsoo Kim, October 2017 ~ December 2017
## Introduction
The goal of this academic project is to become familiar with low-level Unix/POSIX system calls related to processes, files, and interprocess communication (pipes and redirection).
I implemented personal shell which supports some basic operations of shell. It supports both interactive commands as well as reading commands from a file via IO redirection.




## Builtins
  *  **`help`** : Print a list of all builtins and their basic usage.
    * **`exit`** : Exits the shell.
    * **`cd`** : Changes the current working directory of the shell.
    * **`pwd`** : Prints the absolute path of the current working directory.
    * **`color`** : takes a color as an argument (`color COLOR`) and changes the prompt to that color.
    
    available color list : 
      * RED
      * GRN
      * YEL
      * BLU
      * MAG
      * CYN
      * WHT
      * BWN

## Executables
  * **`ls`**
  * **`grep`**
  * **`cat`**


## Redirection

My shell supports all the following redirections : 

  * `prog1 [ARGS] > output.txt`
  * `prog1 [ARGS] < input.txt`
  * `prog1 [ARGS] < input.txt > output.txt`
  * `prog1 [ARGS] > output.txt < input.txt`
  * `prog1 [ARGS] | prog2 [ARGS] | ... | progN [ARGS]`
  * `prog1 [ARGS] < input.txt | prog2 [ARGS] | ... | progN [ARGS]`
* `prog1 [ARGS] | prog2 [ARGS] | ... | progN [ARGS] > output.txt`



## Job Control
One of the main jobs of a shell is to manage the execution lifetime of subprocesses. I implemented a subset of the Bash's job control tools.
  * `Ctrl-C`
  * `Ctrl-Z`
  * `jobs` : This command prints a list of the processes stopped by `Ctrl-Z`.
  * `fg %JID`: This command resumes the process identified by the provided `JID` using a `SIGCONT` signal.
  * `kill`
      * `kill %JID`: This command forces the process identified by the provided `JID` to terminate using a `SIGKILL` signal.
    * `kill PID`: If no percent sign is provided, assume the number is a PID, and send `SIGKILL` to that pid.
  * If a command ends with a `&`, my shell doesn't wait for the program to terminate before returning a prompt to the user.


## Job Control + piped processes.

My shell supports for our full suite of job control tools for piped processes.
This involves placing all the processes of a job in their own process group.
The complete list of tools supported in this shell are:

* `Ctrl-C`
* `Ctrl-Z`
* `jobs` builtin
* `fg` builtin
* `kill` builtin
* `&` operator



