#!/bin/sh

sudo apt-get install ctags
sudo apt-get install cscope

cd ~/.vim
mkdir .backup
mkdir .swp
mkdir .undo