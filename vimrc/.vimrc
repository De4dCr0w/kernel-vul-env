" close compatible
set nocompatible

"""
" display
"""
" show line number
set number
" highlight syntax
syntax enable
syntax on

" set theme
colorscheme desert

" show cmd in bottom
set showcmd

" show current mode
set showmode
" set color
set t_Co=256
"colorscheme desert
"solarzed的深色模式  
set background=dark
" color evening
"color evening
" highlight current line
set cursorline
" show current line info
set  ruler


"""
" file type related
"""
" check file type
filetype on
" adjust indent according to the file type
filetype indent on
" allow plugin
filetype plugin on
" allow auto Completion
filetype plugin indent on


"""
" encode and indent related
"""
" set encoding
set encoding=utf-8
" search encoding order
set fileencodings=ucs-bom,utf-8,cp936,gb18030,big5,euc-jp,euc-kr,latin1
" auto indent
set autoindent
" set tab width
set tabstop=4
" set shift width
set shiftwidth=4
"set tab to space
set expandtab
"when backspace it will delete a tab or 4 spaces
set smarttab
set softtabstop=4


"""
"mouse related
"""
" enable mouse
" set mouse=a


"""
" search related
"""
" auto match the (,{,[
set showmatch
" highlight search result
set hlsearch
" auto search when typing
set incsearch
" ignore char case
set ignorecase
" smart match
set smartcase

"""
" edit related
"""
" spell check
"set spell spelllang=en_us
"auto write
:set autowrite
" no backup
set nobackup
" no swap file
set noswapfile
" keep undo file
set undofile
" set backup, swap, undo file path
set backupdir=~/.vim/.backup//
set directory=~/.vim/.swp//
set undodir=~/.vim/.undo//
" load tags
set tags=tags;
set autochdir
" no error ring
set noerrorbells
" set error visual 
set novisualbell
" set history count
set history=1000
" set auto read, when file changed will raise a warning during edit
set autoread
" when there is more tab or space in the end of line, make it show
"set listchars=tab:»■,trail:■
"set list
" command mode auto suggestion
set wildmenu
set wildmode=longest:list,full



