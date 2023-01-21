
# minishell

This project is all about recreating the shell, taking bash (Bourne Again Shell) as reference. This was a couple project, and I worked with [@lord-47](https://github.com/lord-47) :)


### What the Shell?

As we just said, we are asked to implement our own shell, but what is a shell to begin with? If we think of (for example) Linux as a nut or a seashell, the kernel/seed is the core of the nut and has to be surrounded by a cover or shell. Likewise, the shell we are implementing works as a command interpreter communicating with the OS kernel in a secure way, and allows us to perform a number tasks from a command line, namely execute commands, create or delete files or directories, or read and write content of files, among (many) other things

## Our Implementation of Minishell

The general idea for this shell is executing all the external shell commands and some of the internal commands. Before anything, it is highly recommended to take a deep dive into the [bash manual](https://www.gnu.org/software/bash/manual/bash.html), as it goes over every detail we had to have in mind when doing this project. The target of the project is working with pipes and processes.

### Shell External Commands

As mentioned before, the minishell can execute all system commands. You can also execute commands with pipes, so you can nest commands with a limit of up to 50 linked commands (which we think is more than enough). To connect commands you have to use the character |, for example:

```shell
$ ls *.txt | grep test | wc -l
```

### Execute Commands in Background

It also allows sending commands to the background. This allows you to continue using the minishell while a command is running. To send commands to the background you have to use the character &, for example: 

```shell
$ sleep 3 &
```

### jobs Command

### fg Command

### cd Command

### umaks Command

### exit Command

## Installation

* Prerequisites

Make sure gcc compiler is installed:


* Cloning the Repositories

```shell
git clone https://github.com/BraisCabo/minishell.git
cd minishell
make
```

### Basic Usage

As per the norm, this project compiles an executable called ``minishell`` , and it is compiled using the ``.c`` files inside the folder

To execute the minishell you only have to give execution permissions

```shell
chmod u+x minishell
./minishell
```

## Summary

This was one of our biggest project yet, and it sure was challenging. Co-developing can be tricky but we had fun in the process and learn a lot about concurrent programming :)
