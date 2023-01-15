# minishell 


### Table of Contents

* [Introduction](#introduction)
	* [What the Shell?](#what-the-shell)
* [Our Implementation of Minishell](#our-implementation-of-minishell)
	* [Lexer and Expander](#lexer-and-expander)
	* [Parser](#parser)
	* [Executor](#executor)
	* [Mind Map](#mind-map)
	* [Global Variable](#global-variable)
* [Builtins](#builtins)
* [Prompt](#prompt)
* [Installation](#installation)
	* [Basic Usage](#basic-usage)
	* [Demos](#demos)
* [References](#references)
* [Summary](#summary)


## Introduction

This project is all about recreating the shell, taking bash (Bourne Again SHell) as reference. This was a couple project, and I worked with [@lord-47](https://github.com/lord-47) :)
The 

#### What the Shell?

As we just said, we are asked to implement our own shell, but what is a shell to begin with? If we think of (for example) Linux as a nut or a seashell, the kernel/seed is the core of the nut and has to be surrounded by a cover or shell. Likewise, the shell we are implementing works as a command interpreter communicating with the OS kernel in a secure way, and allows us to perform a number tasks from a command line, namely execute commands, create or delete files or directories, or read and write content of files, among (many) other things

## Our Implementation of Minishell

The general idea for this shell is executing all the external shell commands and some of the internal commands. Before anything, it is highly recommended to take a deep dive into the [bash manual](https://www.gnu.org/software/bash/manual/bash.html), as it goes over every detail we had to have in mind when doing this project. The target of the project is working with pipes and processes.

### Executor

With all our data properly on our structs, the ``executer`` has all the necessary information to execute commands. For this part we use separate processess to execute either our builtins or other commands inside child processes that redirect ``stdin`` and ``stdout`` in the same way we did with our previous [pipex](https://github.com/madebypixel02/pipex) project. If we are given a full path (e.g. ``/bin/ls``) then we do not need to look for the full path of the command and can execute directly with [execve](https://www.man7.org/linux/man-pages/man2/execve.2.html). If we are given a relative path then we use the ``PATH`` environment variable to determine the ``full_path`` of a command. After all commands have started running, we retrieve the exit status of the most recently executed command with the help of [waitpid](https://linux.die.net/man/2/waitpid)

Once all commands have finished running the allocated memory is freed and a new prompt appears to read the next command

### Mind Map

Here is a handy mindmap of our code structure to help you understand everything we mentioned previously

![Concept Map - Frame 5](https://user-images.githubusercontent.com/71781441/144017004-aa68e8d7-5da7-4ece-afc6-b8ab100113df.jpg)
![Concept Map - Frame 4](https://user-images.githubusercontent.com/71781441/144017016-ef2bb606-c301-42c6-88f1-8ed4339d22cd.jpg)

### Global Variable

For this project we could use one global variable. At first it seemed we were never going to need one, but later it became obvious that it is required. Specifically, it has to do with signals. When you use [signal](https://www.man7.org/linux/man-pages/man7/signal.7.html) to capture ``SIGINT`` (from ``Ctrl-C``) and ``SIGQUIT`` (from ``Ctrl-\``) signals, we have to change the error status, and the ``signal`` function has no obvious way of retrieving the updated exit status that shoud change when either of these signals are captured. To work this around, we added a global variable ``g_status`` that updates the error status when signals are detected.

## Builtins

We were asked to implement some basic builtins with the help of some functions, here is a brief overview of them:

| Builtin | Description | Options | Parameters | Helpful Functions |
| :-----: | :---------: | :-----: | :--------: | :---------------: |
| ``echo`` | Prints arguments separated with a space followed by a new line | ``-n`` | :heavy_check_mark: | [write](https://man7.org/linux/man-pages/man2/write.2.html) |
| ``cd`` | Changes current working directory, updating ``PWD`` and ``OLDPWD`` | :x: | :heavy_check_mark: | [chdir](https://man7.org/linux/man-pages/man2/chdir.2.html) |
|  ``pwd``| Prints current working directory | :x: | :x: | [getcwd](https://www.man7.org/linux/man-pages/man3/getcwd.3.html) |
| ``env`` | Prints environment | :x: | :x: | [write](https://man7.org/linux/man-pages/man2/write.2.html) |
| ``export`` | Adds/replaces variable in environment | :x: | :heavy_check_mark: | :x: |
| ``unset`` | Removes variable from environment | :x: | :heavy_check_mark: | :x: |

## Prompt

As mentioned previously, we use ``readline`` to read the string containing the shell commands. To make it more interactive, ``readline`` receives a string to be used as a prompt. We have heavily tweaked the looks of it to be nice to use. The prompt is structured as follows:

```
$USER@minishell $PWD $
```

Some remarks:

* If there is any problem retrieving the user, it will be replaced with ``guest``
* The ``PWD`` is colored blue and dynamically replaces the ``HOME`` variable with ``~`` when the variable is set. See below for more details
* The ``$`` in the end is printed blue or red depending on the exit status in the struct

![Screenshot from 2021-11-24 13-29-43](https://user-images.githubusercontent.com/40824677/143238700-8878c4f3-4763-4c8f-976e-aae049c9ed57.png)
![Screenshot from 2021-11-24 18-30-37](https://user-images.githubusercontent.com/40824677/143287061-7b87efc3-d5ea-4d65-b2f0-87fe5e96ba17.png)


## Extras

These are a few neat extras that were not explicitly mentioned on the subject of the project but we thought would make the whole experience nicer

### User Color

The ``$USER@minishell`` part of the prompt is available in six different colors (based on the first char of the user's username):

![Screenshot from 2021-11-24 12-59-57](https://user-images.githubusercontent.com/40824677/143234581-0ff3d00d-18af-43d6-be44-15eef9583c1d.png)
![Screenshot from 2021-11-24 13-01-09](https://user-images.githubusercontent.com/40824677/143234739-bed5e503-e4ab-4016-a5e3-82e29f75f11f.png)
![Screenshot from 2021-11-24 13-05-36](https://user-images.githubusercontent.com/40824677/143235316-82cb41a2-996b-480c-ac4b-237233162741.png)
![Screenshot from 2021-11-24 13-03-11](https://user-images.githubusercontent.com/40824677/143235017-292489d2-6695-4cc5-b333-5c13fb32eeb1.png)
![Screenshot from 2021-11-24 13-07-13](https://user-images.githubusercontent.com/40824677/143235567-d19bb199-a51f-4c9e-b251-a0f8b9599f75.png)
![Screenshot from 2021-11-24 13-08-12](https://user-images.githubusercontent.com/40824677/143235730-e2d7a8ff-d398-4ff0-9a60-b0929ad38152.png)

Note: ``red`` color is reserved for the ``root`` user

### Process ID

We were told to only expand variables of the form ``$ + alphanumeric chars``. We implemented expansion of ``$$``, which expands to the program's process id (``mini_getpid()``)

![Screenshot from 2021-11-24 18-33-06](https://user-images.githubusercontent.com/40824677/143287427-778538d5-8392-4739-994e-3382f15d803d.png)

### Running without Environment

When running new instances of minishell or minishell withouth environment (``env -i ./minishell``), some environment variables need to be updated manualy, namely the shell level (``SHLVL``) or the ``_`` variable

Here's the env when minishell is launched without an environment:

![Screenshot from 2021-11-24 13-32-04](https://user-images.githubusercontent.com/40824677/143238979-4b8688f7-18ad-4c95-a380-496e5fc2ab17.png)


## Installation

* Prerequisites

Make sure you have gcc compiler installed:


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

November 24th, 2021