"""Plugin install
set nocompatible              " required
filetype off                  " required
set rtp+=~/.vim/bundle/Vundle.vim
call vundle#begin()
Plugin 'gmarik/Vundle.vim'
" add plugin here
Plugin 'scrooloose/nerdtree'
"Plugin 'Valloric/YouCompleteMe'
Plugin 'majutsushi/tagbar'
Plugin 'w0rp/ale'
Plugin 'ludovicchabant/vim-gutentags'
Plugin 'jiangmiao/auto-pairs'
Plugin 'scrooloose/nerdcommenter'
call vundle#end()            " required
filetype plugin indent on    " required
 

" NERDTree config
" map <F2> to NERDTree
map <F2> :NERDTreeToggle<CR>
autocmd StdinReadPre * let s:std_in=1
autocmd VimEnter * if argc() == 0 && !exists("s:std_in") | NERDTree | endif
let NERDTreeWinSize = 25
let NERDTreeShowLineNumbers = 1
let NERDTreeAutoCenter = 1 
let NERDTreeShowBookmarks = 1


" Tagbar Plugin
let g:tagbar_width=25
"let g:tagbar_autofocus=1
"let g:tagbar_left = 1
nmap <F3> :TagbarToggle<CR>
autocmd BufReadPost *.cpp,*.c,*.h,*.cc,*.cxx,*.py call tagbar#autoopen()



" YouCompleteMe {{{
    "youcompleteme  默认tab  s-tab 和自动补全冲突
    "let g:ycm_key_list_select_completion=['<c-n>']
    let g:ycm_key_list_select_completion = ['<tab>','<Down>']
    "let g:ycm_key_list_previous_completion=['<c-p>']
    let g:ycm_key_list_previous_completion = ['<Up>']
    let g:ycm_complete_in_comments = 1  "compelte comment also
    let g:ycm_complete_in_strings = 1   "complete string
    let g:ycm_use_ultisnips_completer = 1 "hint for UltiSnips
    let g:ycm_collect_identifiers_from_comments_and_strings = 1   "the char in comment and strings will also add to sets
    let g:ycm_collect_identifiers_from_tags_files = 1
    " identifier auto suggestion
    let g:ycm_seed_identifiers_with_syntax=1


    " 跳转到定义处, 分屏打开
    let g:ycm_goto_buffer_command = 'horizontal-split'
    " nnoremap <leader>jd :YcmCompleter GoToDefinition<CR>
    nnoremap <leader>jd :YcmCompleter GoToDefinitionElseDeclaration<CR>
    nnoremap <leader>gd :YcmCompleter GoToDeclaration<CR>

    " 引入，可以补全系统，以及python的第三方包 针对新老版本YCM做了兼容
    " old version
    if !empty(glob("~/.vim/bundle/YouCompleteMe/cpp/ycm/.ycm_extra_conf.py"))
        let g:ycm_global_ycm_extra_conf = "~/.vim/bundle/YouCompleteMe/cpp/ycm/.ycm_extra_conf.py"
    endif
    " new version
    if !empty(glob("~/.vim/bundle/YouCompleteMe/third_party/ycmd/cpp/ycm/.ycm_extra_conf.py"))
        let g:ycm_global_ycm_extra_conf = "~/.vim/bundle/YouCompleteMe/third_party/ycmd/cpp/ycm/.ycm_extra_conf.py"
    endif

    " 直接触发自动补全 insert模式下
    " let g:ycm_key_invoke_completion = '<C-Space>'
    " 黑名单,不启用
    let g:ycm_filetype_blacklist = {
        \ 'tagbar' : 1,
        \ 'gitcommit' : 1,
        \}
    let g:ycm_server_python_interpreter = '/usr/bin/python'
    let g:ycm_global_ycm_extra_conf = '~/.vim/.ycm_extra_conf.py'
" }}}


"""
"scope 
"建立数据库：cscope -Rbq
"F5 查找c符号
"F6 查找字符串
"F7 查找函数被谁调用
"F8 查找函数调用了谁
"""

if has("cscope")
	set csprg=/usr/bin/cscope
	set csto=1
	set cst
	set nocsverb
	"add any database in current directory
	if filereadable("cscope.out")
		cs add cscope.out
	endif
	set csverb
endif

:set cscopequickfix=s-,c-,d-,i-,t-,e-

"nmap <C-_>s : cs find s <C-R>=expand("<cword>")<CR><CR>
"F5 查找c符号  F6 查找字符串 F7 查找函数被谁调用 F8 查找函数调用了谁
nmap <silent> <F5> :cs find s <C-R>=expand("<cword>")<CR><CR>
nmap <silent> <F6> :cs find t <C-R>=expand("<cword>")<CR><CR>
nmap <silent> <F7> :cs find c <C-R>=expand("<cword>")<CR><CR>
nmap <silent> <F8> :cs find d <C-R>=expand("<cword>")<CR><CR>


" ale
let g:ale_sign_column_always = 1
"自定义error和warning图标
let g:ale_sign_error = '✗'
let g:ale_sign_warning = '⚡'
"在vim自带的状态栏中整合ale
let g:ale_statusline_format = ['✗ %d', '⚡ %d', '✔ OK']
let g:ale_echo_msg_format = '[%linter%] %s %code: %%s'
let g:ale_lint_on_text_changed = 'normal'
let g:ale_lint_on_insert_leave = 1
let g:ale_c_gcc_options = '-Wall -O2 -std=c99'
let g:ale_cpp_gcc_options = '-Wall -O2 -std=c++14'
let g:ale_c_cppcheck_options = ''
let g:ale_cpp_cppcheck_options = ''

" gutentags搜索工程目录的标志，碰到这些文件/目录名就停止向上一级目录递归 "
let g:gutentags_project_root = ['.root', '.svn', '.git', '.project']

" 配置 ctags 的参数 "
let g:gutentags_ctags_extra_args = ['--fields=+niazS', '--extra=+q']
let g:gutentags_ctags_extra_args += ['--c++-kinds=+pxI']
let g:gutentags_ctags_extra_args += ['--c-kinds=+px']

""nerdcomment
let g:NERDSpaceDelims=1     " 注释后面自动加空格"
let g:mapleader=","


autocmd BufNewFile *.c,*.sh,*.py, exec ":call SetTitle()"
"新建.c,.py,.sh,
"""定义函数SetTitle，自动插入文件头
func SetTitle()
    let filetype_name = strpart(expand("%"), stridx(expand("%"), "."))
    let file_name = strpart(expand("%"), 0, stridx(expand("%"), "."))
    if file_name =~ "\/"
        let file_name = strpart(file_name, strridx(file_name, "/") + 1)
    endif
    let time_value = strftime("%Y-%m-%d %H:%M:%S")
    if filetype_name == ".sh"
        call setline(1, "\#!/bin/bash")
        call append(line("."), "")
        call append(line(".")+1, "\#########################################################################")
        call append(line(".")+2, "\# File Name: ". file_name . filetype_name)
        call append(line(".")+3, "\# Created on: ".time_value)
        call append(line(".")+4, "\# Author: De4dCr0w")
        call append(line(".")+5, "\# Last Modified: ".time_value)
        call append(line(".")+6, "\# Description: ")
        call append(line(".")+7, "\#########################################################################")
        call append(line(".")+8, "")
    elseif filetype_name == ".py"
			call setline(1, "\# -*- coding: utf-8 -*-")
			call append(line("."), "") 
			call append(line(".")+1, "\#########################################################################") 
			call append(line(".")+2, "\# File Name: ". file_name . filetype_name)  
			call append(line(".")+3, "\# Created on : ".time_value)  
			call append(line(".")+4, "\# Author: De4dCr0w")
			call append(line(".")+5, "\# Last Modified: ".time_value)
			call append(line(".")+6, "\# Description:")  
			call append(line(".")+7, "\#########################################################################")
			call append(line(".")+8, "")
    elseif filetype_name == ".c" 
			call setline(1, "\/*************************************************************")
                call append(line("."), " * File Name: ". file_name . filetype_name)
                call append(line(".")+1, " * ")
                call append(line(".")+2, " * Created on: ".time_value)
                call append(line(".")+3, " * Author: De4dCr0w")
                call append(line(".")+4, " * ")
                call append(line(".")+5, " * Last Modified: ".time_value)
                call append(line(".")+6, " * Description: ")
                call append(line(".")+7, "************************************************************/")
                call append(line(".")+8, "")
    endif
endfunc



" modify the last modified time of a file  
function SetLastModifiedTime(lineno)  
    let modif_time = strftime("%Y-%m-%d %H:%M:%S")
    if a:lineno == "-1"  
            let line = getline(7)  
    else  
            let line = getline(a:lineno)  
    endif
    if line =~ '\sLast Modified:'
            let line = strpart(line, 0, stridx(line, ":")) . ": " . modif_time
    endif  
    if a:lineno == "-1"  
            call setline(7, line)  
    else  
            call append(a:lineno, line)  
    endif  
endfunc
" map the SetLastModifiedTime command automatically  
autocmd BufWrite *.c,*.sh,*.py, call SetLastModifiedTime(-1)
