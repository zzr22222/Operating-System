**Project Name:**  Simple Shell programs Design
**Project Goal:** 

1. Try to design a small C program on your own, to fulfill the most basic role of a shell: providing a command-line prompt and being able to accept commands sequentially. There should be three types of commands: internal commands (such as `help`, `exit`, etc.), external commands (common ones like `ls`, `cp`, as well as other executable programs on the disk like `HelloWorld`), and invalid commands (anything that doesn't fall into the previous two categories).
2. Extend the above shell to support piping functionality. This means that your shell program should be able to take an input like "dir || more", execute the `dir` command, and then send its output through a pipe to `more` as standard input.
