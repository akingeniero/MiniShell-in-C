# minishell 


### Table of Contents

* [Introduction](#introduction)
	* [What the Shell?](#what-the-shell)
* [Our Implementation of Minishell](#our-implementation-of-minishell)
	* [Shell External Commands](#shell-external)
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

#### What the Shell?

As we just said, we are asked to implement our own shell, but what is a shell to begin with? If we think of (for example) Linux as a nut or a seashell, the kernel/seed is the core of the nut and has to be surrounded by a cover or shell. Likewise, the shell we are implementing works as a command interpreter communicating with the OS kernel in a secure way, and allows us to perform a number tasks from a command line, namely execute commands, create or delete files or directories, or read and write content of files, among (many) other things

## Our Implementation of Minishell

The general idea for this shell is executing all the external shell commands and some of the internal commands. Before anything, it is highly recommended to take a deep dive into the [bash manual](https://www.gnu.org/software/bash/manual/bash.html), as it goes over every detail we had to have in mind when doing this project. The target of the project is working with pipes and processes.

### Shell External Commands

### Execute Commands in Background

### cd Command

### umaks Command

### fg Command

### exit Command

### jobs Command

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
